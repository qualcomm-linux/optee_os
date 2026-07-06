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

/*
 * OEM metadata field word offsets within the metadata block
 * (little-endian 32-bit words).
 */
#define META_OFF_MAJOR		0
#define META_OFF_MINOR		1
#define META_OFF_SW_ID		2
#define META_OFF_HW_ID		3
#define META_OFF_OEM_ID		4
#define META_OFF_MODEL_ID	5
#define META_OFF_SECONDARY_SW_ID 6
#define META_OFF_FLAGS		7
#define META_OFF_SOC_VERS	8
#define META_NUM_SOC_VERS	12
#define META_OFF_SERIAL_NUM	(META_OFF_SOC_VERS + META_NUM_SOC_VERS)
#define META_NUM_SERIAL_NUM	8
#define META_OFF_ANTI_ROLLBACK	29
#define META_OFF_ROOT_CERT_SEL	28
#define META_MIN_WORDS		(META_OFF_ANTI_ROLLBACK + 1)

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

/* Slice @len bytes from @seg at @*cursor; advance cursor. Zero len → NULL. */
static TEE_Result take_region(const uint8_t *seg, size_t seg_size,
			      size_t *cursor, size_t len,
			      const uint8_t **ptr, size_t *ptr_len)
{
	if (!len) {
		*ptr = NULL;
		*ptr_len = 0;
		return TEE_SUCCESS;
	}

	if (len > seg_size || *cursor > seg_size - len)
		return TEE_ERROR_BAD_FORMAT;

	*ptr = seg + *cursor;
	*ptr_len = len;
	*cursor += len;

	return TEE_SUCCESS;
}

TEE_Result pas_hashseg_peek_version(const uint8_t *md, size_t md_size,
				    uint32_t *version)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	const uint8_t *seg = NULL;
	size_t seg_size = 0;

	if (!md || !md_size || !version)
		return TEE_ERROR_BAD_PARAMETERS;

	res = find_hashseg(md, md_size, &seg, &seg_size);
	if (res)
		return res;

	if (seg_size < MBN_HDR_SIZE_V5)
		return TEE_ERROR_BAD_FORMAT;

	*version = read_u32(seg + MBN_OFF_VERSION);

	return TEE_SUCCESS;
}

TEE_Result pas_hashseg_peek_root_cert_sel(const uint8_t *md, size_t md_size,
					  uint32_t *root_cert_sel)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t oem_meta_size = 0;
	uint32_t qc_meta_size = 0;
	const uint8_t *seg = NULL;
	const uint8_t *oem_meta = NULL;
	size_t oem_meta_len = 0;
	uint32_t version = 0;
	size_t hdr_size = 0;
	size_t seg_size = 0;
	size_t cursor = 0;

	if (!md || !md_size || !root_cert_sel)
		return TEE_ERROR_BAD_PARAMETERS;

	res = find_hashseg(md, md_size, &seg, &seg_size);
	if (res)
		return res;

	if (seg_size < MBN_HDR_SIZE_V5)
		return TEE_ERROR_BAD_FORMAT;

	version = read_u32(seg + MBN_OFF_VERSION);
	if (version == PAS_MBN_VERSION_5) {
		/* v5 carries no OEM metadata; caller uses the default. */
		return TEE_ERROR_NO_DATA;
	}
	if (version != PAS_MBN_VERSION_6) {
		EMSG("PAS auth: unsupported MBN version %"PRIu32, version);
		return TEE_ERROR_BAD_FORMAT;
	}

	hdr_size = MBN_HDR_SIZE_V6;
	if (seg_size < hdr_size)
		return TEE_ERROR_BAD_FORMAT;

	qc_meta_size = read_u32(seg + MBN_OFF_QC_META_SIZE);
	oem_meta_size = read_u32(seg + MBN_OFF_OEM_META_SIZE);

	cursor = hdr_size;
	res = take_region(seg, seg_size, &cursor, qc_meta_size, &oem_meta,
			  &oem_meta_len);
	if (res)
		return res;
	res = take_region(seg, seg_size, &cursor, oem_meta_size, &oem_meta,
			  &oem_meta_len);
	if (res)
		return res;

	if (!oem_meta)
		return TEE_ERROR_NO_DATA;

	if (oem_meta_len < META_MIN_WORDS * sizeof(uint32_t))
		return TEE_ERROR_BAD_FORMAT;

	*root_cert_sel = read_u32(oem_meta + META_OFF_ROOT_CERT_SEL *
				  sizeof(uint32_t));

	return TEE_SUCCESS;
}

