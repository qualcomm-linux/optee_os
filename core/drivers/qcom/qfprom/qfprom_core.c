// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <assert.h>
#include <initcall.h>
#include <inttypes.h>
#include <io.h>
#include <kernel/delay.h>
#include <kernel/panic.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <string.h>
#include <trace.h>
#include <utee_defines.h>
#include <util.h>

#include "qfprom_hal.h"
#include "qfprom_priv.h"
#include "qfprom_target.h"

register_phys_mem_pgdir(MEM_AREA_IO_SEC, TCSR_MUTEX_BASE, CORE_MMU_PGDIR_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, SECURITY_CONTROL_BASE,
			CORE_MMU_PGDIR_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, TCSR_SOC_HW_VERSION_ADDR,
			CORE_MMU_PGDIR_SIZE);

static struct qfprom_context ctx;

struct qfprom_context *qfprom_get_context(void)
{
	return &ctx;
}

static bool is_address_valid(uint32_t addr,
			     enum qfprom_addr_space type)
{
	struct qfprom_context *qfprom_ctx = qfprom_get_context();
	uint32_t lookup_addr = 0;
	paddr_t base = 0;
	paddr_t end = 0;

	if (!qfprom_ctx || !qfprom_ctx->config ||
	    !IS_ALIGNED_WITH_TYPE(addr, uint64_t))
		return false;

	lookup_addr = addr;
	if (type == QFPROM_ADDR_SPACE_CORR)
		lookup_addr = QFPROM_RAW_TO_CORR(addr);

	if (type == QFPROM_ADDR_SPACE_RAW)
		base = qfprom_ctx->config->qfprom_raw_base;
	else
		base = qfprom_ctx->config->qfprom_corr_base;

	end = base + qfprom_ctx->config->qfprom_size;
	if (lookup_addr < base || lookup_addr >= end)
		return false;

	return true;
}

static enum qfprom_error get_region_name(uint32_t addr,
					 enum qfprom_addr_space addr_type,
					 enum qfprom_region_name *region_name)
{
	struct qfprom_context *drv = qfprom_get_context();
	uint32_t region_start = 0;
	uint32_t lookup_addr = 0;
	uint32_t region_end = 0;
	size_t i = 0;

	if (!region_name)
		return QFPROM_DATA_PTR_NULL_ERR;

	if (!drv->config || !drv->config->region_data)
		return QFPROM_ERR_UNKNOWN;

	if (!is_address_valid(addr, addr_type))
		return QFPROM_ADDRESS_INVALID_ERR;

	lookup_addr = (addr_type == QFPROM_ADDR_SPACE_CORR) ?
		      QFPROM_RAW_TO_CORR(addr) : addr;

	for (i = 0; i < drv->config->num_regions; i++) {
		const struct qfprom_region_info *region =
			&drv->config->region_data[i];
		uint32_t offset = 0;

		if (addr_type == QFPROM_ADDR_SPACE_RAW)
			region_start = region->raw_base_addr;
		else if (addr_type == QFPROM_ADDR_SPACE_CORR)
			region_start = region->corr_base_addr;
		else
			return QFPROM_ERR_UNKNOWN;

		region_end = region_start + (region->size * 8);

		if (lookup_addr < region_start)
			continue;

		if (lookup_addr >= region_end)
			continue;

		offset = lookup_addr - region_start;
		if (offset & 7)
			return QFPROM_ADDRESS_INVALID_ERR;

		if ((offset >> 3) >= region->size)
			return QFPROM_ADDRESS_INVALID_ERR;

		*region_name = region->region_name;
		return QFPROM_NO_ERR;
	}

	return QFPROM_REGION_NOT_SUPPORTED_ERR;
}

static int read_row(uint32_t addr,
		    enum qfprom_addr_space type,
		    uint32_t *data)
{
	if (!data)
		return QFPROM_DATA_PTR_NULL_ERR;

	if (type == QFPROM_ADDR_SPACE_RAW)
		return hal_qfprom_read_raw_address_row(addr, data);

	addr = QFPROM_RAW_TO_CORR(addr);
	return hal_qfprom_read_corrected_address_row(addr, data);
}

