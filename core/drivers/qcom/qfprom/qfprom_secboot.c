// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <inttypes.h>
#include <io.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <string.h>
#include <trace.h>
#include <utee_defines.h>
#include <util.h>

#include "qfprom_priv.h"
#include "qfprom_target.h"

register_phys_mem_pgdir(MEM_AREA_IO_SEC, TCSR_SOC_HW_VERSION_ADDR,
			CORE_MMU_PGDIR_SIZE);

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

	if (version <= cur)
		return TEE_SUCCESS;

	if (version > PIL_ARB_LSB_MAX_VERSION + PIL_ARB_MSB_MAX_VERSION) {
		EMSG("PAS ARB: version %#"PRIx32" exceeds fuse capacity",
		     version);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	/* qfprom_write_row() only blows 0->1 bits; OR in current contents. */
	lsb_n = MIN(version, (uint32_t)PIL_ARB_LSB_MAX_VERSION);
	if (version > PIL_ARB_LSB_MAX_VERSION)
		msb_n = version - PIL_ARB_LSB_MAX_VERSION;

	row[0] = cur_lsb | unary_mask(lsb_n);
	row[1] = cur_msb | unary_mask(msb_n);

	res = qfprom_hw_init();
	if (res)
		return res;

	res = qfprom_write_row(PIL_ARB_RAW_ADDR, row);
	qfprom_hw_deinit();
	if (res) {
		EMSG("PAS ARB: fuse write failed: %#"PRIx32, res);
		return res;
	}

	DMSG("PAS ARB: advanced device version %#"PRIx32" -> %#"PRIx32, cur,
	     version);

	return TEE_SUCCESS;
}

static TEE_Result read_sense_reg(paddr_t pa, uint32_t *out)
{
	struct qfprom_context *drv = qfprom_get_context();

	if (!drv->raw_base_va)
		return TEE_ERROR_BAD_STATE;

	*out = io_read32(drv->raw_base_va + (pa - SECURITY_CONTROL_BASE));

	return TEE_SUCCESS;
}

TEE_Result qcom_secboot_get_root_of_trust(uint8_t *hash, size_t len)
{
	size_t off = 0;

	if (!hash)
		return TEE_ERROR_BAD_PARAMETERS;

	if (len != QFPROM_ROOT_OF_TRUST_BYTE_SIZE)
		return TEE_ERROR_BAD_PARAMETERS;

	for (off = 0; off < len; off += sizeof(uint32_t)) {
		TEE_Result res = TEE_ERROR_GENERIC;
		uint32_t word = 0;

		res = read_sense_reg(PK_HASH0_SENSE_ADDR + off, &word);
		if (res)
			return res;

		memcpy(hash + off, &word, MIN(sizeof(word), len - off));
	}

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

TEE_Result qcom_secboot_get_segment_hash_len(uint32_t root_cert_sel,
					     uint32_t *hash_len)
{
	if (!hash_len)
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
			*hash_len = TEE_SHA256_HASH_SIZE;
		else
			*hash_len = TEE_SHA384_HASH_SIZE;
	}
#else
	*hash_len = TEE_SHA384_HASH_SIZE;
#endif

	return TEE_SUCCESS;
}

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

TEE_Result qcom_secboot_get_rsa_disable(bool *disabled)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t val = 0;

	if (!disabled)
		return TEE_ERROR_BAD_PARAMETERS;

	res = read_sense_reg(OEM_CONFIG2_ADDR, &val);
	if (res)
		return res;

	*disabled = val & BIT32(DISABLE_RSA_SHFT);

	return TEE_SUCCESS;
}

TEE_Result qcom_secboot_get_image_encryption_en(bool *enabled)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t val = 0;

	if (!enabled)
		return TEE_ERROR_BAD_PARAMETERS;

	res = read_sense_reg(OEM_CONFIG0_ADDR, &val);
	if (res)
		return res;

	*enabled = val & BIT32(IMAGE_ENCRYPTION_ENABLE_SHFT);

	return TEE_SUCCESS;
}

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

	va = (vaddr_t)phys_to_virt(TCSR_SOC_HW_VERSION_ADDR, MEM_AREA_IO_SEC,
				   sizeof(uint32_t));
	if (!va)
		return TEE_ERROR_GENERIC;

	*fam_dev = (io_read32(va) & SOC_HW_VERSION_FAM_DEV_BMSK) >>
		   SOC_HW_VERSION_FAM_DEV_SHFT;

	return TEE_SUCCESS;
}
