/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_HASHSEG_H
#define __PAS_HASHSEG_H

#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>

/*
 * Qualcomm MBN hash-segment parser.
 *
 * The INIT_IMAGE metadata blob produced by qcom_mdt_read_metadata() is:
 *   [ phdrs[0].p_filesz bytes ]  ELF header + program-header table
 *   [ hash-segment bytes      ]  verbatim content of the MBN hash-segment phdr
 *
 * The MBN hash segment (at phdrs[0].p_filesz) holds signing material:
 *
 *   v5 (40-byte header):
 *     [header][hash table][qti sig][qti certs][oem sig][oem certs]
 *   v6 (48-byte header):
 *     [header][qti meta][oem meta][hash table]
 *     [qti sig][qti certs][oem sig][oem certs]
 *
 * Hash table: one digest per ELF program header; entry 0 = digest of the ELF
 * header plus program-header table, entry i = digest of the segment at phdr i.
 *
 * Signed region: header followed by [qti meta || oem meta || hash table]
 * (v5: header || hash table). Certificate chains: concatenated DER, leaf
 * first, self-signed root last.
 */

#define PAS_MBN_VERSION_5	5
#define PAS_MBN_VERSION_6	6

/*
 * struct pas_hashseg - parsed view of an MBN hash segment
 *
 * All pointers reference the caller-owned metadata buffer.
 *
 * @version:		MBN header version (PAS_MBN_VERSION_5 / _6)
 * @hash_table:		per-program-header digest table
 * @hash_table_size:	size of the hash table in bytes
 * @num_entries:	number of digests in the table
 * @hash_size:		digest size in bytes (32 = SHA-256, 48 = SHA-384)
 * @signed_region:	first byte covered by the signature
 * @signed_region_size:	number of bytes covered by the signature
 * @oem_meta:		OEM metadata block, NULL if absent (v6 only)
 * @oem_meta_size:	OEM metadata size in bytes
 * @oem_sig:		OEM signature (NULL if absent)
 * @oem_sig_size:	OEM signature size
 * @oem_certs:		OEM certificate chain, DER, leaf first (NULL if absent)
 * @oem_certs_size:	OEM certificate chain size
 * @qti_meta:		QTI metadata block, NULL if absent (v6 only)
 * @qti_meta_size:	QTI metadata size in bytes
 * @qti_sig:		QTI signature (NULL if not double-signed)
 * @qti_sig_size:	QTI signature size
 * @qti_certs:		QTI certificate chain (NULL if not double-signed)
 * @qti_certs_size:	QTI certificate chain size
 */
struct pas_hashseg {
	uint32_t version;

	const uint8_t *hash_table;
	size_t hash_table_size;
	uint32_t num_entries;
	uint32_t hash_size;

	const uint8_t *signed_region;
	size_t signed_region_size;

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
};

/*
 * pas_hashseg_parse() - parse the MBN hash segment inside an INIT_IMAGE blob
 * @md:		INIT_IMAGE metadata blob (ELF preamble + hash segment)
 * @md_size:	size of @md in bytes
 * @hash_size:	expected digest size (32 or 48); used to derive entry count
 * @out:	parsed result on success
 *
 * Return TEE_SUCCESS, TEE_ERROR_BAD_FORMAT, or TEE_ERROR_BAD_PARAMETERS.
 */
TEE_Result pas_hashseg_parse(const uint8_t *md, size_t md_size,
			     uint32_t hash_size, struct pas_hashseg *out);

/*
 * pas_hashseg_peek_version() - read the MBN header version ahead of the parse
 * @md:      INIT_IMAGE metadata blob (ELF preamble + hash segment)
 * @md_size: size of @md in bytes
 * @version: decoded MBN header version on success
 *
 * The segment hash table digest size depends on the MBN version (v5 is always
 * SHA-256 by format definition; v6 selects SHA-256/SHA-384 via a fuse). This
 * does a minimal header-only pass to read the version before choosing the
 * digest size. Returns TEE_ERROR_BAD_FORMAT on a malformed segment.
 */
TEE_Result pas_hashseg_peek_version(const uint8_t *md, size_t md_size,
				    uint32_t *version);