static enum qfprom_error is_fec_enabled(enum qfprom_region_name region_name,
					bool *fec_status)
{
	struct qfprom_context *drv = qfprom_get_context();
	const struct qfprom_region_info *info = NULL;
	enum qfprom_error err = QFPROM_NO_ERR;
	paddr_t reg_addr = 0;
	uint32_t val = 0;
	uint32_t bit = 0;
	size_t i = 0;

	if (!fec_status)
		return QFPROM_DATA_PTR_NULL_ERR;

	if (!drv->config || !drv->config->region_data)
		return QFPROM_ERR_UNKNOWN;

	for (i = 0; i < drv->config->num_regions; i++) {
		if (drv->config->region_data[i].region_name == region_name) {
			info = &drv->config->region_data[i];
			break;
		}
	}

	if (!info)
		return QFPROM_REGION_NOT_SUPPORTED_ERR;

	if (info->fec_type == QFPROM_FEC_NONE) {
		*fec_status = false;
		return QFPROM_NO_ERR;
	}

	if (info->fec_type != QFPROM_FEC_63_56 ||
	    info->region_index >= QFPROM_FEC_REGION_MSB_MAX)
		return QFPROM_ERR_UNKNOWN;

	reg_addr = QFPROM_RAW_TO_CORR(FEC_ENABLES_ADDR);
	if (info->region_index >= QFPROM_FEC_REGION_LSB_MAX) {
		reg_addr += 4;
		bit = info->region_index - QFPROM_FEC_REGION_LSB_MAX;
	} else {
		bit = info->region_index;
	}

	err = hal_qfprom_read_corrected_address(reg_addr, &val);
	if (err != QFPROM_NO_ERR) {
		EMSG("Failed to read FEC enable register at 0x%lx, error: %d",
		     reg_addr, err);
		return err;
	}

	*fec_status = !!(val & BIT32(bit));

	return QFPROM_NO_ERR;
}

static bool check_region_access(enum qfprom_region_name region_name,
				uint32_t perm_flags)
{
	struct qfprom_context *drv = qfprom_get_context();
	const struct qfprom_region_info *info = NULL;
	paddr_t perm_addr = 0;
	uint32_t offset = 0;
	uint32_t perm = 0;
	size_t i = 0;

	if (!drv->config || !drv->config->region_data)
		return false;

	for (i = 0; i < drv->config->num_regions; i++) {
		if (drv->config->region_data[i].region_name == region_name) {
			info = &drv->config->region_data[i];
			break;
		}
	}

	if (!info)
		return false;

	/* Check read permission */
	if (perm_flags & REGION_PERM_READ) {
		offset = (info->perm_reg_type == QFPROM_ROW_LSB) ?
			 drv->config->read_perm_lsb_offset :
			 drv->config->read_perm_msb_offset;
		if (!offset) {
			if (!info->read_allowed)
				return false;
		} else {
			perm_addr = drv->config->qfprom_corr_base + offset;
			if (hal_qfprom_read_corrected_address(perm_addr, &perm))
				return false;

			if (perm & info->read_perm_mask)
				return false;
		}
	}

	/* Check write permission */
	if (!(perm_flags & REGION_PERM_WRITE))
		return true;

	offset = (info->perm_reg_type == QFPROM_ROW_LSB) ?
		 drv->config->write_perm_lsb_offset :
		 drv->config->write_perm_msb_offset;
	if (!offset)
		return false;

	perm_addr = drv->config->qfprom_corr_base + offset;
	if (hal_qfprom_read_corrected_address(perm_addr, &perm))
		return false;

	if (perm & info->write_perm_mask)
		return false;

	return true;
}

static enum qfprom_error wait_blow_status_ready(void)
{
	uint64_t timer = timeout_init_us(QFPROM_BLOW_TIMEOUT_US);
	enum qfprom_error err = QFPROM_NO_ERR;
	uint32_t status = 0;

