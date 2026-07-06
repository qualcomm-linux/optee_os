// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <elf32.h>
#include <pas_hashseg.h>
#include <string.h>
#include <tee_internal_api.h>
#include <util.h>

/* MBN header field offsets (bytes from hash-segment start) */
#define MBN_OFF_VERSION		0x04
#define MBN_OFF_QC_SIG_SIZE	0x08
#define MBN_OFF_QC_CERT_SIZE	0x0c
#define MBN_OFF_CODE_SIZE	0x14
#define MBN_OFF_OEM_SIG_SIZE	0x1c
#define MBN_OFF_OEM_CERT_SIZE	0x24
#define MBN_OFF_QC_META_SIZE	0x28	/* v6 only */
#define MBN_OFF_OEM_META_SIZE	0x2c	/* v6 only */

#define MBN_HDR_SIZE_V5		0x28
#define MBN_HDR_SIZE_V6		0x30

static uint32_t read_u32(const uint8_t *p)
{
	uint32_t v = 0;

	memcpy(&v, p, sizeof(v));

	return v;
}

/*
 * Locate the MBN hash segment inside the INIT_IMAGE blob. The blob starts
 * with phdrs[0].p_filesz bytes of ELF header + program-header table; the
 * hash segment follows immediately.
 */
static TEE_Result find_hashseg(const uint8_t *md, size_t md_size,
			       const uint8_t **seg, size_t *seg_size)
{
	const Elf32_Ehdr *ehdr = (const void *)md;
	const Elf32_Phdr *phdrs = NULL;
	size_t phtab_end = 0;
	size_t preamble = 0;

	if (md_size < sizeof(*ehdr))
		return TEE_ERROR_BAD_FORMAT;

	if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
	    ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
	    ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
	    ehdr->e_ident[EI_MAG3] != ELFMAG3 ||
	    ehdr->e_ident[EI_CLASS] != ELFCLASS32)
		return TEE_ERROR_BAD_FORMAT;

	if (ehdr->e_phnum < 2 || !ehdr->e_phoff ||
	    ehdr->e_phentsize < sizeof(Elf32_Phdr))
		return TEE_ERROR_BAD_FORMAT;

	if (MUL_OVERFLOW(ehdr->e_phentsize, ehdr->e_phnum, &phtab_end) ||
	    ADD_OVERFLOW(phtab_end, ehdr->e_phoff, &phtab_end) ||
	    phtab_end > md_size)
		return TEE_ERROR_BAD_FORMAT;

	phdrs = (const void *)(md + ehdr->e_phoff);
	preamble = phdrs[0].p_filesz;

	if (preamble >= md_size)
		return TEE_ERROR_BAD_FORMAT;

	*seg = md + preamble;
	*seg_size = md_size - preamble;

	return TEE_SUCCESS;
}

/* Advance @*cursor past @len bytes of @seg, bounds-checked. */
static TEE_Result skip_region(size_t seg_size, size_t *cursor, size_t len)
{
	if (!len)
		return TEE_SUCCESS;

	if (len > seg_size || *cursor > seg_size - len)
		return TEE_ERROR_BAD_FORMAT;

	*cursor += len;

	return TEE_SUCCESS;
}

TEE_Result pas_hashseg_parse(const uint8_t *md, size_t md_size,
			     uint32_t hash_size, struct pas_hashseg *out)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t oem_meta_size = 0;
	const uint8_t *seg = NULL;
	uint32_t qc_meta_size = 0;
	uint32_t code_size = 0;
	uint32_t version = 0;
	size_t hdr_size = 0;
	size_t seg_size = 0;
	size_t cursor = 0;

	if (!md || !md_size || !out || !hash_size)
		return TEE_ERROR_BAD_PARAMETERS;

	memset(out, 0, sizeof(*out));

	res = find_hashseg(md, md_size, &seg, &seg_size);
	if (res)
		return res;

	if (seg_size < MBN_HDR_SIZE_V5)
		return TEE_ERROR_BAD_FORMAT;

	version = read_u32(seg + MBN_OFF_VERSION);
	switch (version) {
	case PAS_MBN_VERSION_5:
		hdr_size = MBN_HDR_SIZE_V5;
		break;
	case PAS_MBN_VERSION_6:
		hdr_size = MBN_HDR_SIZE_V6;
		break;
	default:
		EMSG("PAS auth: unsupported MBN version %"PRIu32, version);
		return TEE_ERROR_BAD_FORMAT;
	}

	if (seg_size < hdr_size)
		return TEE_ERROR_BAD_FORMAT;

	code_size = read_u32(seg + MBN_OFF_CODE_SIZE);
	if (version == PAS_MBN_VERSION_6) {
		qc_meta_size = read_u32(seg + MBN_OFF_QC_META_SIZE);
		oem_meta_size = read_u32(seg + MBN_OFF_OEM_META_SIZE);
	}

	if (!code_size || code_size % hash_size)
		return TEE_ERROR_BAD_FORMAT;

	/*
	 * Payload after the header:
	 *   [qc_meta][oem_meta][hash table][...]
	 * Skip the metadata blocks to locate the hash table.
	 */
	cursor = hdr_size;
	res = skip_region(seg_size, &cursor, qc_meta_size);
	if (res)
		return res;
	res = skip_region(seg_size, &cursor, oem_meta_size);
	if (res)
		return res;

	if (code_size > seg_size || cursor > seg_size - code_size)
		return TEE_ERROR_BAD_FORMAT;

	out->version = version;
	out->hash_table = seg + cursor;
	out->hash_table_size = code_size;
	out->hash_size = hash_size;
	out->num_entries = code_size / hash_size;

	DMSG("PAS auth: MBN v%"PRIu32", %"PRIu32" entries", version,
	     out->num_entries);

	return TEE_SUCCESS;
}
