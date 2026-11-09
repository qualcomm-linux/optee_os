/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qcom/platforminfo/platforminfo.h>
#include <drivers/qcom/platforminfo/platforminfodefs.h>
#include <drivers/qcom/smem/smem.h>
#include <initcall.h>
#include <inttypes.h>
#include <trace.h>

#include "platforminfo_internal.h"

static struct platforminfo_ctxt platforminfo_ctxt;

/*
 * Return true if an array containing num elements of elem_size fits
 * completely inside the SMEM item beginning at offset.
 *
 * Division is used instead of num * elem_size to prevent integer overflow.
 */
static bool smem_region_fits(size_t size, uint32_t offset, uint32_t num,
			     size_t elem_size)
{
	if (!num)
		return true;

	if (!offset || offset > size || !elem_size)
		return false;

	return num <= (size - offset) / elem_size;
}

enum platforminfo_type platforminfo_get_platform(void)
{
	if (!platforminfo_ctxt.initialized)
		return PLATFORMINFO_TYPE_UNKNOWN;

	return platforminfo_ctxt.platform;
}

TEE_Result
platforminfo_get_platform_info(struct platforminfo_info *info)
{
	if (!info)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!platforminfo_ctxt.initialized)
		return TEE_ERROR_BAD_STATE;

	info->platform = platforminfo_ctxt.platform;
	info->version = platforminfo_ctxt.version;
	info->subtype = platforminfo_ctxt.subtype;
	info->fusion = platforminfo_ctxt.fusion;
	info->oem_variant_id = platforminfo_ctxt.oem_variant_id;

	return TEE_SUCCESS;
}

bool platforminfo_is_fusion(void)
{
	if (!platforminfo_ctxt.initialized)
		return false;

	return platforminfo_ctxt.fusion;
}


uint32_t platforminfo_get_oem_variant(void)
{
	if (!platforminfo_ctxt.initialized)
		return PLATFORMINFO_OEM_VARIANT_UNKNOWN;

	return platforminfo_ctxt.oem_variant_id;
}

TEE_Result qti_platforminfo_init(void)
{
	const struct platforminfo_kvp *smem_kvps = NULL;
	const struct platforminfo_smem *smem = NULL;
	void *smem_ptr = NULL;
	size_t size = 0U;
	uint32_t fmt = 0U;
	uint32_t platform = 0U;
	uint32_t num_kvps = 0U;
	uint32_t kvps_offset = 0U;
	uint32_t i = 0U;
	TEE_Result res = TEE_SUCCESS;

	if (platforminfo_ctxt.initialized)
		return TEE_ERROR_BAD_STATE;

	res = qti_smem_init();
	if (res != TEE_SUCCESS && res != TEE_ERROR_BAD_STATE)
		return res;

	/*
	 * PlatformInfo and ChipInfo are stored in the socinfo SMEM item
	 * populated by boot firmware.
	 */
	res = qti_smem_lookup(QTI_SMEM_HOST_COMMON,
			      SMEM_HW_SW_BUILD_ID,
			      QTI_SMEM_FLAG_NONE,
			      &smem_ptr, &size);
	if (res != TEE_SUCCESS)
		return res;

	if (!smem_ptr || size < sizeof(uint32_t))
		return TEE_ERROR_BAD_FORMAT;

	smem = smem_ptr;
	fmt = smem->format;

	/*
	 * BL2 only populates fields up to the format version it reports, so
	 * gate every field read on fmt. chip_id/version exist from v1;
	 * chip_family from v12.
	 */
	fmt = smem->format;
	if (fmt < PLATFORMINFO_FORMAT_V1)
		return TEE_ERROR_BAD_FORMAT;

	/*
	 * The format version says which fields the layout defines, not that
	 * the item is large enough to hold them; validate size before
	 * dereferencing.
	 */
	if (size < PLATFORMINFO_SMEM_SIZE_V1)
		return TEE_ERROR_BAD_FORMAT;
	

	/*
	 * Cache basic PlatformInfo information.
	 */
	platform = smem->platform_type;

	if (platform >= PLATFORMINFO_NUM_TYPES)
		platform = PLATFORMINFO_TYPE_UNKNOWN;

	platforminfo_ctxt.platform =
		(enum platforminfo_type)platform;
	platforminfo_ctxt.version = smem->platform_version;
	platforminfo_ctxt.subtype = smem->platform_subtype;
	platforminfo_ctxt.fusion = smem->fusion;
	
	
	/*
	 * OEM variant ID was introduced in SMEM format V17.
	 * Check both the format and SMEM size before accessing it.
	 */
	if (fmt >= PLATFORMINFO_FORMAT_V17) {
		if (size < PLATFORMINFO_SMEM_SIZE_V17)
			return TEE_ERROR_BAD_FORMAT;

		platforminfo_ctxt.oem_variant_id = smem->oem_variant_id;
	}

	platforminfo_ctxt.initialized = true;

	return TEE_SUCCESS;
}

early_init(qti_platforminfo_init);