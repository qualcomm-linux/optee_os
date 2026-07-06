// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk_qcom.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_pas.h>
#include <string.h>
#include <trace.h>
#include <utee_defines.h>
#include <util.h>

#ifdef CFG_QCOM_PAS_HASH_VERIFY
#include "pas_auth_core.h"
#endif
#include "pas_subsys.h"

static struct qcom_pas_subsys *pas_lookup(uint32_t pas_id)
{
	struct qcom_pas_subsys *subsys = NULL;
	size_t count = 0;

	subsys = qcom_pas_platform_subsys(&count);
	for (size_t i = 0; i < count; i++) {
		if (subsys[i].data.pas_id == pas_id)
			return &subsys[i];
	}

	return NULL;
}

TEE_Result pas_platform_is_supported(uint32_t pas_id)
{
	if (!pas_lookup(pas_id))
		return TEE_ERROR_NOT_SUPPORTED;

	return TEE_SUCCESS;
}

TEE_Result pas_platform_capabilities(uint32_t pas_id __unused)
{
	return TEE_SUCCESS;
}

TEE_Result pas_platform_init_image(uint32_t pas_id)
{
	if (!pas_lookup(pas_id))
		return TEE_ERROR_NOT_SUPPORTED;

	return TEE_SUCCESS;
}

TEE_Result pas_platform_mem_setup(uint32_t pas_id, uint32_t fw_size,
				  uint32_t fw_base_low, uint32_t fw_base_high)
{
	struct qcom_pas_subsys *subsys = pas_lookup(pas_id);
	struct qcom_pas_data *data = NULL;

	if (!subsys)
		return TEE_ERROR_NOT_SUPPORTED;

	data = &subsys->data;
	data->fw_size = fw_size;
	data->fw_base = fw_base_low;
	data->fw_base |= SHIFT_U64(fw_base_high, 32);

	/* Map the controller */
	if (!data->base.va) {
		data->base.va = (vaddr_t)core_mmu_add_mapping(MEM_AREA_IO_NSEC,
							      data->base.pa,
							      data->size);
		if (!data->base.va)
			return TEE_ERROR_GENERIC;
	}

	return TEE_SUCCESS;
}

#ifdef CFG_QCOM_PAS_HASH_VERIFY
TEE_Result pas_platform_verify_image(uint32_t pas_id, uint32_t fw_size,
				     paddr_t fw_base, const uint8_t *metadata,
				     size_t metadata_size,
				     const uint8_t *hash_table,
				     size_t table_len, uint32_t hash_size)
{
	struct qcom_pas_subsys *subsys = pas_lookup(pas_id);
	struct pas_auth_core_ctx ctx = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	struct qcom_pas_data *data = NULL;
	void *fw_va = NULL;

	if (!subsys)
		return TEE_ERROR_NOT_SUPPORTED;

	if (!metadata || !metadata_size || !hash_table ||
	    !table_len || !fw_size)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!hash_size || table_len % hash_size)
		return TEE_ERROR_BAD_PARAMETERS;

	data = &subsys->data;
	DMSG("PAS verify: pas_id=%"PRIu32" arg=%#"PRIxPA"/%#"PRIx32
	     " setup=%#"PRIxPA"/%#zx",
	     pas_id, fw_base, fw_size, data->fw_base, data->fw_size);

	/*
	 * Prefer the base/size from MEM_SETUP: AUTH_AND_RESET passes the DTS
	 * carveout size which may be larger than the image span used during
	 * MEM_SETUP; use the image span for segment offset bounds checking.
	 */
	if (data->fw_base && data->fw_size) {
		if (fw_base != data->fw_base) {
			EMSG("PAS auth: base %#"PRIxPA" != MEM_SETUP %#"PRIxPA,
			     fw_base, data->fw_base);
			return TEE_ERROR_SECURITY;
		}
		fw_size = data->fw_size; /* use image span for verification */
	} else if (!fw_base || !fw_size) {
		EMSG("PAS auth: no fw_base/size, no MEM_SETUP pas_id=%"PRIu32,
		     pas_id);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	/* Map the non-secure firmware carveout to recompute its digests */
	fw_va = core_mmu_add_mapping(MEM_AREA_RAM_NSEC, fw_base, fw_size);
	if (!fw_va) {
		EMSG("PAS auth: can't map carveout %#"PRIxPA"/%#"PRIx32,
		     fw_base, fw_size);
		return TEE_ERROR_GENERIC;
	}

	switch (hash_size) {
	case TEE_SHA256_HASH_SIZE:
		ctx.hash_algo = TEE_ALG_SHA256;
		break;
	case TEE_SHA384_HASH_SIZE:
		ctx.hash_algo = TEE_ALG_SHA384;
		break;
	default:
		res = TEE_ERROR_NOT_SUPPORTED;
		goto out;
	}

	ctx.hash_size = hash_size;
	ctx.hash_table = hash_table;
	ctx.num_entries = table_len / hash_size;
	ctx.metadata = metadata;
	ctx.metadata_size = metadata_size;
	ctx.fw = fw_va;
	ctx.fw_size = fw_size;
	ctx.fw_phys = fw_base;

	res = pas_auth_core_verify_segments(&ctx);
out:
	if (core_mmu_remove_mapping(MEM_AREA_RAM_NSEC, fw_va, fw_size))
		EMSG("PAS auth: failed to unmap carveout");

	return res;
}
#endif /* CFG_QCOM_PAS_HASH_VERIFY */

