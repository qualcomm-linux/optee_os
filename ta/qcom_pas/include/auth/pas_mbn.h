/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_MBN_PARSER_H
#define __PAS_MBN_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>

/* MBN hash segment layout: v6 [header][qti meta][oem meta][hash table][sigs][certs]
 * v7 [header][common meta][qti meta][oem meta][hash table][sigs][certs] (40-byte header) */

#define PAS_MBN_VERSION_6	6
#define PAS_MBN_VERSION_7	7

/* struct pas_mbn - parsed MBN hash segment (all pointers caller-owned)
 * @version: MBN header version (6 or 7)
 * @hash_table: per-program-header digest table
 * @hash_table_size: size in bytes
 * @num_entries: number of digests
 * @hash_len: digest size (32=SHA-256, 48=SHA-384)
 * @signed_region: first byte covered by signature
 * @signed_region_size: bytes covered by signature
 * @common_meta: v7 common metadata block (NULL if absent)
 * @common_meta_size: common metadata size
 * @oem_meta: OEM metadata block (NULL if absent)
 * @oem_meta_size: OEM metadata size
 * @oem_sig: OEM signature (NULL if absent)
 * @oem_sig_size: OEM signature size
 * @oem_certs: OEM certificate chain, DER, leaf first (NULL if absent)
 * @oem_certs_size: OEM certificate chain size
 * @qti_meta: QTI metadata block (NULL if absent)
 * @qti_meta_size: QTI metadata size
 * @qti_sig: QTI signature (NULL if not double-signed)
 * @qti_sig_size: QTI signature size
 * @qti_certs: QTI certificate chain (NULL if not double-signed)
 * @qti_certs_size: QTI certificate chain size
 * @uie_encrypted: true if segment carries UIE encryption parameter block
 * Populated by pas_mbn_parse() for pas_meta.c under CFG_QCOM_PAS_AUTH */
struct pas_mbn {
	uint32_t version;

	const uint8_t *hash_table;
	size_t hash_table_size;
	uint32_t num_entries;
	uint32_t hash_len;

	const uint8_t *signed_region;
	size_t signed_region_size;

	const uint8_t *common_meta;
	size_t common_meta_size;

	const uint8_t *oem_meta;
	size_t oem_meta_size;
	const uint8_t *oem_sig;
	size_t oem_sig_size;
	const uint8_t *oem_certs;
	size_t oem_certs_size;

	const uint8_t *qti_meta;
	size_t qti_meta_size;
	const uint8_t *qti_sig;
	size_t qti_sig_size;
	const uint8_t *qti_certs;
	size_t qti_certs_size;

	bool uie_encrypted;
};

TEE_Result pas_mbn_parse(const uint8_t *md, size_t md_size,
			 uint32_t hash_len, struct pas_mbn *out);

#endif /* __PAS_MBN_PARSER_H */
