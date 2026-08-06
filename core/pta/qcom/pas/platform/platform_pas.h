/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef PLATFORM_PAS_H
#define PLATFORM_PAS_H

#include <resource_table.h>
#include <tee_api_types.h>
#include <types_ext.h>

TEE_Result pas_platform_mem_setup(uint32_t pas_id, uint32_t fw_size,
				  uint32_t fw_base_low, uint32_t fw_base_high);
TEE_Result pas_platform_get_resource_table(uint32_t pas_id,
					   struct resource_table *rt,
					   size_t *size);
TEE_Result pas_platform_set_remote_state(uint32_t pas_id, uint32_t state);
TEE_Result pas_platform_auth_and_reset(uint32_t pas_id);
TEE_Result pas_platform_is_supported(uint32_t pas_id);
TEE_Result pas_platform_capabilities(uint32_t pas_id);
TEE_Result pas_platform_init_image(uint32_t pas_id);
TEE_Result pas_platform_shutdown(uint32_t pas_id);

/*
 * struct pas_fw_region - loaded firmware carveout
 * @base: physical base address
 * @size: size in bytes
 */
struct pas_fw_region {
	paddr_t base;
	uint32_t size;
};

/*
 * struct pas_metadata - ELF metadata blob (header + phdrs)
 * @data: used for ELF parsing and the ELF header hash (entry 0); NOT in
 *        the firmware carveout
 * @size: size of @data in bytes
 */
struct pas_metadata {
	const uint8_t *data;
	size_t size;
};

/*
 * struct pas_hash_table - per-segment digest table, one entry per program
 * header. Supplied by the caller and not authenticated here.
 * @table:      digest table
 * @len:        size of @table in bytes
 * @entry_size: digest size in bytes (32 for SHA-256, 48 for SHA-384)
 */
struct pas_hash_table {
	const uint8_t *table;
	size_t len;
	uint32_t entry_size;
};

/*
 * Verify the integrity of a loaded firmware image against @hash. Maps
 * @fw, recomputes each segment digest and compares it to the table, then
 * unmaps.
 *
 * TODO: authenticating the hash table itself (certificate chain and
 * signature) will be added incrementally.
 */
#ifdef CFG_QCOM_PAS_AUTH
TEE_Result pas_platform_verify_image(uint32_t pas_id,
				     const struct pas_fw_region *fw,
				     const struct pas_metadata *metadata,
				     const struct pas_hash_table *hash);
#else
static inline TEE_Result
pas_platform_verify_image(uint32_t pas_id __unused,
			  const struct pas_fw_region *fw __unused,
			  const struct pas_metadata *metadata __unused,
			  const struct pas_hash_table *hash __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}
#endif /* CFG_QCOM_PAS_AUTH */

#endif
