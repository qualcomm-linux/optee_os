/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_META_H
#define __PAS_META_H

#include <pas_mbn_parser.h>
#include <stdbool.h>
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
 * pas_meta_peek_hash_table_algo() - read the v7 segment-hash digest size
 * @md:		 INIT_IMAGE metadata blob (ELF preamble + hash segment)
 * @md_size:	 size of @md in bytes
 * @hash_size:	 decoded digest size (32 or 48) on success
 *
 * MBN v7 selects the segment hash-table's digest algorithm via the common
 * metadata's hash_table_algo field (SHA-256 or SHA-384; SHA-512 and every
 * zero-initialized-segment "_ZI" variant are rejected - see pas_meta.c).
 * Unlike v6, this is not fuse-selected: it is a signed metadata field, so no
 * fuse-PTA round trip is needed to pick the digest size for a v7 image.
 *
 * Returns TEE_ERROR_NOT_SUPPORTED for an unrecognized algorithm,
 * TEE_ERROR_BAD_FORMAT on a malformed segment.
 */
TEE_Result pas_meta_peek_hash_table_algo(const uint8_t *md, size_t md_size,
					 uint32_t *hash_size);

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
 * struct pas_meta - decoded OEM metadata (MBN v6, or v7's per-signing block)
 * @is_v7:		true if @flags uses the v7 2-bit "bound" encoding
 *			(PAS_META7_FLAG_*_SHIFT); false for v6's single-bit
 *			"independent" encoding (PAS_META_FLAG_*). Callers must
 *			branch on this before interpreting @flags - the two
 *			encodings use overlapping shift values with opposite
 *			polarity and are never compatible.
 * @major:		metadata major version (v6: 0 or 1; v7: 2 or 3)
 * @minor:		metadata minor version
 * @sw_id:		image software type (v7: from the common-metadata block)
 * @hw_id:		JTAG/HW id bound value (when the JTAG binding gate
 *			is set)
 * @oem_id:		OEM id (bound unless its independent/not-bound gate
 *			is set)
 * @model_id:		model/product id (bound unless its gate is set)
 * @secondary_sw_id:	secondary software id
 * @flags:		binding flags; see @is_v7
 * @soc_vers:		accepted SoC family|device versions (when SOC_HW bound)
 * @serial_num:		accepted device serials (when serial binding set);
 *			v6 entries are 32-bit on the wire, v7 entries are
 *			64-bit - widened to uint64_t here so one comparison
 *			against the (32-bit) fused serial works for both
 * @root_cert_sel:	index of the root certificate this image chains to
 * @anti_rollback:	minimum image version permitted (rollback floor)
 *
 * Mirrors the Qualcomm OEM metadata structure embedded in the MBN hash
 * segment. The fields PAS binds are decoded here; several are gated on
 * @flags bits.
 */
struct pas_meta {
	bool is_v7;
	uint32_t major;
	uint32_t minor;
	uint32_t sw_id;
	uint32_t hw_id;
	uint32_t oem_id;
	uint32_t model_id;
	uint32_t secondary_sw_id;
	uint32_t flags;
	uint32_t soc_vers[12];
	uint64_t serial_num[8];
	uint32_t root_cert_sel;
	uint32_t anti_rollback;
};

/*
 * Metadata @flags bit positions (MBN v6 only - single "independent" bits).
 * Fields whose "independent" bit is set are not bound; SoC/JTAG/serial are
 * bound only when their "in use" bit is set.
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
 * Metadata @flags bit-pair shifts (MBN v7 only - "bound" pairs, NOT
 * compatible with the v6 PAS_META_FLAG_* single-bit encoding above). Each
 * field occupies 2 bits: "10" (MSB=1, LSB=0) means the field is bound and
 * must be enforced, "01" (MSB=0, LSB=1) means it is not bound; "00"/"11" are
 * invalid encodings (see pas_meta_v7_flags_valid()).
 */
#define PAS_META7_FLAG_SOC_HW_VERSION_BOUND_SHIFT	0
#define PAS_META7_FLAG_JTAG_ID_BOUND_SHIFT		4
#define PAS_META7_FLAG_SERIAL_NUMBER_BOUND_SHIFT	6
#define PAS_META7_FLAG_OEM_ID_BOUND_SHIFT		8
#define PAS_META7_FLAG_OEM_PRODUCT_ID_BOUND_SHIFT	10
#define PAS_META7_FLAG_MAX_SHIFT			20

/*
 * pas_meta7_flag_bit() - read one bit of a v7 flags bit-pair
 * @flags: metadata flags word
 * @shift: bit-pair base shift (a PAS_META7_FLAG_*_SHIFT value)
 * @msb:   true to read the pair's high bit, false for the low bit
 */
static inline bool pas_meta7_flag_bit(uint32_t flags, uint32_t shift,
				      bool msb)
{
	return (flags >> (shift + (msb ? 1 : 0))) & 1;
}

/*
 * pas_meta7_flag_bound() - true if the v7 flags bit-pair at @shift is "10"
 * (bound). Does not validate the pair; call pas_meta7_flags_valid() first.
 */
static inline bool pas_meta7_flag_bound(uint32_t flags, uint32_t shift)
{
	return pas_meta7_flag_bit(flags, shift, true) &&
	       !pas_meta7_flag_bit(flags, shift, false);
}

/*
 * pas_meta7_flags_valid() - true if every 2-bit field in a v7 flags word is
 * "10" or "01" (never "00"/"11")
 */
bool pas_meta7_flags_valid(uint32_t flags);

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
 * pas_meta_oem_signed_copy() - build the OEM-signed-region bytes
 * @hs:     parsed hash segment
 * @out:    receives a newly allocated copy of the signed region
 * @out_len: receives the length of @out
 *
 * The OEM signature is computed over the signed region with the QTI header
 * size fields and QTI metadata block zeroed. The caller must TEE_Free(*out).
 */
TEE_Result pas_meta_oem_signed_copy(const struct pas_mbn *hs,
				    uint8_t **out, size_t *out_len);

#endif /* __PAS_META_H */
