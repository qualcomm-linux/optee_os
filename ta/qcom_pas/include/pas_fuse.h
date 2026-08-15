/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_FUSE_H
#define __PAS_FUSE_H

#include <stdbool.h>
#include <stdint.h>
#include <tee_api_types.h>

#ifdef CFG_QCOM_PAS_AUTH
TEE_Result pas_fuse_open(void);

void pas_fuse_close(void);
#else
static inline TEE_Result pas_fuse_open(void)
{
	return TEE_SUCCESS;
}

static inline void pas_fuse_close(void)
{
}
#endif /* CFG_QCOM_PAS_AUTH */

TEE_Result pas_fuse_get_secboot_and_root_anchor(uint8_t *anchor,
						bool *secboot_on);

struct pas_fuse_hw_binding_info {
	uint32_t oem_id;
	uint32_t model_id;
	uint32_t jtag_id;
	uint32_t serial_num;
	bool use_serial_num_override;
	uint32_t soc_fam_dev;
};

TEE_Result pas_fuse_get_hw_binding_info(bool need_soc_vers,
					struct pas_fuse_hw_binding_info *info);

TEE_Result pas_fuse_get_eku_enforcement_en(bool *eku_enforced);

TEE_Result pas_fuse_get_rsa_disable(bool *rsa_disabled);

bool pas_fuse_get_image_encryption_en(void);

TEE_Result pas_fuse_get_segment_hash_len(uint32_t root_cert_sel,
					 uint32_t *hash_len);

struct pas_fuse_mrc_info {
	uint32_t num_roots;
	uint32_t activation_list;
	uint32_t revocation_list;
};

TEE_Result pas_fuse_get_mrc_info(struct pas_fuse_mrc_info *info);

TEE_Result pas_fuse_get_pil_rollback_version(uint32_t *dev_ver);

TEE_Result pas_fuse_blow_pil_rollback_version(uint32_t version);

#endif /* __PAS_FUSE_H */
