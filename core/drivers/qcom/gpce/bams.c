// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * BAM driver implementation.
 *
 * Provides descriptor management, pipe control,
 * transfer submission, and completion handling
 * for the Qualcomm Crypto Engine BAM interface.
 */

#include <stdint.h>
#include <stdbool.h>
#include "bam.h"
#include "string.h"
#include <trace.h>
#include <malloc.h>

/* OP-TEE: this driver only ever talks to one BAM instance (its PA/VA come
 * from the bam_config_type the caller passes into bam_init — see ce_bam.c),
 * for one client (the CE HAL, serialized by a single mutex upstream),
 * always in polling mode (bam_irq is always 0 — see
 * ucenv_optee.c). There is no target-config lookup / multi-BAM / EE table:
 * the handful of values that used to come from a bam_target_config_type are
 * simply fixed constants below.
 */
#define BAM_CNFG_BITS_VAL 0xFFFFF004

/* This EE's pipe-attribute "enabled" bit, checked by bamcore_check() before
 * reading capabilities. Distinct from BAM_EE_TRUST, which bamcore_bam_enabled()
 * uses as a special case to read BAM_CTRL.BAM_EN instead.
 */
#define BAM_QCE_EE 0

/*
 * BAM internal types (originally from bami.h), kept here since nothing else
 * in the driver needs them.
 */

/* IOVEC options in bam_iovec_options_type that bam hw would understand */
#define BAM_IOVEC_HW_FLAGS                                              \
	(BAM_IOVEC_FLAG_INT | BAM_IOVEC_FLAG_EOT | BAM_IOVEC_FLAG_EOB | \
	 BAM_IOVEC_FLAG_NWD | BAM_IOVEC_FLAG_CMD | BAM_IOVEC_FLAG_LCK | \
	 BAM_IOVEC_FLAG_UNLCK | BAM_IOVEC_FLAG_IMM_CMD)

/* Return true if size is not multiple of 0x8 or greater than 64k else false*/
#define BAM_IS_INVALID_DESC_FIFO_SIZE(size) \
	(((size % 8) != 0) || (size > 0x10000))
/*< Return true if address is not 8-byte aligned else false */
#define BAM_IS_DESC_FIFO_ALIGNED(desc_pa) ((desc_pa & 0x7) != 0)

typedef struct _bamdev bam_dev;

/* One pipe's worth of state: just the descriptor FIFO it was configured
 * with, and the three offsets needed to submit/retire descriptors. */
/* Minimal per-pipe descriptor-FIFO config kept around after bam_pipe_init():
 * just enough to (re)program the pipe on bam_pipe_enable() and to submit/
 * retire descriptors. This drops the BAM2BAM peer-pipe fields
 * (peer_base_pa, data_base_pa, peer_pipe_num, data_size) from the public
 * bam_pipe_config_type — callers (ce_bam.c) still set them since that
 * struct is also the bam_pipe_init() parameter type, but nothing here
 * ever reads them back (this port never uses BAM2BAM mode).
 */
typedef struct {
	uint32_t options; /*<-- BAM_O_* — checked on every poll (EOT/DESC_DONE) and (re)init */
	bam_dir_type dir; /*<-- pipe direction — needed to (re)program the pipe */
	bam_mode_type mode; /*<-- checked on every transfer (BAM2BAM guard) */
	bam_vaddr desc_base_va; /*<-- desc fifo base VA */
	bam_paddr desc_base_pa; /*<-- desc fifo base PA */
	uint32_t desc_size; /*<-- desc fifo size — used for offset wraparound on every transfer/poll */
	uint16_t evt_thresh; /*<-- needed to (re)program the pipe */
	uint32_t lock_group; /*<-- needed to (re)program the pipe */
} pipe_desc_cfg_t;

typedef struct _pipedev {
	bam_dev *bam;
	uint32_t pipe_num;
	pipe_desc_cfg_t pipe_cfg;
	uint32_t last_offset; /*<-- last descriptor submitted */
	uint32_t desc_offset; /*<-- next new descriptor to be written to hardware */
	uint32_t acked_offset; /*<-- next descriptor to be retired by software */
} pipe_dev;

struct _bamdev {
	bam_paddr bam_pa;
	bam_vaddr bam_va;
	uint32_t num_pipes;
	uint32_t active_pipes_mask; /*<-- checked on every poll (pipe-disabled guard) */
	pipe_dev **pipes;
};

/* Globals common to the driver */
uint32_t bam_drv_init_done = 0, evnt_ofst_readback;
volatile bam_iovec_type iovec_debug;

/* The one BAM device this port ever allocates (see bam_init). Deliberately
 * file-scope (not a function-local static) so it stays a plain, stable
 * symbol that JTAG/debugger variable dumps can look up by name, the same
 * way the original multi-device "bam_dev **bams" pool was.
 */
bam_dev *bams;

/* ============================================================================
 * Static helpers, ordered so each is defined before its first caller below —
 * no forward declarations needed.
 * ============================================================================
 */

/*
 * OSAL helpers (originally in bamosal_optee.c, reached indirectly through
 * bamosal.h). Kept as static functions with identical signatures/behavior,
 * so no call site elsewhere in this file needed to change.
 */

static void *bam_osal_malloc(uint32_t size)
{
	return malloc(size);
}

static void bam_osal_free(void *mem)
{
	free(mem);
}

