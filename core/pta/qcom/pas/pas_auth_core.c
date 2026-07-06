// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <crypto/crypto.h>
#include <elf32.h>
#include <elf64.h>
#include <pas_auth_core.h>
#include <string.h>
#include <string_ext.h>
#include <trace.h>
#include <utee_defines.h>
#include <inttypes.h>
#include <util.h>

static bool range_ok(size_t off, size_t len, size_t total)
{
	size_t end = 0;

	if (ADD_OVERFLOW(off, len, &end))
		return false;

	return end <= total;
}

/*
 * Qualcomm program-header flags encode segment type and access type.
 * QCOM_MDT_RELOCATABLE (bit 27) marks a relocatable image.
 */
#define MI_PBT_PAGE_MODE_MASK		0x00100000
#define MI_PBT_PAGE_MODE_SHIFT		20
#define MI_PBT_ACCESS_TYPE_MASK		0x00E00000
#define MI_PBT_ACCESS_TYPE_SHIFT	21
#define MI_PBT_SEGMENT_TYPE_MASK	0x07000000
#define MI_PBT_SEGMENT_TYPE_SHIFT	24

#define MI_PBT_NON_PAGED_SEGMENT	0x0
#define MI_PBT_HASH_SEGMENT		0x2
#define MI_PBT_NOTUSED_SEGMENT		0x3
#define MI_PBT_SHARED_SEGMENT		0x4

#define QCOM_MDT_RELOCATABLE		BIT(27)

#define MI_PBT_PAGE_MODE(x) \
	(((x) & MI_PBT_PAGE_MODE_MASK) >> MI_PBT_PAGE_MODE_SHIFT)
#define MI_PBT_ACCESS_TYPE(x) \
	(((x) & MI_PBT_ACCESS_TYPE_MASK) >> MI_PBT_ACCESS_TYPE_SHIFT)
#define MI_PBT_SEGMENT_TYPE(x) \
	(((x) & MI_PBT_SEGMENT_TYPE_MASK) >> MI_PBT_SEGMENT_TYPE_SHIFT)

static bool is_hashed(uint32_t p_type, uint32_t p_flags)
{
	if (p_type != PT_LOAD)
		return false;

	return MI_PBT_PAGE_MODE(p_flags) == MI_PBT_NON_PAGED_SEGMENT &&
	       MI_PBT_SEGMENT_TYPE(p_flags) != MI_PBT_HASH_SEGMENT &&
	       MI_PBT_ACCESS_TYPE(p_flags) != MI_PBT_NOTUSED_SEGMENT &&
	       MI_PBT_ACCESS_TYPE(p_flags) != MI_PBT_SHARED_SEGMENT;
}

struct elf_info {
	size_t ehsize;
	size_t phoff;
	size_t phentsize;
	size_t phnum;
	bool is_64;
};

static TEE_Result parse_elf(const uint8_t *fw, size_t fw_size,
			    struct elf_info *info)
{
	const unsigned char *ident = fw;

	if (fw_size < EI_NIDENT)
		return TEE_ERROR_BAD_FORMAT;

	if (ident[EI_MAG0] != ELFMAG0 || ident[EI_MAG1] != ELFMAG1 ||
	    ident[EI_MAG2] != ELFMAG2 || ident[EI_MAG3] != ELFMAG3)
		return TEE_ERROR_BAD_FORMAT;

	if (ident[EI_CLASS] == ELFCLASS64) {
		const Elf64_Ehdr *ehdr = (const void *)fw;

		if (fw_size < sizeof(*ehdr))
			return TEE_ERROR_BAD_FORMAT;

		if (ehdr->e_phentsize < sizeof(Elf64_Phdr))
			return TEE_ERROR_BAD_FORMAT;

		info->is_64 = true;
		info->ehsize = ehdr->e_ehsize;
		info->phoff = ehdr->e_phoff;
		info->phentsize = ehdr->e_phentsize;
		info->phnum = ehdr->e_phnum;
	} else if (ident[EI_CLASS] == ELFCLASS32) {
		const Elf32_Ehdr *ehdr = (const void *)fw;

		if (fw_size < sizeof(*ehdr))
			return TEE_ERROR_BAD_FORMAT;

		if (ehdr->e_phentsize < sizeof(Elf32_Phdr))
			return TEE_ERROR_BAD_FORMAT;

		info->is_64 = false;
		info->ehsize = ehdr->e_ehsize;
		info->phoff = ehdr->e_phoff;
		info->phentsize = ehdr->e_phentsize;
		info->phnum = ehdr->e_phnum;
	} else {
		return TEE_ERROR_BAD_FORMAT;
	}

