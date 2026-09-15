// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * BAM driver definitions.
 *
 * Defines BAM data types, descriptor formats, pipe
 * configuration structures, events, and driver APIs
 * used by the Qualcomm Crypto Engine BAM transport.
 */

#ifndef BAM_H_
#define BAM_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	BAM_SUCCESS = 0x0,
	BAM_FAILED = 0x1,
} bam_status_type;

typedef enum {
	BAM_O_DESC_DONE = 0x00000001,
	BAM_O_EOT = 0x00000020,
	BAM_O_STREAMING = 0x00010000,
	BAM_O_PIPE_NO_INIT = 0x04000000,
} bam_options_type;

typedef enum {
	BAM_EVENT_INVALID = 0,
	BAM_EVENT_EOT,
	BAM_EVENT_DESC_DONE,
} bam_event_type;

typedef enum {
	BAM_IOVEC_FLAG_INT = 0x80,
	BAM_IOVEC_FLAG_EOT = 0x40,
	BAM_IOVEC_FLAG_EOB = 0x20,
	BAM_IOVEC_FLAG_NWD = 0x10,
	BAM_IOVEC_FLAG_CMD = 0x08,
	BAM_IOVEC_FLAG_LCK = 0x04,
	BAM_IOVEC_FLAG_UNLCK = 0x02,
	BAM_IOVEC_FLAG_IMM_CMD = 0x01,
	BAM_IOVEC_FLAG_NO_SUBMIT = 0x100
} bam_iovec_options_type;

typedef enum { BAM_DIR_PRODUCER = 0x1, BAM_DIR_CONSUMER = 0x0 } bam_dir_type;
typedef enum { BAM_MODE_BAM2BAM = 0x0, BAM_MODE_SYSTEM = 0x1 } bam_mode_type;
typedef enum {
	BAM_IRQ_HRESP_ERR_EN = 0x2,
	BAM_IRQ_ERR_EN = 0x4,
} bam_irq_options_type;
typedef enum {
	BAM_CE_OP_WRITE = 0,
	BAM_CE_OP_WRITE_LITE = 0x4
} bam_ce_oper_type;

typedef uint64_t bam_vaddr;
typedef uint64_t bam_paddr;

typedef enum { BAM_CACHE_INVALIDATE=0x0, BAM_CACHE_FLUSH=0x1 } bam_cache_op_type;
typedef enum { BAM_DEVICE_MAPPING=0x0, BAM_MEMORY_MAPPING=0x1 } bam_mapping_op_type;
typedef struct { uint64_t pa; bam_vaddr va; uint32_t size; void *handle; } bam_osal_meminfo;

typedef struct _bamconfig {
	bam_paddr bam_pa;
	bam_vaddr bam_va;
	uint32_t bam_irq;
	uint32_t bam_irq_mask;
	uint32_t sum_thresh;
	uint32_t options;
	uint32_t bam_mti_irq_pa;
	void *data;
} bam_config_type;

typedef struct _PipeConfig {
	uint32_t options;
	bam_dir_type dir;
	bam_mode_type mode;
	bam_vaddr desc_base_va;
	bam_paddr desc_base_pa;
	uint32_t desc_size;
	uint16_t evt_thresh;
	uint32_t lock_group;
	bam_paddr peer_base_pa;
	bam_paddr data_base_pa;
	uint32_t peer_pipe_num;
	uint32_t data_size;
} bam_pipe_config_type;

typedef struct {
	uint32_t buf_pa;
	uint32_t buf_size : 16;
	uint32_t buf_pa_msb : 8;
	uint32_t flags : 8;
} bam_iovec_type;

typedef struct {
	uint32_t reg_addr : 24;
	uint32_t command : 8;
	uint32_t data;
	uint32_t mask;
	uint32_t reserved;
} bam_ce_type;

typedef struct _bam_result_type {
	void *cb_data;
	bam_event_type event;
	struct {
		bam_iovec_type iovec;
	} data;
} bam_result_type;