static void bam_osal_mallocmemregion(bam_osal_meminfo *meminfo, uint32_t size,
				     bam_mapping_op_type map_op)
{
	/* size/map_op unused: this driver only ever deals with buffers whose
	 * physical address already equals its usable virtual address, so
	 * there is no real PA->VA mapping step to perform here. */
	(void)size;
	(void)map_op;
	meminfo->handle = (void *)1;
	meminfo->va = meminfo->pa;
}

static void bam_osal_memorybarrier(void)
{
	__asm__ volatile("dsb sy" ::: "memory");
}

/* bam_osal_cachecmd — inlined from bamosal_optee.c (qce/ removed).
 * Performs a D-cache clean+invalidate by VA range using ARM cache maintenance.
 */
static void bam_osal_cachecmd(bam_cache_op_type op, bam_vaddr addr, uint32_t size)
{
        uintptr_t a = (uintptr_t)addr & ~(uintptr_t)63;
        uintptr_t end = (uintptr_t)addr + size;
        (void)op;
        for (; a < end; a += 64)
                __asm__ volatile("dc civac, %0" :: "r"(a) : "memory");
        __asm__ volatile("dsb sy" ::: "memory");
}

static void bamdev_write_reg(bam_vaddr base, uint32_t offset, uint32_t val)
{
	bam_vaddr addr = base + offset;
	(*((volatile uint32_t *)(addr))) = val;
	DMSG("data.set 0x%x %%LONG 0x%x", (uint32_t)offset, (uint32_t)val);
}

static void bamdev_write_reg_field(bam_vaddr base, uint32_t offset,
				   uint32_t mask, uint32_t shift,
				   uint32_t field_val)
{
	bam_vaddr addr = base + offset;
	uint32_t regval = (*((volatile uint32_t *)(addr)));

	regval &= ~mask;
	regval = regval | (field_val << shift);
	(*((volatile uint32_t *)(addr))) = regval;
	DMSG("data.set 0x%x %%LONG 0x%x", (uint32_t)offset, (uint32_t)regval);
}

static uint32_t bamdev_read_reg_field(bam_vaddr base, uint32_t offset,
				      uint32_t mask, uint32_t shift)
{
	bam_vaddr addr = base + offset;
	uint32_t regval = (*((volatile uint32_t *)(addr)));

	regval &= mask;
	regval >>= shift;
	return regval;
}

/**
  Check whether the given bam supports LPAE

  @return
     1   -  LPAE supported
     0   -  LPAE not supported
*/
static uint32_t bamcore_lpae_supported(bam_vaddr base)
{
	//BAM_NUM_PIPES_BAM_DATA_ADDR_BUS_WIDTH == 1 means, 36 bit addresses.
	//BAM_NUM_PIPES_BAM_DATA_ADDR_BUS_WIDTH == 2 means, 40 bit addresses.
	uint32_t regval = bamdev_read_reg_field(
		base, BAM_NUM_PIPES,
		BAM_MNS(BAM_NUM_PIPES, BAM_DATA_ADDR_BUS_WIDTH));
	return (regval == 1 || regval == 2);
}

/**
  Check whether a given bam is enabled

  @param[in] base       BAM virtual base address
  @param[in] ee         ee of the current enviornment

  @return
     1   - bam is enabled
     0   - bam is disabled
*/
static uint32_t bamcore_bam_enabled(bam_vaddr base, uint32_t ee)
{
	uint32_t bam_enabled = 0;

	//For ndp_bam version >=0x23 , bam_version > 0x9 or bam_lite_version > 16, use the
	//pipe attribute register

	if (ee != BAM_EE_TRUST) {
		bam_enabled = bamdev_read_reg_field(base, BAM_PIPE_ATTR_EEn(ee),
						    BAM_MNS(BAM_PIPE_ATTR_EEn,
							    BAM_ENABLED));
	} else {
		bam_enabled = bamdev_read_reg_field(base, BAM_CTRL,
						    BAM_MNS(BAM_CTRL, BAM_EN));
	}

	return (bam_enabled != 0);
}

/**
  Check whether a given pipe is enabled

  @return
     1   - pipe is enabled
     0   - pipe is disabled
*/
static uint32_t bamcore_pipe_enabled(bam_vaddr base, uint32_t pipe_num)
{
	return bamdev_read_reg_field(base, BAM_P_CTRLn(pipe_num),
				     BAM_MNS(BAM_P_CTRLn, P_EN));
}

/**
  Check if nwd_done toggle was recived for this pipe
  Check if 2nd bit is set in P_CTRL
  @return
     1   - nwd_done toggle recived
     0   - nwd_done toggle not recived
*/
static uint32_t bamcore_pipe_nwd_done(bam_vaddr base, uint32_t pipe_num)
{
	return (bamdev_read_reg_field(base, BAM_P_CNSMR_SDBNDn(pipe_num),
				      BAM_MNS(BAM_P_CNSMR_SDBNDn, BAM_P_CTRL)) &
		0x2) == 0;
}

/**
  Gets the offset to the next descriptor in the FIFO that will be processed by
  the BAM.

  Gets the offset to the next descriptor in the FIFO that will be processed by
  the BAM. It may or may not process this offset, if the Event Register doesnt
  have an offset that is beyond this offset read from the BAM.

  @param[in]  base        BAM virtual base address
  @param[in]  pipe_num    Index of the pipe

  @return
  Offset of the descriptor to be processed next.
*/
static uint32_t bamcore_pipe_getdescreadoffset(bam_vaddr base,
					       uint32_t pipe_num)
{
	return bamdev_read_reg_field(base, BAM_P_SW_OFSTSn(pipe_num),
				     BAM_MNS(BAM_P_SW_OFSTSn, SW_DESC_OFST));
}

