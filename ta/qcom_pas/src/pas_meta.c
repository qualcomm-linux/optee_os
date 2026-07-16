// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <elf32.h>
#include <elf64.h>
#include <pas_mbn_parser_priv.h>
#include <pas_meta.h>
#include <string.h>
#include <string_ext.h>
#include <tee_internal_api.h>
#include <utee_defines.h>
#include <util.h>

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

TEE_Result pas_meta_peek_version(const uint8_t *meta_data,
				 size_t meta_data_size, uint32_t *version)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	const uint8_t *seg = NULL;
	size_t seg_size = 0;

	if (!meta_data || !meta_data_size || !version)
		return TEE_ERROR_BAD_PARAMETERS;

	res = pas_mbn_locate(meta_data, meta_data_size, &seg, &seg_size);
	if (res)
		return res;

	if (seg_size < MBN_HDR_SIZE_V5)
		return TEE_ERROR_BAD_FORMAT;

	*version = pas_mbn_read_u32(seg + MBN_OFF_VERSION);

	return TEE_SUCCESS;
}

TEE_Result pas_meta_peek_root_cert_sel(const uint8_t *meta_data,
				       size_t meta_data_size,
				       uint32_t *root_cert_sel)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t oem_meta_size = 0;
	uint32_t qc_meta_size = 0;
	const uint8_t *oem_meta = NULL;
	const uint8_t *qc_meta = NULL;
	const uint8_t *segment = NULL;
	size_t oem_meta_len = 0;
	size_t qc_meta_len = 0;
	uint32_t version = 0;
	size_t hdr_size = 0;
	size_t segment_size = 0;
	size_t offset = 0;

	if (!meta_data || !meta_data_size || !root_cert_sel)
		return TEE_ERROR_BAD_PARAMETERS;

	res = pas_mbn_locate(meta_data, meta_data_size, &segment,
			     &segment_size);
	if (res)
		return res;

	if (segment_size < MBN_HDR_SIZE_V5)
		return TEE_ERROR_BAD_FORMAT;

	version = pas_mbn_read_u32(segment + MBN_OFF_VERSION);
	if (version == PAS_MBN_VERSION_5) {
		/* v5 carries no OEM metadata; caller uses the default. */
		return TEE_ERROR_NO_DATA;
	}
	if (version != PAS_MBN_VERSION_6) {
		EMSG("PAS auth: unsupported MBN version %"PRIu32, version);
		return TEE_ERROR_BAD_FORMAT;
	}

	hdr_size = MBN_HDR_SIZE_V6;
	if (segment_size < hdr_size)
		return TEE_ERROR_BAD_FORMAT;

	qc_meta_size = pas_mbn_read_u32(segment + MBN_OFF_QC_META_SIZE);
	oem_meta_size = pas_mbn_read_u32(segment + MBN_OFF_OEM_META_SIZE);

	offset = hdr_size;
	/* Skip the QC metadata; only the OEM block carries root_cert_sel. */
	res = pas_mbn_reserve_region(segment, segment_size, &offset,
				     qc_meta_size, &qc_meta, &qc_meta_len);
	if (res)
		return res;
	res = pas_mbn_reserve_region(segment, segment_size, &offset,
				     oem_meta_size, &oem_meta, &oem_meta_len);
	if (res)
		return res;

	if (!oem_meta)
		return TEE_ERROR_NO_DATA;

	if (oem_meta_len < META_MIN_WORDS * sizeof(uint32_t))
		return TEE_ERROR_BAD_FORMAT;

	*root_cert_sel = pas_mbn_read_u32(oem_meta + META_OFF_ROOT_CERT_SEL *
					      sizeof(uint32_t));

	return TEE_SUCCESS;
}

/*
 * Entry 0 digests exactly ehdr_size + phentsize * phnum from the metadata
 * blob - the canonical entry-0 definition the signing tool commits to.
 * Using pas_mbn_locate()'s preamble instead would silently include any
 * padding between the phdr table and the hash segment, diverging from the
 * signed digest.
 */