	while (true) {
		err = hal_qfprom_read_blow_status(&status);
		if (err != QFPROM_NO_ERR)
			return err;

		if (status != QFPROM_BLOW_STATUS_BUSY_VAL)
			break;

		if (timeout_elapsed(timer)) {
			EMSG("QFPROM blow operation timed out");
			return QFPROM_ERROR_TIMEOUT;
		}

		udelay(10);
	}

	if (status != QFPROM_BLOW_STATUS_READY_VAL)
		return QFPROM_WRITE_ERR;

	return QFPROM_NO_ERR;
}

static enum qfprom_error raw_write(uint32_t addr,
				   const uint32_t *data)
{
	enum qfprom_region_name region_name = QFPROM_LAST_REGION_DUMMY;
	enum qfprom_error err = QFPROM_NO_ERR;
	bool fec_enabled = false;
	uint32_t verify[2] = {0};

	if (!data)
		return QFPROM_DATA_PTR_NULL_ERR;

	err = get_region_name(addr, QFPROM_ADDR_SPACE_RAW, &region_name);
	if (err != QFPROM_NO_ERR)
		return err;

	if (!check_region_access(region_name,
				 REGION_PERM_READ | REGION_PERM_WRITE))
		return QFPROM_REGION_NOT_WRITABLE_ERR;

	if (is_fec_enabled(region_name, &fec_enabled) || fec_enabled)
		return QFPROM_REGION_NOT_WRITABLE_ERR;

	err = wait_blow_status_ready();
	if (err != QFPROM_NO_ERR)
		return err;

	err = hal_qfprom_write_raw_address(addr, data[0]);
	if (err != QFPROM_NO_ERR)
		return err;

	err = wait_blow_status_ready();
	if (err != QFPROM_NO_ERR)
		return err;

	err = hal_qfprom_write_raw_address(addr + 4, data[1]);
	if (err != QFPROM_NO_ERR)
		return err;

	err = wait_blow_status_ready();
	if (err != QFPROM_NO_ERR)
		return err;

	if (!check_region_access(region_name, REGION_PERM_READ))
		return QFPROM_NO_ERR;

	err = read_row(addr, QFPROM_ADDR_SPACE_RAW, verify);
	if (err != QFPROM_NO_ERR)
		return QFPROM_NO_ERR;

	if ((verify[0] & data[0]) != data[0] ||
	    (verify[1] & data[1]) != data[1])
		return QFPROM_WRITE_ERR;

	return QFPROM_NO_ERR;
}

TEE_Result qfprom_read_row(uint32_t addr,
			   enum qfprom_addr_space type,
			   uint32_t *data)
{
	enum qfprom_region_name region_name = QFPROM_LAST_REGION_DUMMY;
	enum qfprom_error err = QFPROM_NO_ERR;

	if (!data)
		return TEE_ERROR_BAD_PARAMETERS;

	err = get_region_name(addr, type, &region_name);
	if (err == QFPROM_ADDRESS_INVALID_ERR)
		return TEE_ERROR_BAD_PARAMETERS;
	else if (err != QFPROM_NO_ERR)
		return TEE_ERROR_GENERIC;

	if (!check_region_access(region_name, REGION_PERM_READ))
		return TEE_ERROR_ACCESS_DENIED;

	err = read_row(addr, type, data);
	if (err != QFPROM_NO_ERR) {
		EMSG("QFPROM read failed for address 0x%08"PRIx32", error: %d",
		     addr, err);
		return TEE_ERROR_GENERIC;
	}

	if (type == QFPROM_ADDR_SPACE_CORR) {
		bool fec_enabled = false;

		err = is_fec_enabled(region_name, &fec_enabled);
		if (err != QFPROM_NO_ERR) {
			EMSG("FEC status check failed, err: %d", err);
			return TEE_ERROR_GENERIC;
		}

		if (fec_enabled && hal_qfprom_is_fec_error_seen()) {
			uint16_t err_addr = 0;

			hal_qfprom_read_error_address(&err_addr);
			EMSG("FEC error: 0x%04"PRIx16" req 0x%08"PRIx32,
			     err_addr, addr);
			hal_qfprom_clear_fec_error_status();
			return TEE_ERROR_CORRUPT_OBJECT;
		}
	}

	return TEE_SUCCESS;
}