/**
  Gets the offset to the next descriptor in the FIFO that will be updated by
  the Software for the next transfer (in future).

  Gets the offset to the next descriptor in the FIFO that will be updated by
  the Software for the next transfer (in future). It indicates that BAM should
  process descriptors prior to that offset from the Descriptor Fifo.

  @param[in]  base        BAM virtual base address
  @param[in]  pipe_num    Index of the pipe

  @return
  Offset of the descriptor to be processed next.

*/
static uint32_t bamcore_pipe_getdescwriteoffset(bam_vaddr base,
						uint32_t pipe_num)
{
	return bamdev_read_reg_field(base, BAM_P_EVNT_REGn(pipe_num),
				     BAM_MNS(BAM_P_EVNT_REGn,
					     P_DESC_FIFO_PEER_OFST));
}

/**
  Starts a transfer by writing an offset to the Event Register.

  Starts a transfer by writing an offset to the Event Register. The offset is
  the offset of a descriptor *after* the last descriptor written by software.
  It indicates that BAM should process descriptors prior to that offset from the
  Descriptor Fifo.

  @param[in]  base          BAM virtual base address
  @param[in]  pipe_num      Index of the pipe
  @param[in]  next_write    Offset that will contain the next transfer
  @param[in]  bytes_consumed    Bytes consumed for triggerring event (SBZ for system mode)

  @return
  None.

*/
static void bamcore_pipe_setdescwriteoffset(bam_vaddr base, uint32_t pipe_num,
					    uint32_t next_write,
					    uint32_t bytes_consumed)
{
	uint32_t value = ((next_write << BAM_SHFT(BAM_P_EVNT_REGn,
						  P_DESC_FIFO_PEER_OFST)) &
			  BAM_MASK(BAM_P_EVNT_REGn, P_DESC_FIFO_PEER_OFST)) |
			 ((bytes_consumed
			   << BAM_SHFT(BAM_P_EVNT_REGn, P_BYTES_CONSUMED)) &
			  BAM_MASK(BAM_P_EVNT_REGn, P_BYTES_CONSUMED));

	bamdev_write_reg(base, BAM_P_EVNT_REGn(pipe_num), value);
}

/**
  Check the state of the BAM Core.

  Check if the BAM is enabled and, if so, read its pipe count (the only
  capability this driver ever needs).

  @param[in]      base        BAM virtual base address
  @param[in,out]  num_pipes   Number of pipes implemented by the BAM

  @return
  Status of the BAM.

*/
static bam_status_type bamcore_check(bam_vaddr base, uint32_t *num_pipes)
{
	if (num_pipes == NULL) {
		return BAM_FAILED;
	}

	// Check if the bam is enabled before spending time reading the rest of
	// its capability registers
	if (0 == bamcore_bam_enabled(base, BAM_QCE_EE)) {
		EMSG("bamcore_check:BAM not enable bam_base:0X%x", base);
		return BAM_FAILED;
	}

	*num_pipes = bamdev_read_reg_field(
		base, BAM_NUM_PIPES, BAM_MNS(BAM_NUM_PIPES, BAM_NUM_PIPES));

	return BAM_SUCCESS;
}

/**
  Initialize a BAM device.

  This function initializes a BAM device.

  @param[in]  base            BAM virtual base address
  @param[in]  sum_thresh      summing threshold (global for all pipes)

  @return
  None.

  @dependencies
  BAM must be clocked.

  @sa
  Other_Relevant_APIS_or_Delete.
*/
static void bamcore_init(bam_vaddr base, uint32_t sum_thresh)
{
	//Reset the BAM
	bamdev_write_reg_field(base, BAM_CTRL, BAM_MNS(BAM_CTRL, BAM_SW_RST),
			       1);
	/* No delay needed */
	bamdev_write_reg_field(base, BAM_CTRL, BAM_MNS(BAM_CTRL, BAM_SW_RST),
			       0);

	// Clock gating is always enabled for this target
	bamdev_write_reg_field(base, BAM_CTRL,
			       BAM_MNS(BAM_CTRL, LOCAL_CLK_GATING), 1);
	//Enable the bam
	bamdev_write_reg_field(base, BAM_CTRL, BAM_MNS(BAM_CTRL, BAM_EN), 0x1);

	// Descriptor summing threshold
	bamdev_write_reg(base, BAM_DESC_CNT_TRSHLD, sum_thresh);

	//Config Bits for BAM
	bamdev_write_reg(base, BAM_CNFG_BITS, BAM_CNFG_BITS_VAL);

	/* No IRQ setup here: this port always runs BAM in polling mode
	 * (bam_irq is always 0 — see bam_init), so there is
	 * nothing to unmask at the BAM level.
	 */
	return;
}

