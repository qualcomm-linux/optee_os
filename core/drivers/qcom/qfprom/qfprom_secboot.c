// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * Secure-boot fuse accessors: OEM root-of-trust anchor and enable state,
 * device identity, EKU/image-encryption enforcement and SoC HW version.
 * Built only under CFG_QCOM_FUSE_PTA; other qfprom consumers live in
 * qfprom_core.c.
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

/*
 * SOC_HW_VERSION is a TCSR register, so its mapping is registered here
 * rather than in qfprom_core.c: only needed when these accessors are built.
 */
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
 * address, bypassing the region/permission table. Used for fuses that have no
 * corrected-space region entry (e.g. the PK_HASH words); each caller documents
 * why direct access is required.
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

	if (len != QFPROM_ROOT_OF_TRUST_BYTE_SIZE)
		return TEE_ERROR_BAD_PARAMETERS;

	/*
	 * qfprom_read_row() requires 8-byte-aligned addresses; use
	 * read_corr_word() instead, which accepts 4-byte alignment.
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
 * Read the OEM_CONFIG2 EKU_ENFORCEMENT_EN bit: when blown, the leaf
 * certificate's Extended Key Usage extension must carry the
 * "Downloadable Code Signing" OID.
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
 * Read the IMAGE_ENCRYPTION_ENABLE bit of OEM_CONFIG0: when blown, OEM image
 * encryption (UIE) is provisioned on the device.
 */
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

TEE_Result qcom_secboot_get_soc_hw_version(uint32_t *fam_dev)
{
	vaddr_t va = 0;

	if (!fam_dev)
		return TEE_ERROR_BAD_PARAMETERS;

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