TEE_Result pas_hashseg_parse(const uint8_t *md, size_t md_size,
			     uint32_t hash_size, struct pas_hashseg *out)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t oem_cert_size = 0;
	uint32_t oem_meta_size = 0;
	const uint8_t *seg = NULL;
	uint32_t oem_sig_size = 0;
	uint32_t qc_cert_size = 0;
	uint32_t qc_meta_size = 0;
	uint32_t qc_sig_size = 0;
	size_t signed_size = 0;
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
	qc_sig_size = read_u32(seg + MBN_OFF_QC_SIG_SIZE);
	qc_cert_size = read_u32(seg + MBN_OFF_QC_CERT_SIZE);
	oem_sig_size = read_u32(seg + MBN_OFF_OEM_SIG_SIZE);
	oem_cert_size = read_u32(seg + MBN_OFF_OEM_CERT_SIZE);
	if (version == PAS_MBN_VERSION_6) {
		qc_meta_size = read_u32(seg + MBN_OFF_QC_META_SIZE);
		oem_meta_size = read_u32(seg + MBN_OFF_OEM_META_SIZE);
	}

	if (!code_size || code_size % hash_size)
		return TEE_ERROR_BAD_FORMAT;

	/*
	 * Payload after the header:
	 *   [qc_meta][oem_meta][hash table][qc_sig][qc_cert][oem_sig][oem_cert]
	 * The signature covers the MBN header followed by
	 * [qc_meta || oem_meta || hash table], so the signed region spans the
	 * header and that payload from the segment start.
	 */
	cursor = hdr_size;
	out->signed_region = seg;

	if (ADD_OVERFLOW(qc_meta_size, oem_meta_size, &signed_size) ||
	    ADD_OVERFLOW(signed_size, code_size, &signed_size) ||
	    ADD_OVERFLOW(signed_size, hdr_size, &signed_size))
		return TEE_ERROR_BAD_FORMAT;

	if (signed_size > seg_size)
		return TEE_ERROR_BAD_FORMAT;
	out->signed_region_size = signed_size;

	res = take_region(seg, seg_size, &cursor, qc_meta_size,
			  &out->qti_meta, &out->qti_meta_size);
	if (res)
		return res;
	res = take_region(seg, seg_size, &cursor, oem_meta_size,
			  &out->oem_meta, &out->oem_meta_size);
	if (res)
		return res;

	out->hash_table = seg + cursor;
	out->hash_table_size = code_size;
	out->hash_size = hash_size;
	out->num_entries = code_size / hash_size;
	cursor += code_size;

	res = take_region(seg, seg_size, &cursor, qc_sig_size,
			  &out->qti_sig, &out->qti_sig_size);
	if (res)
		return res;
	res = take_region(seg, seg_size, &cursor, qc_cert_size,
			  &out->qti_certs, &out->qti_certs_size);
	if (res)
		return res;
	res = take_region(seg, seg_size, &cursor, oem_sig_size,
			  &out->oem_sig, &out->oem_sig_size);
	if (res)
		return res;
	res = take_region(seg, seg_size, &cursor, oem_cert_size,
			  &out->oem_certs, &out->oem_certs_size);
	if (res)
		return res;

	out->version = version;

	DMSG("PAS auth: MBN v%"PRIu32", %"PRIu32" entries", version,
	     out->num_entries);

	return TEE_SUCCESS;
}

TEE_Result pas_hashseg_get_meta(const struct pas_hashseg *hs,
				struct pas_meta *meta)
{
	const uint8_t *m = NULL;
	size_t i = 0;