/**
  Initializes a bam pipe.

  Optional_detailed_description_goes_here.

  @param[in]      base        BAM virtual base address
  @param[in]      pipe_num    Index of the pipe
  @param[in,out]  pipe_cfg    Configuration to be used

  @return
  Status of the operation.

*/
static bam_status_type bamcore_pipe_init(bam_vaddr base, uint32_t pipe_num,
					 pipe_desc_cfg_t *pipe_cfg)
{
	uint32_t lpae_supported = 0;
	// Reset the BAM pipe
	bamdev_write_reg(base, BAM_P_RSTn(pipe_num), 1);
	// make sure the reset write goes through
	bam_osal_memorybarrier();

	bamdev_write_reg(base, BAM_P_RSTn(pipe_num), 0);

	lpae_supported = bamcore_lpae_supported(base);

	bamdev_write_reg_field(base, BAM_P_CTRLn(pipe_num),
			       BAM_MNS(BAM_P_CTRLn, P_DIRECTION),
			       pipe_cfg->dir);
	bamdev_write_reg_field(base, BAM_P_CTRLn(pipe_num),
			       BAM_MNS(BAM_P_CTRLn, P_SYS_MODE),
			       pipe_cfg->mode);
	bamdev_write_reg_field(base, BAM_P_CTRLn(pipe_num),
			       BAM_MNS(BAM_P_CTRLn, P_SYS_STRM),
			       (pipe_cfg->options & BAM_O_STREAMING) ? 1 : 0);

	bamdev_write_reg(base, BAM_P_EVNT_GEN_TRSHLDn(pipe_num),
			 pipe_cfg->evt_thresh);

	bamdev_write_reg(base, BAM_P_DESC_FIFO_ADDRn(pipe_num),
			 ADDR_LPAE_LSB(pipe_cfg->desc_base_pa));

	if (lpae_supported) {
		bamdev_write_reg(base, BAM_P_DESC_FIFO_ADDR_MSBn(pipe_num),
				 ADDR_LPAE_MSB(pipe_cfg->desc_base_pa));
	}

	bamdev_write_reg_field(base, BAM_P_FIFO_SIZESn(pipe_num),
			       BAM_MNS(BAM_P_FIFO_SIZESn, P_DESC_FIFO_SIZE),
			       pipe_cfg->desc_size);

	/* inline bamcore_pipe_setlockgroup */
	bamdev_write_reg_field(base, BAM_P_CTRLn(pipe_num),
			       BAM_MNS(BAM_P_CTRLn, P_LOCK_GROUP),
			       pipe_cfg->lock_group);

	// Pipe Enable - at last
	bamdev_write_reg_field(base, BAM_P_CTRLn(pipe_num),
			       BAM_MNS(BAM_P_CTRLn, P_EN), 1);

	// Verify that the pipe was enabled successfully
	if (0 == bamdev_read_reg_field(base, BAM_P_CTRLn(pipe_num),
				       BAM_MNS(BAM_P_CTRLn, P_EN))) {
		EMSG("bamcore_pipe_init:BAM Pipe successfully not enable bam_base:0X%x",
		     base);
		return BAM_FAILED;
	}

	return BAM_SUCCESS;
}

/**
  De-initializes a BAM pipe.

  Disables the BAM pipe from further use.

  @param[in]  base        BAM virtual base address
  @param[in]  pipe_num    Index of the pipe

  @return
  Status of the operation.

*/
static bam_status_type bamcore_pipe_deinit(bam_vaddr base, uint32_t pipe_num)
{
	// Pipe Disable - at last
	bamdev_write_reg_field(base, BAM_P_CTRLn(pipe_num),
			       BAM_MNS(BAM_P_CTRLn, P_EN), 0);
	return BAM_SUCCESS;
}

/* Polls (and retires) at most one completed descriptor for a pipe.
 *
 * This is the single code path ce_bam.c actually uses: it always calls
 * bam_pipe_poll() with a non-NULL result pointer and only ever inspects
 * result->event, so there is no per-descriptor callback/user-data to
 * invoke or propagate here — just the real offset/toggle bookkeeping.
 */
static void bam_pipe_eot_handler(pipe_dev *pipe, bam_result_type *result)
{
	uint32_t endoffset, nwd_skip = 0;
	bam_iovec_type *iovec;

	result->event = BAM_EVENT_INVALID;

	// Unlikely. However, if the pipe is already disabled, simply return
	if ((pipe->bam->active_pipes_mask & (1 << pipe->pipe_num)) == 0) {
		return;
	}

	// Invalidate any cache line that might have been prefetched before reading
	bam_osal_cachecmd(BAM_CACHE_INVALIDATE,
			  pipe->pipe_cfg.desc_base_va + pipe->acked_offset,
			  sizeof(bam_iovec_type));

	iovec = ((bam_iovec_type *)(pipe->pipe_cfg.desc_base_va +
				    pipe->acked_offset));
	/* When polling, skip the NWD toggle wait for CMD descriptors: the
	 * SW_OFSTSn offset is updated by HW even without a toggle for
	 * command-only descriptors. This is applicable only for CE BAM.
	 */
	if (iovec->flags & BAM_IOVEC_FLAG_CMD) {
		nwd_skip = 1;
	}
	// Get the last descriptor completed by the pipe
	endoffset = bamcore_pipe_getdescreadoffset(pipe->bam->bam_va,
						   pipe->pipe_num);
	// check if we got back the nwd_done toggle
	while ((pipe->acked_offset != endoffset) &&
	       (nwd_skip || bamcore_pipe_nwd_done(pipe->bam->bam_va,
						  pipe->pipe_num))) {
		iovec = ((bam_iovec_type *)(pipe->pipe_cfg.desc_base_va +
					    pipe->acked_offset));
		pipe->acked_offset += BAM_DESC_SIZE;
		if (pipe->acked_offset == pipe->pipe_cfg.desc_size)
			pipe->acked_offset = 0;

		if ((iovec->flags & BAM_IOVEC_FLAG_EOT) &&
		    (pipe->pipe_cfg.options & BAM_O_EOT)) {
			result->event = BAM_EVENT_EOT;
		} else if ((iovec->flags & BAM_IOVEC_FLAG_INT) &&
			   (pipe->pipe_cfg.options & BAM_O_DESC_DONE)) {
			result->event = BAM_EVENT_DESC_DONE;
		} else {
			continue;
		}

		result->data.iovec = *iovec;
		// Return only the first result; the caller polls again for more.
		break;
	}
	return;
}