TEE_Result qfprom_hw_init(void)
{
	struct qfprom_context *drv = qfprom_get_context();
	TEE_Result res = TEE_ERROR_GENERIC;

	res = qfprom_acquire_hw_mutex();
	if (res != TEE_SUCCESS)
		return res;

	res = qfprom_enable_voltage();
	if (res != TEE_SUCCESS)
		goto err_unlock;

	res = qfprom_write_set_clock_settings();
	if (res != TEE_SUCCESS)
		goto err_disable_voltage;

	drv->write_op_allowed = true;
	return TEE_SUCCESS;

err_disable_voltage:
	if (qfprom_disable_voltage() != TEE_SUCCESS)
		EMSG("Failed to disable voltage");
err_unlock:
	qfprom_release_hw_mutex();
	return res;
}

void qfprom_hw_deinit(void)
{
	struct qfprom_context *drv = qfprom_get_context();

	drv->write_op_allowed = false;
	qfprom_write_reset_clock_settings();
	if (qfprom_disable_voltage() != TEE_SUCCESS)
		EMSG("Failed to disable voltage");
	qfprom_release_hw_mutex();
}

TEE_Result qfprom_write_row(uint32_t addr, uint32_t *data)
{
	enum qfprom_error err = QFPROM_NO_ERR;
	uint32_t write_data[2] = {0};

	if (!data)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!IS_ENABLED(CFG_QFPROM_PROGRAMMING))
		return TEE_ERROR_NOT_SUPPORTED;

	write_data[0] = data[0];
	write_data[1] = data[1];

	err = raw_write(addr, write_data);
	if (err != QFPROM_NO_ERR) {
		EMSG("QFPROM write failed for address 0x%08"PRIx32", error: %d",
		     addr, err);
		return TEE_ERROR_GENERIC;
	}

	return TEE_SUCCESS;
}

TEE_Result qfprom_row_has_fec_bits(uint32_t addr,
				   enum qfprom_addr_space type,
				   uint8_t *has_fec)
{
	enum qfprom_region_name region_name = QFPROM_LAST_REGION_DUMMY;
	struct qfprom_context *drv = qfprom_get_context();
	enum qfprom_error err = QFPROM_NO_ERR;
	size_t i = 0;

	if (!has_fec)
		return TEE_ERROR_BAD_PARAMETERS;

	err = get_region_name(addr, type, &region_name);
	if (err == QFPROM_ADDRESS_INVALID_ERR)
		return TEE_ERROR_BAD_PARAMETERS;
	else if (err != QFPROM_NO_ERR)
		return TEE_ERROR_GENERIC;

	if (!drv->config || !drv->config->region_data)
		return TEE_ERROR_GENERIC;

	for (i = 0; i < drv->config->num_regions; i++) {
		const struct qfprom_region_info *region =
			&drv->config->region_data[i];

		if (region->region_name == region_name) {
			*has_fec = false;
			if (check_region_access(region_name, REGION_PERM_READ))
				*has_fec = region->fec_type != QFPROM_FEC_NONE;

			return TEE_SUCCESS;
		}
	}

	return TEE_ERROR_ITEM_NOT_FOUND;
}

uint32_t qfprom_fec_63_56_bit(uint32_t lsb_data, uint32_t msb_data)
{
	uint8_t lfsr[7] = {0};
	uint64_t data_loc = 0;
	uint32_t fec_val = 0;
	uint32_t temp = 0;
	int i = 0;

	data_loc = (SHIFT_U64(msb_data, 32)) | lsb_data;

	for (i = 0; i < 56; i++) {
		temp = lfsr[0] ^ ((data_loc >> i) & 0x1);

		lfsr[0] = lfsr[1] ^ temp;
		lfsr[1] = lfsr[2];
		lfsr[2] = lfsr[3];
		lfsr[3] = lfsr[4];
		lfsr[4] = lfsr[5] ^ temp;
		lfsr[5] = lfsr[6];
		lfsr[6] = temp;
	}

	for (i = 6; i >= 0; i--) {
		temp = SHIFT_U32(lfsr[i], i);
		fec_val = (fec_val | temp);
	}

	return (SHIFT_U32(fec_val, 24) | msb_data);
}

