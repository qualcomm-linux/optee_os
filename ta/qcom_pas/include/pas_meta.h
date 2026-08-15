/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_META_H
#define __PAS_META_H

#include <pas_mbn_parser.h>
#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>

TEE_Result pas_meta_get_version(const uint8_t *meta_data,
				size_t meta_data_size, uint32_t *version);

TEE_Result pas_meta_segment_hash_len(const uint8_t *meta_data,
				     size_t meta_data_size,
				     uint32_t *hash_len);

TEE_Result pas_meta_get_root_cert_sel(const uint8_t *meta_data,
				      size_t meta_data_size,
				      uint32_t *root_cert_sel);

TEE_Result pas_meta_verify_preamble(const uint8_t *meta_data,
				    size_t meta_data_size,
				    const uint8_t *hash_table,
				    uint32_t hash_len);

struct pas_meta {
	uint32_t major;
	uint32_t minor;
	uint32_t sw_id;
	uint32_t hw_id;
	uint32_t oem_id;
	uint32_t model_id;
	uint32_t secondary_sw_id;
	uint32_t flags;
	uint32_t soc_vers[12];
	uint32_t serial_num[8];
	uint32_t root_cert_sel;
	uint32_t anti_rollback;
};

#define PAS_META_FLAG_IN_USE_SOC_HW_VERSION	1
#define PAS_META_FLAG_USE_SERIAL_NUMBER		2
#define PAS_META_FLAG_OEM_ID_INDEPENDENT	3
#define PAS_META_FLAG_IN_USE_JTAG_ID		10
#define PAS_META_FLAG_MODEL_ID_INDEPENDENT	11

#define PAS_META_FLAG_ROOT_REVOKE_ACTIVATE_SHIFT	4
#define PAS_META_FLAG_UIE_KEY_SWITCH_SHIFT		6
#define PAS_META_FLAG_DEBUG_SHIFT			8
#define PAS_META_OPTION_MASK				3U
#define PAS_META_OPTION_MAX				2U
#define PAS_META_OPTION_ENABLE_SN			2U

TEE_Result pas_meta_get(const struct pas_mbn *hs, struct pas_meta *meta);

TEE_Result pas_meta_signed_copy(const struct pas_mbn *hs,
				uint8_t **out, size_t *out_len);

#endif /* __PAS_META_H */