static bam_status_type
bami_pipe_transfer_no_lock(pipe_dev *pipe, bam_paddr buf_pa, uint64_t buf_size,
			   uint16_t xfer_opts)
{
	uint32_t next_write_offset;
	bam_iovec_type iovec;
	bam_status_type result = BAM_FAILED;

	next_write_offset = pipe->desc_offset + BAM_DESC_SIZE;
	if (next_write_offset >= pipe->pipe_cfg.desc_size)
		next_write_offset = 0;

	if (next_write_offset == pipe->acked_offset) {
		EMSG("bami_pipe_transfer_no_lock:BAM 0x%x:BAM out of descriptors next_Write_offset: %d, acked_offset %d",
		     pipe->bam->bam_pa, next_write_offset,
		     pipe->acked_offset);
		//TODO: Do we have to poll here?
		// Out of Descriptors
		goto exit;
	}

	// Validate descriptor flags
	// IF a command desriptor is being submitted make sure preceding DD has a NWD set
	if (xfer_opts & BAM_IOVEC_FLAG_CMD && pipe->last_offset != 0xFFFFFFFF &&
	    !(((bam_iovec_type *)(pipe->last_offset +
				  pipe->pipe_cfg.desc_base_va))
			  ->flags &
	      (BAM_IOVEC_FLAG_CMD | BAM_IOVEC_FLAG_NWD))) {
		EMSG("bami_pipe_transfer_no_lock:BAM 0x%x:DD descriptor preceding CMD must have NWD bit set",
		     pipe->bam->bam_pa);
		goto exit;
	}
	if (xfer_opts & BAM_IOVEC_FLAG_NWD &&
	    !(xfer_opts & (BAM_IOVEC_FLAG_CMD | BAM_IOVEC_FLAG_EOT))) {
		EMSG("bami_pipe_transfer_no_lock:BAM 0x%x: NWD must be paired with EOT or CMD descriptor,pipeno:%d",
		     pipe->bam->bam_pa, pipe->pipe_num);
		goto exit;
	}
	if (xfer_opts & BAM_IOVEC_FLAG_CMD &&
	    xfer_opts & BAM_IOVEC_FLAG_IMM_CMD) {
		EMSG("bami_pipe_transfer_no_lock:CMD descriptor cannot have immediate flag set,pipeno:%d",
		     pipe->bam->bam_pa, pipe->pipe_num);
		goto exit;
	}
	if (!bamcore_lpae_supported(pipe->bam->bam_va) &&
	    ADDR_LPAE_MSB(buf_pa)) {
		EMSG("bami_pipe_transfer_no_lock:BAM 0x%x: buffer address > 32bits in a non-LPAE BAM, pipeno %d MSB 0x%x",
		     pipe->bam->bam_pa, pipe->pipe_num,
		     ADDR_LPAE_MSB(buf_pa));
		goto exit;
	}

	// Create a descriptor
	iovec.buf_pa = (uint32_t)ADDR_LPAE_LSB(buf_pa);
	iovec.buf_pa_msb = ADDR_LPAE_MSB(buf_pa);
	iovec.buf_size = buf_size;
	iovec.flags = xfer_opts & BAM_IOVEC_HW_FLAGS;
	// Update hardware descriptor FIFO
	*((bam_iovec_type *)(pipe->pipe_cfg.desc_base_va + pipe->desc_offset)) =
		iovec;

	pipe->last_offset = pipe->desc_offset;

	// Move the desc offset
	pipe->desc_offset = next_write_offset;

	// Finally submit the command to the pipe if no_submit is NOT provided
	if (BAM_OPT_NOT_SET(xfer_opts, BAM_IOVEC_FLAG_NO_SUBMIT)) {
		// Ideally we should operate with uncached memory. But some systems may not
		// provision uncached memory
		bam_osal_cachecmd(BAM_CACHE_FLUSH, pipe->pipe_cfg.desc_base_va,
				  pipe->pipe_cfg.desc_size);
		bam_osal_memorybarrier();
		// read back of descriptor that is updated in desc fifo to make sure that descriptor is not stuck at NOC level
		iovec_debug = *((bam_iovec_type *)(pipe->pipe_cfg.desc_base_va +
						   pipe->last_offset));
		bamcore_pipe_setdescwriteoffset(pipe->bam->bam_va,
						pipe->pipe_num,
						next_write_offset, 0);
		bam_osal_memorybarrier();
		evnt_ofst_readback = bamcore_pipe_getdescwriteoffset(
			pipe->bam->bam_va, pipe->pipe_num);
	}
	result = BAM_SUCCESS;
exit:

	return result;
}

/* ============================================================================
**  Public API
** ============================================================================
*/