/*
 * pas_hashseg_peek_root_cert_sel() - read root_cert_sel ahead of the full parse
 * @md:		 INIT_IMAGE metadata blob (ELF preamble + hash segment)
 * @md_size:	 size of @md in bytes
 * @root_cert_sel: decoded metadata word 28 on success
 *
 * The digest size the segment hash table uses (SHA-256 vs SHA-384) is chosen
 * per-image via the OEM metadata's root_cert_sel field, but that field lives
 * inside the same metadata pas_hashseg_parse() needs @hash_size to locate.
 * This does a minimal header-only pass to read root_cert_sel before the real
 * parse.
 *
 * Returns TEE_ERROR_NO_DATA when the segment carries no OEM metadata (v5, or
 * an image signed without it) - the caller should then use the default
 * root_cert_sel of 0. Returns TEE_ERROR_BAD_FORMAT on a malformed segment.
 */
TEE_Result pas_hashseg_peek_root_cert_sel(const uint8_t *md, size_t md_size,
					  uint32_t *root_cert_sel);

/*
 * struct pas_meta - decoded MBN v6 OEM metadata
 * @major:		metadata major version
 * @minor:		metadata minor version
 * @sw_id:		image software type
 * @hw_id:		JTAG/HW id bound value (when IN_USE_JTAG_ID set)
 * @oem_id:		OEM id (bound unless OEM_ID_INDEPENDENT set)
 * @model_id:		model/product id (bound unless MODEL_ID_INDEPENDENT set)
 * @secondary_sw_id:	secondary software id
 * @flags:		binding flags (see PAS_META_FLAG_*)
 * @soc_vers:		accepted SoC family|device versions (when SOC_HW bound)
 * @serial_num:		accepted device serials (when serial binding set)
 * @root_cert_sel:	index of the root certificate this image chains to
 * @anti_rollback:	minimum image version permitted (rollback floor)
 *
 * Mirrors the Qualcomm OEM metadata structure embedded in the MBN hash
 * segment. The fields PAS binds are decoded here; several are gated on
 * @flags bits.
 */
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

/*
 * Metadata @flags bit positions. Fields whose "independent" bit is set are
 * not bound; SoC/JTAG/serial are bound only when their "in use" bit is set.
 */
#define PAS_META_FLAG_IN_USE_SOC_HW_VERSION	1
#define PAS_META_FLAG_USE_SERIAL_NUMBER		2
#define PAS_META_FLAG_OEM_ID_INDEPENDENT	3
#define PAS_META_FLAG_IN_USE_JTAG_ID		10
#define PAS_META_FLAG_MODEL_ID_INDEPENDENT	11

/*
 * 2-bit option fields within @flags (root-revoke/activate, UIE key switch,
 * debug re-enable). Valid values are 0-2; 3 is reserved and rejected.
 */
#define PAS_META_FLAG_ROOT_REVOKE_ACTIVATE_SHIFT	4
#define PAS_META_FLAG_UIE_KEY_SWITCH_SHIFT		6
#define PAS_META_FLAG_DEBUG_SHIFT			8
#define PAS_META_OPTION_MASK				3U
#define PAS_META_OPTION_MAX				2U

/*
 * pas_hashseg_get_meta() - decode the OEM metadata fields from a hash segment
 * @hs:   parsed hash segment
 * @meta: decoded metadata on success
 *
 * Returns TEE_ERROR_NO_DATA when the segment carries no OEM metadata (v5 or an
 * image signed without it), TEE_ERROR_BAD_FORMAT on a short block, else
 * TEE_SUCCESS.
 */
TEE_Result pas_hashseg_get_meta(const struct pas_hashseg *hs,
				struct pas_meta *meta);

/* Which signer's signature is being verified over the signed region. */
enum pas_signer {
	PAS_SIGNER_OEM,
	PAS_SIGNER_QTI,
};

/*
 * pas_hashseg_signed_copy() - build the signed-region bytes for one signer
 * @hs:     parsed hash segment
 * @signer: which signature will be verified over the returned buffer
 * @out:    receives a newly allocated copy of the signed region
 * @out_len: receives the length of @out
 *
 * For double-signed images each signature is computed over the signed region
 * with the OTHER signer's header size fields and metadata block zeroed. The
 * caller must TEE_Free(*out).
 */
TEE_Result pas_hashseg_signed_copy(const struct pas_hashseg *hs,
				   enum pas_signer signer,
				   uint8_t **out, size_t *out_len);

#endif /* __PAS_HASHSEG_H */