TEE_Result pas_platform_get_resource_table(uint32_t pas_id,
					   struct resource_table *rt,
					   size_t *size)
{
	struct qcom_pas_subsys *subsys = pas_lookup(pas_id);

	if (!subsys || !subsys->ops->get_resource_table)
		return TEE_ERROR_NOT_SUPPORTED;

	return subsys->ops->get_resource_table(rt, size);
}

TEE_Result pas_platform_set_remote_state(uint32_t pas_id, uint32_t state)
{
	struct qcom_pas_subsys *subsys = pas_lookup(pas_id);

	if (!subsys || !subsys->ops->fw_set_state)
		return TEE_ERROR_NOT_IMPLEMENTED;

	return subsys->ops->fw_set_state(&subsys->data, state);
}

TEE_Result pas_platform_auth_and_reset(uint32_t pas_id)
{
	struct qcom_pas_subsys *subsys = pas_lookup(pas_id);
	struct qcom_pas_data *data = NULL;
	TEE_Result res = TEE_ERROR_GENERIC;

	if (!subsys)
		return TEE_ERROR_NOT_SUPPORTED;

	data = &subsys->data;
	if (!data->fw_base)
		return TEE_ERROR_NO_DATA;

	switch (subsys->reset_seq) {
	case QCOM_PAS_RESET_CLK_FULL:
		res = qcom_clock_pas_reset(data->clk_group);
		if (res != TEE_SUCCESS)
			return res;

		res = qcom_clock_enable(data->clk_group);
		if (res != TEE_SUCCESS)
			return res;

		res = subsys->ops->fw_start(data);
		if (res != TEE_SUCCESS)
			return res;

		return qcom_clock_enable_pas_processor(data->clk_group);
	case QCOM_PAS_RESET_CLK_ENABLE:
		res = qcom_clock_enable(data->clk_group);
		if (res != TEE_SUCCESS) {
			EMSG("Failed to enable clocks: %d", res);
			return res;
		}

		return subsys->ops->fw_start(data);
	case QCOM_PAS_RESET_NONE:
		return subsys->ops->fw_start(data);
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}

TEE_Result pas_platform_shutdown(uint32_t pas_id)
{
	struct qcom_pas_subsys *subsys = pas_lookup(pas_id);

	if (!subsys || !subsys->ops->fw_shutdown)
		return TEE_ERROR_NOT_SUPPORTED;

	return subsys->ops->fw_shutdown(&subsys->data);
}