	if (!hs || !meta)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!hs->oem_meta || !hs->oem_meta_size)
		return TEE_ERROR_NO_DATA;

	if (hs->oem_meta_size < META_MIN_WORDS * sizeof(uint32_t))
		return TEE_ERROR_BAD_FORMAT;

	m = hs->oem_meta;
	meta->major = read_u32(m + META_OFF_MAJOR * sizeof(uint32_t));
	meta->minor = read_u32(m + META_OFF_MINOR * sizeof(uint32_t));
	meta->sw_id = read_u32(m + META_OFF_SW_ID * sizeof(uint32_t));
	meta->hw_id = read_u32(m + META_OFF_HW_ID * sizeof(uint32_t));
	meta->oem_id = read_u32(m + META_OFF_OEM_ID * sizeof(uint32_t));
	meta->model_id = read_u32(m + META_OFF_MODEL_ID * sizeof(uint32_t));
	meta->secondary_sw_id = read_u32(m + META_OFF_SECONDARY_SW_ID *
					 sizeof(uint32_t));
	meta->flags = read_u32(m + META_OFF_FLAGS * sizeof(uint32_t));
	for (i = 0; i < META_NUM_SOC_VERS; i++)
		meta->soc_vers[i] = read_u32(m + (META_OFF_SOC_VERS + i) *
					     sizeof(uint32_t));
	for (i = 0; i < META_NUM_SERIAL_NUM; i++)
		meta->serial_num[i] = read_u32(m + (META_OFF_SERIAL_NUM + i) *
					       sizeof(uint32_t));
	meta->root_cert_sel = read_u32(m + META_OFF_ROOT_CERT_SEL *
				       sizeof(uint32_t));
	meta->anti_rollback = read_u32(m + META_OFF_ANTI_ROLLBACK *
				       sizeof(uint32_t));

	return TEE_SUCCESS;
}

/* Zero a metadata sub-block within the signed-region copy, if present. */
static void mask_meta_block(uint8_t *copy, size_t copy_len,
			    const uint8_t *block, size_t block_len,
			    const uint8_t *base)
{
	size_t off = 0;

	if (!block || !block_len)
		return;

	off = (size_t)(block - base);
	if (off < copy_len && block_len <= copy_len - off)
		memset(copy + off, 0, block_len);
}

/* Zero a header uint32_t field at @off within the signed-region copy. */
static void zero_field(uint8_t *copy, size_t copy_len, size_t off)
{
	if (off + sizeof(uint32_t) <= copy_len)
		memset(copy + off, 0, sizeof(uint32_t));
}

TEE_Result pas_hashseg_signed_copy(const struct pas_hashseg *hs,
				   enum pas_signer signer,
				   uint8_t **out, size_t *out_len)
{
	uint8_t *copy = NULL;

	if (!hs || !hs->signed_region || !hs->signed_region_size || !out ||
	    !out_len)
		return TEE_ERROR_BAD_PARAMETERS;

	copy = TEE_Malloc(hs->signed_region_size, TEE_MALLOC_FILL_ZERO);
	if (!copy)
		return TEE_ERROR_OUT_OF_MEMORY;

	memcpy(copy, hs->signed_region, hs->signed_region_size);

	/*
	 * Each signer signs the region with the other signer's header size
	 * fields and metadata block zeroed. The header sits at the start of
	 * the signed region, so its size-field offsets index directly into
	 * the copy.
	 */
	if (signer == PAS_SIGNER_OEM) {
		zero_field(copy, hs->signed_region_size, MBN_OFF_QC_SIG_SIZE);
		zero_field(copy, hs->signed_region_size, MBN_OFF_QC_CERT_SIZE);
		mask_meta_block(copy, hs->signed_region_size, hs->qti_meta,
				hs->qti_meta_size, hs->signed_region);
	} else {
		zero_field(copy, hs->signed_region_size, MBN_OFF_OEM_SIG_SIZE);
		zero_field(copy, hs->signed_region_size, MBN_OFF_OEM_CERT_SIZE);
		mask_meta_block(copy, hs->signed_region_size, hs->oem_meta,
				hs->oem_meta_size, hs->signed_region);
	}

	*out = copy;
	*out_len = hs->signed_region_size;

	return TEE_SUCCESS;
}