/**
 * @brief Initialize BAM and configures it.
 * Initializes the BAM. Allocates a handle to the BAM,
 * which the client is expected to use for future calls to the driver.
 *
 * @param *handle - Pointer to be allocated for the BAM handle
 * @param bam_cfg  - Configuration data for BAM. This is peripheral specific
 * @param bam_cb - Callback to get any top level BAM events
 *
 * @return A handle to the BAM Device if successful
 *
 * @note There is exactly one BAM instance in this port, with exactly one
 *       client (the CE HAL, serialized upstream by a single mutex), so
 *       this simply allocates that one bam_dev the first time it's called
 *       and hands back the same handle after
 *       that — no device pool, no client list, no shared-config checks.
*/
bam_handle bam_init(bam_config_type *bam_cfg, bam_callback_type *bam_cb)
{
	bam_osal_meminfo bam_mem = { 0 };
	bam_dev *bam;

	(void)bam_cb; /* no top-level BAM callback is ever installed (ce_bam.c always passes func=NULL) */

	if (bams != NULL)
		return (bam_handle)bams;

	if (bam_cfg->bam_va == 0x0) {
		bam_mem.pa = bam_cfg->bam_pa;
		bam_mem.va = 0;
		bam_mem.size = BAM_MAX_MMAP;
		bam_osal_mallocmemregion(&bam_mem, bam_mem.size,
					 BAM_DEVICE_MAPPING);
		if (bam_mem.handle == NULL) {
			EMSG("bam_init:BAM 0x%x PA to VA mapping failed",
			     bam_cfg->bam_pa);
			return 0;
		}
		bam_cfg->bam_va = bam_mem.va;
	}

	bam = (bam_dev *)bam_osal_malloc(sizeof(bam_dev));
	if (NULL == bam) {
		EMSG("bam_init:BAM 0x%x: Allocating memory for bam_dev failed size:%d bytes",
		     bam_cfg->bam_pa, sizeof(bam_dev));
		return (bam_handle)NULL;
	}
	memset(bam, 0x0, sizeof(bam_dev));

	bam->bam_pa = bam_cfg->bam_pa;
	bam->bam_va = bam_cfg->bam_va;

	if (!bamcore_bam_enabled(bam_cfg->bam_va, BAM_EE_TRUST))
		bamcore_init(bam_cfg->bam_va, bam_cfg->sum_thresh);

	if (bamcore_check(bam->bam_va, &bam->num_pipes) != BAM_SUCCESS) {
		// Note: if the clocks were off, the num_pipes returned by bamcore_check could be zero
		// In that case, we may run into issues if the pipes were used later on by a driver
		//Either enable failed or BAM isnt enabled at cold boot.
		EMSG("bam_init:BAM 0x%x: Core Check Failed", bam_cfg->bam_pa);
		bam_osal_free(bam);
		return (bam_handle)NULL;
	}

	bam->pipes = (pipe_dev **)bam_osal_malloc(sizeof(pipe_dev *) *
						  bam->num_pipes);
	if (bam->pipes == NULL) {
		EMSG("bam_init:BAM 0x%x: pipe dev pool allocation failed, size:%d bytes",
		     bam_cfg->bam_pa, (sizeof(pipe_dev *) * bam->num_pipes));
		bam_osal_free(bam);
		return (bam_handle)NULL;
	}
	memset(bam->pipes, 0x0, sizeof(pipe_dev *) * bam->num_pipes);

	bams = bam;

	return (bam_handle)bam;
}