TEE_Result qcom_secboot_is_enabled(bool *enabled)
{
	struct qfprom_context *drv = qfprom_get_context();
	uint32_t val = 0;

	if (!enabled)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!drv->raw_base_va)
		return TEE_ERROR_BAD_STATE;

	val = io_read32(drv->raw_base_va +
			(SECURE_BOOT_APPS_ADDR - SECURITY_CONTROL_BASE));
	*enabled = (val & SECURE_BOOT_AUTH_EN_BMSK) != 0;

	return TEE_SUCCESS;
}

/*
 * Read the APPS SECURE_BOOTn USE_SERIAL_NUM bit: a device-fuse override
 * that forces serial-number binding regardless of the image metadata's
 * own serial-number-binding flag. Same register as
 * qcom_secboot_is_enabled(), different bit.
 */
TEE_Result qcom_secboot_get_use_serial_num(bool *enabled)
{
	struct qfprom_context *drv = qfprom_get_context();
	uint32_t val = 0;

	if (!enabled)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!drv->raw_base_va)
		return TEE_ERROR_BAD_STATE;

	val = io_read32(drv->raw_base_va +
			(SECURE_BOOT_APPS_ADDR - SECURITY_CONTROL_BASE));
	*enabled = (val & SECURE_BOOT_USE_SERIAL_NUM_BMSK) != 0;

	return TEE_SUCCESS;
}

/*
 * Read one 32-bit corrected QFPROM word directly via its mapped virtual
 * address. PIL_ARB fuses are not accessed through the region table (they use
 * the corrected address space and have no QFPROM_ADDR_SPACE_CORR region entry
 * for the EN fuse), so we skip the permission layer and read them directly.
 */
static TEE_Result read_corr_word(paddr_t pa, uint32_t mask, uint32_t *out)
{
	struct qfprom_context *drv = qfprom_get_context();
	vaddr_t va = 0;

	if (!drv->corr_base_va)
		return TEE_ERROR_BAD_STATE;

	va = drv->corr_base_va + (pa - QFPROM_CORR_BASE);
	*out = io_read32(va) & mask;
	return TEE_SUCCESS;
}

TEE_Result qcom_secboot_get_root_of_trust(uint8_t *hash, size_t len)
{
	paddr_t corr_base = QFPROM_RAW_TO_CORR(PK_HASH_0_ADDR);
	size_t off = 0;

	if (!hash)
		return TEE_ERROR_BAD_PARAMETERS;

	/* PK_HASH_0 holds the OEM root-of-trust digest (SHA-384, 48 bytes). */
	if (len != QFPROM_ROOT_OF_TRUST_BYTE_SIZE)
		return TEE_ERROR_BAD_PARAMETERS;

	/*
	 * qfprom_read_row() rejects addresses that are not 8-byte aligned
	 * (the corrected space is read in 64-bit rows). Use read_corr_word()
	 * which reads directly from the pre-mapped corrected VA and accepts
	 * any 4-byte-aligned offset within the QFPROM corrected window.
	 */
	for (off = 0; off < len; off += sizeof(uint32_t)) {
		TEE_Result res = TEE_ERROR_GENERIC;
		uint32_t word = 0;

		res = read_corr_word(corr_base + off, UINT32_MAX, &word);
		if (res)
			return res;

		memcpy(hash + off, &word, MIN(sizeof(word), len - off));
	}

	return TEE_SUCCESS;
}

