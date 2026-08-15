/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_SIG_AUTH_H
#define __PAS_SIG_AUTH_H

#include <pas_mbn_parser.h>
#include <qcom_pas_priv.h>
#include <tee_internal_api.h>

#ifdef CFG_QCOM_PAS_AUTH
TEE_Result pas_sig_auth_hash_len(const struct pas_md_slot *slot,
				 uint32_t *hash_len);

TEE_Result pas_sig_auth_authenticate(const struct pas_mbn *hs,
				     const uint8_t *meta_data,
				     size_t meta_data_size,
				     uint32_t pas_id, uint32_t hash_len,
				     const uint8_t *anchor);

TEE_Result pas_sig_auth_commit_rollback(const struct pas_mbn *hs);
#else
static inline TEE_Result
pas_sig_auth_hash_len(const struct pas_md_slot *slot __unused,
		      uint32_t *hash_len)
{
	*hash_len = TEE_SHA384_HASH_SIZE;
	return TEE_SUCCESS;
}

static inline TEE_Result
pas_sig_auth_authenticate(const struct pas_mbn *hs __unused,
			  const uint8_t *meta_data __unused,
			  size_t meta_data_size __unused,
			  uint32_t pas_id __unused,
			  uint32_t hash_len __unused,
			  const uint8_t *anchor __unused)
{
	return TEE_SUCCESS;
}

static inline TEE_Result
pas_sig_auth_commit_rollback(const struct pas_mbn *hs __unused)
{
	return TEE_SUCCESS;
}
#endif /* CFG_QCOM_PAS_AUTH */

#endif /* __PAS_SIG_AUTH_H */