typedef void *bam_handle;
typedef void (*bam_callback_func_type)(bam_result_type bam_result);
typedef struct {
	bam_callback_func_type func;
	void *data;
} bam_callback_type;

void bam_fill_ce(bam_vaddr ce_base, uint32_t index, bam_paddr reg,
		 bam_ce_oper_type cmd, uint64_t data, uint32_t mask);
#define BAM_FILL_CE(CE_BASE, INDEX, REG, CMD, DATA, MASK) \
	bam_fill_ce((CE_BASE), INDEX, REG, CMD, DATA, MASK)

bam_handle bam_init(bam_config_type *bam_cfg, bam_callback_type *bam_cb);
bam_handle bam_pipe_init(bam_handle bamhandle, uint32_t pipe_num,
			 bam_pipe_config_type *pipe_cfg,
			 bam_callback_type *pipe_cb);
bam_status_type bam_pipe_deinit(bam_handle pipehandle);
bam_status_type bam_pipe_enable(bam_handle pipehandle);
bam_status_type bam_pipe_setirqmode(bam_handle pipehandle, uint32_t irq_en,
				    uint32_t pipe_cfg_opts);
bam_status_type bam_pipe_transfer(bam_handle pipehandle, bam_paddr buf_pa,
				  uint64_t buf_size, uint16_t xfer_opts,
				  void *user_data);
bam_status_type bam_pipe_poll(bam_handle pipehandle, bam_result_type *result);

#define BAM_MAX_MMAP 0x2800
#define BAM_EE_TRUST 3
#define BAM_OPT_NOT_SET(options, flag) (!((options) & (flag)))

#define BAM_DESC_SIZE 8 // Size of a descriptor in bytes

//extract the MSB 4 bits of a 36 bit & 8 bits of a 40 bit LPAE address
#define ADDR_LPAE_MSB(x) ((((uint64_t)(x)) >> 32) & 0xFF)

//extract the LSB 32 bits of a 36 bit address
#define ADDR_LPAE_LSB(x) ((uint32_t)((x) & 0xFFFFFFFF))

/*
 * BAM register offsets — derived from IPCAT (CRYPTO0_CRYPTO_BAM_REG_BASE + offset)
 * Used by bams.c as: bamdev_read_reg(base, BAM_CTRL)
 * where base = virtual address of BAM register space
 */

/* Non-indexed register offsets */
#define BAM_CTRL              0x0
#define BAM_NUM_PIPES         0x1008
#define BAM_CNFG_BITS         0x7c
#define BAM_DESC_CNT_TRSHLD   0x8

/* Indexed register offsets — pipe-indexed (stride 0x1000) */
#define BAM_PIPE_ATTR_EEn(ee)           (0x300C + (0x1000 * (ee)))
#define BAM_P_CTRLn(pipe)               (0x13000 + (0x1000 * (pipe)))
#define BAM_P_RSTn(pipe)                (0x13004 + (0x1000 * (pipe)))
#define BAM_P_CNSMR_SDBNDn(pipe)        (0x13028 + (0x1000 * (pipe)))
#define BAM_P_SW_OFSTSn(pipe)           (0x13800 + (0x1000 * (pipe)))
#define BAM_P_EVNT_REGn(pipe)           (0x13818 + (0x1000 * (pipe)))
#define BAM_P_DESC_FIFO_ADDRn(pipe)     (0x1381C + (0x1000 * (pipe)))
#define BAM_P_FIFO_SIZESn(pipe)         (0x13820 + (0x1000 * (pipe)))
#define BAM_P_EVNT_GEN_TRSHLDn(pipe)   (0x13828 + (0x1000 * (pipe)))
#define BAM_P_DESC_FIFO_ADDR_MSBn(pipe) (0x13914 + (0x1000 * (pipe)))