	return TEE_SUCCESS;
}

static void get_phdr(const uint8_t *fw, const struct elf_info *info,
		     size_t idx, uint32_t *p_type, uint32_t *p_flags,
		     uint64_t *p_paddr, size_t *p_filesz, size_t *p_memsz)
{
	const uint8_t *p = fw + info->phoff + idx * info->phentsize;

	if (info->is_64) {
		const Elf64_Phdr *phdr = (const void *)p;

		*p_type = phdr->p_type;
		*p_flags = phdr->p_flags;
		*p_paddr = phdr->p_paddr;
		*p_filesz = phdr->p_filesz;
		*p_memsz = phdr->p_memsz;
	} else {
		const Elf32_Phdr *phdr = (const void *)p;

		*p_type = phdr->p_type;
		*p_flags = phdr->p_flags;
		*p_paddr = phdr->p_paddr;
		*p_filesz = phdr->p_filesz;
		*p_memsz = phdr->p_memsz;
	}
}

/*
 * Loader predicate matching the kernel MDT loader's mdt_phdr_loadable(): a
 * segment contributes to the relocation base only if it is PT_LOAD, not the
 * hash segment, and has a nonzero memory size. This is deliberately distinct
 * from is_hashed() (the hashing filter, which also excludes paged/NOTUSED/
 * SHARED segments): the relocation base must be computed over exactly the set
 * the kernel loader used to place the segments.
 */
static bool mdt_loadable(uint32_t p_type, uint32_t p_flags, size_t p_memsz)
{
	return p_type == PT_LOAD &&
	       MI_PBT_SEGMENT_TYPE(p_flags) != MI_PBT_HASH_SEGMENT &&
	       p_memsz != 0;
}

/*
 * Mirror qcom_mdt_load_no_init(): if any loadable phdr has the RELOCATABLE
 * bit, segments are placed from min(p_paddr) over the loadable set; otherwise
 * from fw_phys.
 */
static uint64_t reloc_base(const uint8_t *fw, const struct elf_info *info,
			   paddr_t fw_phys)
{
	uint64_t min_paddr = UINT64_MAX;
	bool relocatable = false;
	uint64_t p_paddr = 0;
	uint32_t p_flags = 0;
	uint32_t p_type = 0;
	size_t p_filesz = 0;
	size_t p_memsz = 0;
	size_t i = 0;

	for (i = 0; i < info->phnum; i++) {
		get_phdr(fw, info, i, &p_type, &p_flags, &p_paddr, &p_filesz,
			 &p_memsz);

		if (!mdt_loadable(p_type, p_flags, p_memsz))
			continue;

		if (p_flags & QCOM_MDT_RELOCATABLE)
			relocatable = true;

		if (p_paddr < min_paddr)
			min_paddr = p_paddr;
	}

	if (relocatable && min_paddr != UINT64_MAX)
		return min_paddr;

	return fw_phys;
}

static TEE_Result hash_verify(uint32_t algo, const uint8_t *data,
			      size_t data_len, const uint8_t *expected,
			      size_t hash_size)
{
	uint8_t dgst[PAS_AUTH_CORE_MAX_HASH_SIZE] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	void *ctx = NULL;

	if (hash_size > sizeof(dgst))
		return TEE_ERROR_BAD_PARAMETERS;

	res = crypto_hash_alloc_ctx(&ctx, algo);
	if (res)
		return res;

	res = crypto_hash_init(ctx);
	if (res)
		goto out;

	res = crypto_hash_update(ctx, data, data_len);
	if (res)
		goto out;

	res = crypto_hash_final(ctx, dgst, hash_size);
	if (res)
		goto out;

	if (consttime_memcmp(dgst, expected, hash_size))
		res = TEE_ERROR_SECURITY;
	else
		res = TEE_SUCCESS;
out:
	crypto_hash_free_ctx(ctx);
	memzero_explicit(dgst, sizeof(dgst));

	return res;
}