static uint32_t popcount32(uint32_t v)
{
	v = v - ((v >> 1) & 0x55555555u);
	v = (v & 0x33333333u) + ((v >> 2) & 0x33333333u);
	return (((v + (v >> 4)) & 0x0f0f0f0fu) * 0x01010101u) >> 24;
}

TEE_Result qcom_secboot_get_pil_rollback_version(uint32_t *version)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	bool secboot = false;
	uint32_t lsb = 0;
	uint32_t msb = 0;
	uint32_t en = 0;

	if (!version)
		return TEE_ERROR_BAD_PARAMETERS;

	*version = 0;

	res = qcom_secboot_is_enabled(&secboot);
	if (res)
		return res;
	if (!secboot)
		return TEE_SUCCESS;

	res = read_corr_word(PIL_ARB_EN_CORR_ADDR, PIL_ARB_EN_BMSK, &en);
	if (res)
		return res;
	if (!en)
		return TEE_SUCCESS;

	res = read_corr_word(PIL_ARB_LSB_CORR_ADDR, PIL_ARB_LSB_BMSK, &lsb);
	if (res)
		return res;

	*version = popcount32(lsb);

	if (PIL_ARB_MSB_ENABLED) {
		res = read_corr_word(PIL_ARB_MSB_CORR_ADDR, PIL_ARB_MSB_BMSK,
				     &msb);
		if (res)
			return res;
		*version += popcount32(msb);
	}

	return TEE_SUCCESS;
}

/* Unary ("thermometer") mask of @n low bits: version N -> N set bits. */
static uint32_t unary_mask(uint32_t n)
{
	if (n >= 32)
		return 0xffffffffu;
	if (!n)
		return 0;
	return (uint32_t)((UINT64_C(1) << n) - 1);
}

TEE_Result qcom_secboot_blow_pil_rollback_version(uint32_t version)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t row[2] = { };
	uint32_t cur_lsb = 0;
	uint32_t cur_msb = 0;
	bool secboot = false;
	uint32_t lsb_n = 0;
	uint32_t msb_n = 0;
	uint32_t cur = 0;
	uint32_t en = 0;

	if (!IS_ENABLED(CFG_QFPROM_PROGRAMMING))
		return TEE_ERROR_NOT_SUPPORTED;

	/* Only advance the fuse when ARB enforcement is active. */
	res = qcom_secboot_is_enabled(&secboot);
	if (res)
		return res;
	if (!secboot)
		return TEE_SUCCESS;

	res = read_corr_word(PIL_ARB_EN_CORR_ADDR, PIL_ARB_EN_BMSK, &en);
	if (res)
		return res;
	if (!en)
		return TEE_SUCCESS;

	/* Current device version: popcount of the LSB and MSB, if used. */
	res = read_corr_word(PIL_ARB_LSB_CORR_ADDR, PIL_ARB_LSB_BMSK, &cur_lsb);
	if (res)
		return res;
	cur = popcount32(cur_lsb);
	if (PIL_ARB_MSB_ENABLED) {
		res = read_corr_word(PIL_ARB_MSB_CORR_ADDR, PIL_ARB_MSB_BMSK,
				     &cur_msb);
		if (res)
			return res;
		cur += popcount32(cur_msb);
	}

	/* Monotonic: never lower the version, nothing to do if already >=. */
	if (version <= cur)
		return TEE_SUCCESS;

	if (version > PIL_ARB_LSB_MAX_VERSION + PIL_ARB_MSB_MAX_VERSION) {
		EMSG("PAS ARB: version %"PRIu32" exceeds fuse capacity",
		     version);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	/*
	 * Unary-encode into the LSB bank first, overflow into the MSB bank.
	 * qfprom_write_row() only blows 0->1 bits, so OR the target masks with
	 * the current contents to preserve already-blown bits.
	 */
	lsb_n = MIN(version, (uint32_t)PIL_ARB_LSB_MAX_VERSION);
	if (version > PIL_ARB_LSB_MAX_VERSION)
		msb_n = version - PIL_ARB_LSB_MAX_VERSION;

	row[0] = cur_lsb | unary_mask(lsb_n);
	row[1] = cur_msb | unary_mask(msb_n);

	res = qfprom_write_row(PIL_ARB_RAW_ADDR, row);
	if (res) {
		EMSG("PAS ARB: fuse write failed: %#"PRIx32, res);
		return res;
	}

	DMSG("PAS ARB: advanced device version %"PRIu32" -> %"PRIu32, cur,
	     version);

	return TEE_SUCCESS;
}