/**
 * @brief Initializes pipe pipe_num in the BAM based on the config provided.
 *
 * Enables pipe pipe_num in the BAM based on the pipe_cfg provided. Returns
 * a handle to the pipe allocated to the client to be used for future pipe
 * operations
 *
 * @param *bamhandle - Pointer to the handle to the BAM from which the pipe
 *                     is to be allocated
 * @param pipe_num - Pipe to be allocated in the bam.
 * @param bam_cfg  - Configuration data for the pipe. This is connection specific
 * @param pipe_cb  - Callback to be notified of Pipe level events
 *
 * @return A handle to the pipe allocated successfully
 *
*/
bam_handle bam_pipe_init(bam_handle bamhandle, uint32_t pipe_num,
			 bam_pipe_config_type *pipe_cfg,
			 bam_callback_type *pipe_cb)
{
	bam_dev *bam;
	pipe_dev *pipe;
	bam_status_type status;

	(void)pipe_cb; /* no pipe-level callback is ever installed in this port (always-polling) */

	//Sanity check
	if (bamhandle == NULL || pipe_cfg == NULL) {
		EMSG("bam_pipe_init: Invalid BAM Pipe init params, pipe:%d, pipe_cfg:0X%X, handle:0X%X",
		     pipe_num, pipe_cfg, bamhandle);
		return (bam_handle)NULL;
	}

	bam = (bam_dev *)bamhandle;
	if ((bam->active_pipes_mask & 1 << pipe_num) ||
	    (bam->num_pipes <= pipe_num)) {
		EMSG("bam_pipe_init:BAM 0x%x: Pipe already active (0x%x) or invalid pipe number (%d)",
		     bam->bam_pa, bam->active_pipes_mask, pipe_num);
		return (bam_handle)NULL;
	}
	if ((BAM_IS_INVALID_DESC_FIFO_SIZE(pipe_cfg->desc_size)) ||
	    (BAM_IS_DESC_FIFO_ALIGNED(pipe_cfg->desc_base_pa))) {
		EMSG("bam_pipe_init:BAM 0x%x: Desc fifo or Desc fifo size not aligned to 8 byte boundary Desc pa:0x%x, Desc fio size:0x%x",
		     bam->bam_pa, pipe_cfg->desc_base_pa,
		     pipe_cfg->desc_size);
		return (bam_handle)NULL;
	}

	pipe = (pipe_dev *)bam_osal_malloc(sizeof(pipe_dev));
	if (pipe == NULL) {
		EMSG("bam_pipe_init:BAM 0x%x: Pipe dev allocation failed, size:%d bytes",
		     bam->bam_pa, sizeof(pipe_dev));
		return (bam_handle)NULL;
	}
	memset(pipe, 0x0, sizeof(pipe_dev));

	pipe->pipe_cfg.options = pipe_cfg->options;
	pipe->pipe_cfg.dir = pipe_cfg->dir;
	pipe->pipe_cfg.mode = pipe_cfg->mode;
	pipe->pipe_cfg.desc_base_va = pipe_cfg->desc_base_va;
	pipe->pipe_cfg.desc_base_pa = pipe_cfg->desc_base_pa;
	pipe->pipe_cfg.desc_size = pipe_cfg->desc_size;
	pipe->pipe_cfg.evt_thresh = pipe_cfg->evt_thresh;
	pipe->pipe_cfg.lock_group = pipe_cfg->lock_group;

	status = BAM_SUCCESS;
	if (BAM_OPT_NOT_SET(pipe->pipe_cfg.options, BAM_O_PIPE_NO_INIT)) {
		status = bamcore_pipe_init(bam->bam_va, pipe_num,
					   &pipe->pipe_cfg);
	}
	if (status != BAM_SUCCESS) {
		bam_osal_free(pipe);
		return (bam_handle)NULL;
	}

	pipe->bam = bam;
	pipe->pipe_num = pipe_num;
	pipe->last_offset = 0xFFFFFFFF;
	bam->active_pipes_mask |= (1 << pipe_num);
	bam->pipes[pipe_num] = pipe;

	// Map the descriptor VA if it hasn't been mapped and if the options does not
	// contain no-init
	if (BAM_OPT_NOT_SET(pipe->pipe_cfg.options, BAM_O_PIPE_NO_INIT)) {
		if (pipe->pipe_cfg.desc_base_va == 0) {
			bam_osal_meminfo desc_mem = { 0 };

			desc_mem.pa = pipe_cfg->desc_base_pa;
			desc_mem.size = pipe_cfg->desc_size;
			bam_osal_mallocmemregion(&desc_mem, desc_mem.size,
						 BAM_MEMORY_MAPPING);
			if (desc_mem.handle == NULL) {
				EMSG("bam_pipe_init:BAM 0x%x: Descriptor fifo PA to VA mapping failed",
				     pipe_cfg->desc_base_pa);
				bam->pipes[pipe_num] = NULL;
				bam->active_pipes_mask &= ~(1 << pipe_num);
				bamcore_pipe_deinit(bam->bam_va,
						   pipe_num);
				bam_osal_free(pipe);
				return (bam_handle)NULL;
			}
			pipe->pipe_cfg.desc_base_va = desc_mem.va;
		}
		// clear the desc fifo
		memset((void *)pipe->pipe_cfg.desc_base_va, 0,
		       pipe->pipe_cfg.desc_size);
	}

	return (bam_handle)pipe;
}

/**
 * @brief Uninitializes the pipe.
 *
 * Uninitializes a pipe and disables it. No further operations
 * can be performed on the pipe, and the handle is invalidated.
 *
 * @param *pipehandle - handle to the pipe to be uninitialized
 *
 * @return Status of the operation
 *
*/
bam_status_type bam_pipe_deinit(bam_handle pipehandle)
{
	pipe_dev *pipe = (pipe_dev *)pipehandle;
	bam_dev *bam;

	if (pipe == NULL)
		return BAM_FAILED;

	bam = pipe->bam;
	if (bam == NULL)
		return BAM_FAILED;

	if (BAM_OPT_NOT_SET(pipe->pipe_cfg.options, BAM_O_PIPE_NO_INIT))
		bamcore_pipe_deinit(bam->bam_va, pipe->pipe_num);

	bam->pipes[pipe->pipe_num] = NULL;
	bam->active_pipes_mask &= ~(1 << pipe->pipe_num);

	bam_osal_free(pipe);
	return BAM_SUCCESS;
}

/**
 * @brief Enables a pipe.
 *
 * Enables a pipe for transfers
 *
 * @param *pipehandle - handle to the pipe to be enabled
 *
 * @return Status of the operation
 *
 * @dependencies bam_pipe_init must have succeeded
 *
*/
bam_status_type bam_pipe_enable(bam_handle pipehandle)
{
	pipe_dev *pipe = (pipe_dev *)pipehandle;
	bam_dev *bam;
	bam_status_type status = BAM_SUCCESS;

	if (pipe == NULL) {
		EMSG("bam_pipe_enable:Invalid BAM pipe handle, handle:0X%x",
		     pipe);
		return BAM_FAILED;
	}

	bam = pipe->bam;
	if (BAM_OPT_NOT_SET(pipe->pipe_cfg.options, BAM_O_PIPE_NO_INIT)) {
		status = bamcore_pipe_init(bam->bam_va, pipe->pipe_num,
					   &pipe->pipe_cfg);
		if (status != BAM_SUCCESS) {
			EMSG("bam_pipe_enable:BAM 0x%x: BamCore pipe init failed, pipe:%d",
			     bam->bam_pa, pipe->pipe_num);
			return status;
		}
	}
	//Reset the state variables
	pipe->acked_offset = 0x0;
	pipe->desc_offset = 0x0;
	bam->active_pipes_mask |= (1 << pipe->pipe_num);

	return BAM_SUCCESS;
}