TEE_Result pas_meta_verify_preamble(const uint8_t *meta_data,
				    size_t meta_data_size,
				    const uint8_t *hash_table,
				    uint32_t hash_size)
{
	uint8_t dgst[TEE_SHA384_HASH_SIZE] = { };
	TEE_OperationHandle op = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	size_t dgst_len = sizeof(dgst);
	size_t ehdr_size = 0;
	size_t phentsize = 0;
	size_t phnum = 0;
	size_t hdr_len = 0;
	uint32_t algo = 0;

	if (!meta_data || !meta_data_size || !hash_table)
		return TEE_ERROR_BAD_PARAMETERS;

	switch (hash_size) {
	case TEE_SHA256_HASH_SIZE:
		algo = TEE_ALG_SHA256;
		break;
	case TEE_SHA384_HASH_SIZE:
		algo = TEE_ALG_SHA384;
		break;
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}

	if (meta_data_size < EI_NIDENT)
		return TEE_ERROR_BAD_FORMAT;

	if (meta_data[EI_MAG0] != ELFMAG0 || meta_data[EI_MAG1] != ELFMAG1 ||
	    meta_data[EI_MAG2] != ELFMAG2 || meta_data[EI_MAG3] != ELFMAG3)
		return TEE_ERROR_BAD_FORMAT;

	if (meta_data[EI_CLASS] == ELFCLASS64) {
		const Elf64_Ehdr *ehdr = (const void *)meta_data;

		if (meta_data_size < sizeof(*ehdr))
			return TEE_ERROR_BAD_FORMAT;
		ehdr_size = ehdr->e_ehsize;
		phentsize = ehdr->e_phentsize;
		phnum = ehdr->e_phnum;
	} else if (meta_data[EI_CLASS] == ELFCLASS32) {
		const Elf32_Ehdr *ehdr = (const void *)meta_data;

		if (meta_data_size < sizeof(*ehdr))
			return TEE_ERROR_BAD_FORMAT;
		ehdr_size = ehdr->e_ehsize;
		phentsize = ehdr->e_phentsize;
		phnum = ehdr->e_phnum;
	} else {
		return TEE_ERROR_BAD_FORMAT;
	}

	if (MUL_OVERFLOW(phentsize, phnum, &hdr_len) ||
	    ADD_OVERFLOW(hdr_len, ehdr_size, &hdr_len) ||
	    hdr_len > meta_data_size)
		return TEE_ERROR_BAD_FORMAT;

	res = TEE_AllocateOperation(&op, algo, TEE_MODE_DIGEST, 0);
	if (res != TEE_SUCCESS)
		return res;

	res = TEE_DigestDoFinal(op, meta_data, hdr_len, dgst, &dgst_len);
	if (res != TEE_SUCCESS)
		goto out;

	if (dgst_len != hash_size ||
	    consttime_memcmp(dgst, hash_table, hash_size) != 0)
		res = TEE_ERROR_SECURITY;
	else
		res = TEE_SUCCESS;
out:
	TEE_FreeOperation(op);
	memzero_explicit(dgst, sizeof(dgst));

	return res;
}

TEE_Result pas_meta_get(const struct pas_mbn *hs, struct pas_meta *meta)
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
	meta->major = pas_mbn_read_u32(m + META_OFF_MAJOR *
					   sizeof(uint32_t));
	meta->minor = pas_mbn_read_u32(m + META_OFF_MINOR *
					   sizeof(uint32_t));
	meta->sw_id = pas_mbn_read_u32(m + META_OFF_SW_ID *
					   sizeof(uint32_t));
	meta->hw_id = pas_mbn_read_u32(m + META_OFF_HW_ID *
					   sizeof(uint32_t));
	meta->oem_id = pas_mbn_read_u32(m + META_OFF_OEM_ID *
					    sizeof(uint32_t));
	meta->model_id = pas_mbn_read_u32(m + META_OFF_MODEL_ID *
					      sizeof(uint32_t));
	meta->secondary_sw_id = pas_mbn_read_u32(m +
						     META_OFF_SECONDARY_SW_ID *
						     sizeof(uint32_t));
	meta->flags = pas_mbn_read_u32(m + META_OFF_FLAGS *
					   sizeof(uint32_t));
	for (i = 0; i < META_NUM_SOC_VERS; i++)
		meta->soc_vers[i] = pas_mbn_read_u32(m + (META_OFF_SOC_VERS +
							  i) *
						     sizeof(uint32_t));
	for (i = 0; i < META_NUM_SERIAL_NUM; i++)
		meta->serial_num[i] = pas_mbn_read_u32(m +
						       (META_OFF_SERIAL_NUM +
							i) * sizeof(uint32_t));
	meta->root_cert_sel = pas_mbn_read_u32(m + META_OFF_ROOT_CERT_SEL *
						   sizeof(uint32_t));
	meta->anti_rollback = pas_mbn_read_u32(m + META_OFF_ANTI_ROLLBACK *
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

TEE_Result pas_meta_signed_copy(const struct pas_mbn *hs,
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
	 * The OEM signature is computed over the signed region with the QTI
	 * header size fields and QTI metadata block zeroed. For an OEM-only
	 * image these are already zero and hs->qti_meta is NULL, so the
	 * masking is a no-op.
	 */
	zero_field(copy, hs->signed_region_size, MBN_OFF_QC_SIG_SIZE);
	zero_field(copy, hs->signed_region_size, MBN_OFF_QC_CERT_SIZE);
	mask_meta_block(copy, hs->signed_region_size, hs->qti_meta,
			hs->qti_meta_size, hs->signed_region);

	*out = copy;
	*out_len = hs->signed_region_size;

	return TEE_SUCCESS;
}
