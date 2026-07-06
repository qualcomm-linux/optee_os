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
 * The MBN hash segment (at phdrs[0].p_filesz) holds the per-segment digest
 * table:
 *
 *   v5 (40-byte header):
 *     [header][hash table][...]
 *   v6 (48-byte header):
 *     [header][qti meta][oem meta][hash table][...]
 *
 * Hash table: one digest per ELF program header; entry 0 = digest of the ELF
 * header plus program-header table, entry i = digest of the segment at phdr i.
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
 */
struct pas_hashseg {
	uint32_t version;

	const uint8_t *hash_table;
	size_t hash_table_size;
	uint32_t num_entries;
	uint32_t hash_size;
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

#endif /* __PAS_HASHSEG_H */
