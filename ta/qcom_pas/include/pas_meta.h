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

/*
 * OEM/QTI metadata, signature and certificate-region access for the MBN hash
 * segment. Builds on struct pas_mbn / pas_mbn_parse() (pas_mbn_parser.h),
 * which locates these regions but does not itself interpret them.
 */

/*
 * pas_meta_peek_version() - read the MBN header version ahead of the parse
 * @meta_data:      INIT_IMAGE metadata blob (ELF preamble + hash segment)
 * @meta_data_size: size of @meta_data in bytes
 * @version: decoded MBN header version on success
 *
 * The segment hash table digest size depends on the MBN version (v5 is always
 * SHA-256 by format definition; v6 selects SHA-256/SHA-384 via a fuse). This
 * does a minimal header-only pass to read the version before choosing the
 * digest size. Returns TEE_ERROR_BAD_FORMAT on a malformed segment.
 */
TEE_Result pas_meta_peek_version(const uint8_t *meta_data,
				 size_t meta_data_size, uint32_t *version);

/*
 * pas_meta_peek_root_cert_sel() - read root_cert_sel ahead of the full parse
 * @meta_data:		 INIT_IMAGE metadata blob (ELF preamble + hash segment)
 * @meta_data_size:	 size of @meta_data in bytes
 * @root_cert_sel: decoded metadata word 28 on success
 *
 * The digest size the segment hash table uses (SHA-256 vs SHA-384) is chosen
 * per-image via the OEM metadata's root_cert_sel field, but that field lives
 * inside the same metadata pas_mbn_parse() needs @hash_size to locate.
 * This does a minimal header-only pass to read root_cert_sel before the real
 * parse.
 *
 * Returns TEE_ERROR_NO_DATA when the segment carries no OEM metadata (v5, or
 * an image signed without it) - the caller should then use the default
 * root_cert_sel of 0. Returns TEE_ERROR_BAD_FORMAT on a malformed segment.
 */
TEE_Result pas_meta_peek_root_cert_sel(const uint8_t *meta_data,
				       size_t meta_data_size,
				       uint32_t *root_cert_sel);

/*
 * pas_meta_verify_preamble() - authenticate hash-table entry 0
 * @meta_data:		 INIT_IMAGE metadata blob (ELF preamble + hash segment)
 * @meta_data_size:	 size of @meta_data in bytes
 * @hash_table:	 hash table from a successful pas_mbn_parse()
 * @hash_size:	 digest size in bytes (32 or 48); selects the hash algorithm
 *
 * Entry 0 of the hash table digests the ELF header plus program-header
 * table (the metadata blob's preamble) rather than a loaded segment.
 * Authenticate it at INIT_IMAGE time, before any segment is loaded, rather
 * than waiting for the per-segment check at AUTH_AND_RESET.
 *
 * Returns TEE_ERROR_NOT_SUPPORTED for an unrecognized @hash_size,
 * TEE_ERROR_SECURITY on a digest mismatch, else TEE_SUCCESS.
 */
TEE_Result pas_meta_verify_preamble(const uint8_t *meta_data,
				    size_t meta_data_size,
				    const uint8_t *hash_table,
				    uint32_t hash_size);

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
 * Decodes the Qualcomm OEM metadata structure embedded in the MBN hash
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
 * debug re-enable). Valid values are 0-2; 3 is reserved and rejected. Value 2
 * is the SN-gated enable for all three fields: it requires the device serial
 * to match the metadata allow-list. (Root-revoke/activate and UIE key-switch
 * value 1 is a plain enable with no serial requirement; the debug option has
 * no such plain-enable value - 0 is NOP, 1 is DISABLE, 2 is the only ENABLE.)
 */
#define PAS_META_FLAG_ROOT_REVOKE_ACTIVATE_SHIFT	4
#define PAS_META_FLAG_UIE_KEY_SWITCH_SHIFT		6
#define PAS_META_FLAG_DEBUG_SHIFT			8
#define PAS_META_OPTION_MASK				3U
#define PAS_META_OPTION_MAX				2U
#define PAS_META_OPTION_ENABLE_SN			2U

/*
 * pas_meta_get() - decode the OEM metadata fields from a hash segment
 * @hs:   parsed hash segment
 * @meta: decoded metadata on success
 *
 * Returns TEE_ERROR_NO_DATA when the segment carries no OEM metadata (v5 or an
 * image signed without it), TEE_ERROR_BAD_FORMAT on a short block, else
 * TEE_SUCCESS.
 */
TEE_Result pas_meta_get(const struct pas_mbn *hs, struct pas_meta *meta);

/*
 * pas_meta_signed_copy() - build the OEM-signed-region bytes
 * @hs:     parsed hash segment
 * @out:    receives a newly allocated copy of the signed region
 * @out_len: receives the length of @out
 *
 * The OEM signature is computed over the signed region with the QTI header
 * size fields and QTI metadata block zeroed. The caller must TEE_Free(*out).
 */
TEE_Result pas_meta_signed_copy(const struct pas_mbn *hs,
				uint8_t **out, size_t *out_len);

#endif /* __PAS_META_H */