/* Read a device-identity sense register from the mapped SECURITY_CONTROL. */
static TEE_Result read_sense_reg(paddr_t pa, uint32_t *out)
{
	struct qfprom_context *drv = qfprom_get_context();

	if (!drv->raw_base_va)
		return TEE_ERROR_BAD_STATE;

	*out = io_read32(drv->raw_base_va + (pa - SECURITY_CONTROL_BASE));

	return TEE_SUCCESS;
}

TEE_Result qcom_secboot_get_device_ids(struct qcom_secboot_device_ids *ids)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t val = 0;

	if (!ids)
		return TEE_ERROR_BAD_PARAMETERS;

	res = read_sense_reg(OEM_ID_SENSE_ADDR, &val);
	if (res)
		return res;
	ids->oem_id = (val & OEM_ID_BMSK) >> OEM_ID_SHFT;
	ids->model_id = (val & MODEL_ID_BMSK) >> MODEL_ID_SHFT;

	res = read_sense_reg(JTAG_ID_SENSE_ADDR, &val);
	if (res)
		return res;
	ids->jtag_id = val & JTAG_ID_AUTH_BMSK;

	res = read_sense_reg(SERIAL_NUM_SENSE_ADDR, &ids->serial_num);
	if (res)
		return res;

	return TEE_SUCCESS;
}

#define SEGMENT_HASH_ROOT_CERT_SEL_MAX	3U

TEE_Result qcom_secboot_get_segment_hash_size(uint32_t root_cert_sel,
					      uint32_t *hash_size)
{
	if (!hash_size)
		return TEE_ERROR_BAD_PARAMETERS;

	if (root_cert_sel > SEGMENT_HASH_ROOT_CERT_SEL_MAX)
		return TEE_ERROR_BAD_PARAMETERS;

#if SEGMENT_HASH_SELECT_SUPPORTED
	{
		TEE_Result res = TEE_ERROR_GENERIC;
		uint32_t val = 0;

		res = read_sense_reg(OEM_CONFIG2_ADDR, &val);
		if (res)
			return res;

		if (val & BIT32(SEGMENT_HASH_FUNCTION_SELECT0_SHFT +
				root_cert_sel))
			*hash_size = TEE_SHA256_HASH_SIZE;
		else
			*hash_size = TEE_SHA384_HASH_SIZE;
	}
#else
	*hash_size = TEE_SHA384_HASH_SIZE;
#endif

	return TEE_SUCCESS;
}

/*
 * Read the EKU_ENFORCEMENT_EN bit of OEM_CONFIG2: when blown, the leaf
 * certificate's Extended Key Usage extension must carry the
 * "Downloadable Code Signing" OID. Same register/bit on lemans and kodiak.
 */
TEE_Result qcom_secboot_get_eku_enforcement_en(bool *enabled)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t val = 0;

	if (!enabled)
		return TEE_ERROR_BAD_PARAMETERS;

	res = read_sense_reg(OEM_CONFIG2_ADDR, &val);
	if (res)
		return res;

	*enabled = val & BIT32(EKU_ENFORCEMENT_EN_SHFT);

	return TEE_SUCCESS;
}

/*
 * Root selection only applies when the root-of-trust anchor is fuse-resident
 * (PK_HASH_IN_FUSE) rather than a ROM constant. OEM_CONFIG0 ROOT_CERT_TOTAL_NUM
 * encodes (root count - 1); a single provisioned root disables selection.
 */
#define SECBOOT_MAX_NUM_ROOT_CERTS	4U

