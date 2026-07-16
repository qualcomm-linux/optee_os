/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_SIG_AUTH_H
#define __PAS_SIG_AUTH_H

#include <pas_mbn_parser.h>
#include <qcom_pas_priv.h>
#include <tee_internal_api.h>

/*
 * Signature-authentication backend for the PAS TA: certificate chain,
 * signature, SW_ID/HW binding and anti-rollback. Runs this crypto work
 * BEFORE the REE loads any ELF segment into the carveout, between parsing
 * the hash table and trusting it.
 */

#ifdef CFG_QCOM_PAS_AUTH
/*
 * Determine the per-segment hash digest size for @slot's metadata. The OEM
 * metadata's root_cert_sel selects the fuse-configured algorithm on
 * platforms that implement the field; MBN v5 images (no OEM metadata) always
 * use SHA-256.
 */
TEE_Result pas_sig_auth_hash_size(const struct pas_md_slot *slot,
				  uint32_t *hash_size);

/*
 * Authenticate @hs (already parsed by pas_mbn_parse()): reject
 * UIE-encrypted images, verify the certificate chain, signature and
 * fuse-bound bindings, verify the hash-table preamble entry, and enforce
 * anti-rollback. @meta_data/@meta_data_size are the same INIT_IMAGE
 * metadata blob @hs was parsed from. @anchor is the OEM root-of-trust
 * digest the caller already read from the fuse PTA
 * (PTA_QCOM_FUSE_ROOT_OF_TRUST_SIZE bytes); callable only when the caller's
 * secure-boot gate is on, so every check here is unconditional.
 */
TEE_Result pas_sig_auth_authenticate(const struct pas_mbn *hs,
				     const uint8_t *meta_data,
				     size_t meta_data_size,
				     uint32_t pas_id, uint32_t hash_size,
				     const uint8_t *anchor);
#else
static inline TEE_Result
pas_sig_auth_hash_size(const struct pas_md_slot *slot __unused,
		       uint32_t *hash_size)
{
	*hash_size = TEE_SHA384_HASH_SIZE;
	return TEE_SUCCESS;
}

static inline TEE_Result
pas_sig_auth_authenticate(const struct pas_mbn *hs __unused,
			  const uint8_t *meta_data __unused,
			  size_t meta_data_size __unused,
			  uint32_t pas_id __unused,
			  uint32_t hash_size __unused,
			  const uint8_t *anchor __unused)
{
	return TEE_SUCCESS;
}
#endif /* CFG_QCOM_PAS_AUTH */

#endif /* __PAS_SIG_AUTH_H */