/*
 * BAM register field masks and shifts.
 * Naming: <REG>_<FIELD>_MASK / <REG>_<FIELD>_SHFT
 * Used by: BAM_MNS(reg, field), BAM_MASK(reg, field), BAM_SHFT(reg, field)
 */

/* BAM_CTRL fields */
#define BAM_CTRL_LOCAL_CLK_GATING_MASK          0x60000
#define BAM_CTRL_LOCAL_CLK_GATING_SHFT          17
#define BAM_CTRL_BAM_EN_MASK                    0x2
#define BAM_CTRL_BAM_EN_SHFT                    1
#define BAM_CTRL_BAM_SW_RST_MASK                0x1
#define BAM_CTRL_BAM_SW_RST_SHFT                0

/* BAM_NUM_PIPES fields */
#define BAM_NUM_PIPES_BAM_DATA_ADDR_BUS_WIDTH_MASK  0xc000
#define BAM_NUM_PIPES_BAM_DATA_ADDR_BUS_WIDTH_SHFT  14
#define BAM_NUM_PIPES_BAM_NUM_PIPES_MASK            0xff
#define BAM_NUM_PIPES_BAM_NUM_PIPES_SHFT            0

/* BAM_PIPE_ATTR_EEn fields */
#define BAM_PIPE_ATTR_EEn_BAM_ENABLED_MASK      0x80000000
#define BAM_PIPE_ATTR_EEn_BAM_ENABLED_SHFT      31

/* BAM_P_CTRLn fields */
#define BAM_P_CTRLn_P_LOCK_GROUP_MASK           0x1f0000
#define BAM_P_CTRLn_P_LOCK_GROUP_SHFT           16
#define BAM_P_CTRLn_P_SYS_MODE_MASK             0x20
#define BAM_P_CTRLn_P_SYS_MODE_SHFT             5
#define BAM_P_CTRLn_P_SYS_STRM_MASK             0x10
#define BAM_P_CTRLn_P_SYS_STRM_SHFT             4
#define BAM_P_CTRLn_P_DIRECTION_MASK            0x8
#define BAM_P_CTRLn_P_DIRECTION_SHFT            3
#define BAM_P_CTRLn_P_EN_MASK                   0x2
#define BAM_P_CTRLn_P_EN_SHFT                   1

/* BAM_P_CNSMR_SDBNDn fields */
#define BAM_P_CNSMR_SDBNDn_BAM_P_CTRL_MASK     0xf0000
#define BAM_P_CNSMR_SDBNDn_BAM_P_CTRL_SHFT     16

/* BAM_P_SW_OFSTSn fields */
#define BAM_P_SW_OFSTSn_SW_DESC_OFST_MASK      0xffff
#define BAM_P_SW_OFSTSn_SW_DESC_OFST_SHFT      0

/* BAM_P_EVNT_REGn fields */
#define BAM_P_EVNT_REGn_P_BYTES_CONSUMED_MASK      0xffff0000
#define BAM_P_EVNT_REGn_P_BYTES_CONSUMED_SHFT      16
#define BAM_P_EVNT_REGn_P_DESC_FIFO_PEER_OFST_MASK 0xffff
#define BAM_P_EVNT_REGn_P_DESC_FIFO_PEER_OFST_SHFT 0

/* BAM_P_FIFO_SIZESn fields */
#define BAM_P_FIFO_SIZESn_P_DESC_FIFO_SIZE_MASK    0xffff
#define BAM_P_FIFO_SIZESn_P_DESC_FIFO_SIZE_SHFT    0

/** Mask 'n' Shift Macro */
#define BAM_MNS(reg, field)   reg##_##field##_MASK, reg##_##field##_SHFT

/** Mask macro */
#define BAM_MASK(reg, field)  (reg##_##field##_MASK)

/** Shift Macro */
#define BAM_SHFT(reg, field)  (reg##_##field##_SHFT)

#endif /* BAM_H_ */