TEE_Result qcom_secboot_get_mrc_info(bool *root_sel_enabled,
				     uint32_t *num_roots,
				     uint32_t *activation_list,
				     uint32_t *revocation_list)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t total = 0;
	uint32_t val = 0;

	if (!root_sel_enabled || !num_roots || !activation_list ||
	    !revocation_list)
		return TEE_ERROR_BAD_PARAMETERS;

	*root_sel_enabled = false;
	*num_roots = 1;
	*activation_list = 0;
	*revocation_list = 0;

	res = read_sense_reg(SECURE_BOOT_APPS_ADDR, &val);
	if (res)
		return res;
	if (!(val & SECURE_BOOT_PK_HASH_IN_FUSE_BMSK))
		return TEE_SUCCESS;

	res = read_sense_reg(OEM_CONFIG0_ADDR, &val);
	if (res)
		return res;

	total = ((val & ROOT_CERT_TOTAL_NUM_BMSK) >> ROOT_CERT_TOTAL_NUM_SHFT) +
		1;
	if (total > SECBOOT_MAX_NUM_ROOT_CERTS)
		return TEE_ERROR_BAD_STATE;
	if (total <= 1)
		return TEE_SUCCESS;

	res = read_sense_reg(MRC_ACTIVATION_LIST_ADDR, &val);
	if (res)
		return res;
	*activation_list = val & MRC_ROOT_CERT_LIST_BMSK;

	res = read_sense_reg(MRC_REVOCATION_LIST_ADDR, &val);
	if (res)
		return res;
	*revocation_list = val & MRC_ROOT_CERT_LIST_BMSK;

	*num_roots = total;
	*root_sel_enabled = true;

	return TEE_SUCCESS;
}

TEE_Result qcom_secboot_get_soc_hw_version(uint32_t *fam_dev)
{
	vaddr_t va = 0;

	if (!fam_dev)
		return TEE_ERROR_BAD_PARAMETERS;

	/* SOC_HW_VERSION is a TCSR register, not part of the fuse map. */
	va = (vaddr_t)phys_to_virt(TCSR_SOC_HW_VERSION_ADDR, MEM_AREA_IO_SEC,
				   sizeof(uint32_t));
	if (!va) {
		va = (vaddr_t)core_mmu_add_mapping(MEM_AREA_IO_SEC,
						   TCSR_SOC_HW_VERSION_ADDR,
						   sizeof(uint32_t));
		if (!va)
			return TEE_ERROR_GENERIC;
	}

	*fam_dev = (io_read32(va) & SOC_HW_VERSION_FAM_DEV_BMSK) >>
		   SOC_HW_VERSION_FAM_DEV_SHFT;

	return TEE_SUCCESS;
}

static TEE_Result qfprom_init(void)
{
	const struct qfprom_platform_config *config = NULL;
	struct qfprom_context *drv = qfprom_get_context();

	config = qfprom_get_platform_config();
	if (!config) {
		EMSG("Failed to get platform configuration");
		goto err_panic;
	}

	drv->config = config;

	drv->raw_base_va = (vaddr_t)phys_to_virt(SECURITY_CONTROL_BASE,
						 MEM_AREA_IO_SEC,
						 SECURITY_CONTROL_SIZE);
	if (!drv->raw_base_va) {
		EMSG("Failed to get VA for security control at PA 0x%lx",
		     (unsigned long)SECURITY_CONTROL_BASE);
		goto err_panic;
	}

	drv->mutex_reg_va = (vaddr_t)phys_to_virt(QFPROM_MUTEX_REG_ADDR,
						  MEM_AREA_IO_SEC,
						  sizeof(uint32_t));
	if (!drv->mutex_reg_va) {
		EMSG("Failed to get VA for mutex register at PA 0x%lx",
		     (unsigned long)QFPROM_MUTEX_REG_ADDR);
		goto err_panic;
	}

	drv->corr_base_va = QFPROM_RAW_TO_CORR(drv->raw_base_va);

	return TEE_SUCCESS;

err_panic:
	panic("QFPROM driver initialization failed");
}

early_init_late(qfprom_init);