/**
 * @brief Enables/Disables the IRQ mode of a pipe.
 *
 * @param *handle - handle to the pipe
 * @param irq_en  - Enable (1)/ Disable (0) the irq mode
 * @param pipe_cfg_opts - IRQs to be enabled for the BAM. See bam_options_type
 *
 * @return Status of the operation
 *
 * @note This port always runs BAM in polling mode (bam_init is always
 *       called with bam_irq == 0), so there is nothing to
 *       program at the HW level here — kept only so ce_bam.c's call sites
 *       (which always pass irq_en = 0) keep compiling/working unchanged.
*/
bam_status_type bam_pipe_setirqmode(bam_handle pipehandle, uint32_t irq_en,
				    uint32_t pipe_cfg_opts)
{
	pipe_dev *pipe = (pipe_dev *)pipehandle;

	(void)irq_en;
	(void)pipe_cfg_opts;

	if (pipe == NULL) {
		EMSG("bam_pipe_setirqmode:Invalid BAM pipe handle handle:0x%x",
		     pipe);
		return BAM_FAILED;
	}

	return BAM_SUCCESS;
}

/**
 * @brief Perform a single DMA transfer via the BAM pipe
 *
 * @details
 *
 * This function submits a DMA transfer request consisting of a single buffer
 * for a BAM pipe associated with a peripheral-to/from-memory
 * connection.
 *
 * The data buffer must be DMA ready.  The client is responsible for insuring
 * physically contiguous memory, cache maintenance, and memory barrier.
 *
 * The client must not modify the data buffer until the completion indication is
 * received.
 *
 * This function cannot be used if transfer queuing is disabled (see option
 * BAM_O_NO_Q).  The client must set the BAM_O_EOT option to receive a callback
 * event trigger when the transfer is complete.  The BAM driver will insure the
 * appropriate flags in the I/O vectors are set to generate the completion
 * indication.
 *
 * The return value from this function may indicate that an error occurred.
 * Possible causes include invalid arguments.
 *
 * @param pipe_handle - Handle to the pipe
 *
 * @param buf_pa - Physical address of buffer to transfer
 *
 * @param buf_size - Size in bytes of buffer to transfer
 *
 * @param xfer_opts - OR'd I/O vector option flags (see BAM_IOVEC_FLAG defines)
 *
 * @param user_data - User pointer that would be returned to user as part of
 *    event payload — not used in this port: ce_bam.c always polls with an
 *    explicit result pointer and only inspects its event field, so there is
 *    no per-descriptor callback data to track.
 *
 * @return Status of the operation
 *
 */
bam_status_type bam_pipe_transfer(bam_handle pipehandle, bam_paddr buf_pa,
				  uint64_t buf_size, uint16_t xfer_opts,
				  void *user_data)
{
	pipe_dev *pipe = (pipe_dev *)pipehandle;
	bam_status_type result = BAM_FAILED;

	(void)user_data;

	if (pipe == NULL) {
		EMSG("bam_pipe_transfer: Invalid BAM pipe handle, handle:0x%x buf_pa: 0x%xl",
		     pipe, buf_pa);
		return BAM_FAILED;
	}

	if (pipe->pipe_cfg.mode == BAM_MODE_BAM2BAM) {
		EMSG("bam_pipe_transfer:BAM 0x%x: BAM2BAM mode, cannot call pipe transfer,",
		     pipe->bam->bam_pa);
		return BAM_FAILED;
	}
	if (!bamcore_pipe_enabled(pipe->bam->bam_va, pipe->pipe_num)) {
		EMSG("bam_pipe_transfer: BAM pipe not enabled, bam:0x%x pipe no: %d",
		     pipe->bam->bam_pa, pipe->pipe_num);
		return BAM_FAILED;
	}
	result = bami_pipe_transfer_no_lock(pipe, buf_pa, buf_size, xfer_opts);

	return result;
}

/**
 * @brief Polls for a result
 *
 * This function polls the pipe for a result and retires at most one
 * completed descriptor into it.
 *
 * @param pipe_handle - Pointer to client's handle to the BAM Pipe
 * @param[out] result - Pointer to the structure that holds the result.
 *
 * @return Status of the operation.
 *
 * @note The Pipe should be operating in polling mode
 *
 */
bam_status_type bam_pipe_poll(bam_handle pipehandle, bam_result_type *result)
{
	pipe_dev *pipe = (pipe_dev *)pipehandle;

	if (pipe == NULL || result == NULL) {
		EMSG("bam_pipe_poll:Invalid BAM pipe handle, pipe:0X%x", pipe);
		return BAM_FAILED;
	}

	bam_pipe_eot_handler(pipe, result);

	return BAM_SUCCESS;
}

/* ============================================================================
**  Function : bam_fill_ce
** ============================================================================
*/
/**
  Fills a command element for BAM CMD descriptor mode.
  Used by ce_bam.c in uclib for BAM CMD mode operations.

  @param[in]  ce_base   Virtual base address of CE
  @param[in]  index     Index of the command element
  @param[in]  reg       Register physical address
  @param[in]  cmd       Command type
  @param[in]  data      Data to write
  @param[in]  mask      Mask for the data

  @return None.
*/
void bam_fill_ce(bam_vaddr ce_base, uint32_t index, bam_paddr reg,
		 bam_ce_oper_type cmd, uint64_t data, uint32_t mask)
{
	bam_ce_type *ce = (bam_ce_type *)ce_base + index;
	ce->reg_addr = (uint32_t)reg & 0xFFFFFF;
	ce->command = cmd;
	ce->data = (uint32_t)data;
	ce->mask = mask;
	ce->reserved = 0;
}