static TEE_Result verify_elf_header(const struct pas_auth_core_ctx *ctx,
				    const struct elf_info *info)
{
	size_t hdr_len = 0;

	if (MUL_OVERFLOW(info->phentsize, info->phnum, &hdr_len) ||
	    ADD_OVERFLOW(hdr_len, info->ehsize, &hdr_len) ||
	    hdr_len > ctx->metadata_size)
		return TEE_ERROR_BAD_FORMAT;

	return hash_verify(ctx->hash_algo, ctx->metadata, hdr_len,
			   ctx->hash_table, ctx->hash_size);
}

TEE_Result pas_auth_core_verify_segments(const struct pas_auth_core_ctx *ctx)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	const uint8_t *expected = NULL;
	struct elf_info info = { };
	size_t phtab_size = 0;
	uint64_t p_paddr = 0;
	uint32_t p_flags = 0;
	uint64_t offset = 0;
	uint32_t p_type = 0;
	size_t p_filesz = 0;
	size_t p_memsz = 0;
	size_t verified = 0;
	uint64_t base = 0;
	size_t i = 0;

	if (!ctx || !ctx->hash_table || !ctx->fw || !ctx->metadata)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!ctx->hash_size || ctx->hash_size > PAS_AUTH_CORE_MAX_HASH_SIZE)
		return TEE_ERROR_BAD_PARAMETERS;

	/*
	 * Parse the ELF from the metadata blob (ELF header + phdrs + hash seg),
	 * NOT from the carveout. The carveout holds only the loaded segments;
	 * the ELF header is not placed there by the MDT loader.
	 */
	res = parse_elf(ctx->metadata, ctx->metadata_size, &info);
	if (res) {
		EMSG("PAS auth: ELF parse from metadata failed: %#"PRIx32, res);
		return res;
	}

	if (MUL_OVERFLOW(info.phentsize, info.phnum, &phtab_size) ||
	    !range_ok(info.phoff, phtab_size, ctx->metadata_size)) {
		EMSG("PAS auth: phdr table oob phentsize=%zu phnum=%zu md=%zu",
		     info.phentsize, info.phnum, ctx->metadata_size);
		return TEE_ERROR_BAD_FORMAT;
	}

	if (ctx->num_entries != info.phnum) {
		EMSG("PAS auth: hash entries %"PRIu32" != phnum %zu",
		     ctx->num_entries, info.phnum);
		return TEE_ERROR_SECURITY;
	}

	/* Entry 0: hash of ELF header + phdr table from metadata */
	res = verify_elf_header(ctx, &info);
	if (res) {
		EMSG("PAS auth: ELF header hash mismatch");
		return res;
	}

	/* Reloc base: for relocatable images = min(p_paddr); else = fw_phys */
	base = reloc_base(ctx->metadata, &info, ctx->fw_phys);
	DMSG("PAS auth: fw_phys=%#"PRIx64" fw_size=%zu base=%#"PRIx64,
	     (uint64_t)ctx->fw_phys, ctx->fw_size, base);

	for (i = 0; i < info.phnum; i++) {
		get_phdr(ctx->metadata, &info, i, &p_type, &p_flags, &p_paddr,
			 &p_filesz, &p_memsz);

		if (!is_hashed(p_type, p_flags) || !p_filesz)
			continue;

		if (p_paddr < base) {
			EMSG("PAS auth: seg %zu paddr=%#"PRIx64
			     " < base=%#"PRIx64, i, p_paddr, base);
			return TEE_ERROR_BAD_FORMAT;
		}
		offset = p_paddr - base;

		if (offset > ctx->fw_size ||
		    !range_ok(offset, p_filesz, ctx->fw_size)) {
			EMSG("PAS auth: seg %zu off=%#"PRIx64
			     " sz=%zu > fw_sz=%zu",
			     i, offset, p_filesz, ctx->fw_size);
			return TEE_ERROR_BAD_FORMAT;
		}

		expected = ctx->hash_table + i * ctx->hash_size;

		DMSG("PAS auth: seg %zu paddr=%#"PRIx64" off=%#"PRIx64" sz=%zu",
		     i, p_paddr, offset, p_filesz);

		res = hash_verify(ctx->hash_algo, ctx->fw + offset, p_filesz,
				  expected, ctx->hash_size);
		if (res) {
			EMSG("PAS auth: segment %zu hash mismatch", i);
			return res;
		}

		verified++;
	}

	DMSG("PAS auth: ELF header + %zu segment(s) verified", verified);

	return TEE_SUCCESS;
}
