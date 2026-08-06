// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <elf32.h>
#include <elf64.h>
#include <kernel/cache_helpers.h>
#include <mm/core_mmu.h>
#include <pas_auth_core.h>
#include <platform_pas.h>
#include <string.h>
#include <string_ext.h>
#include <tee/tee_cryp_utl.h>
#include <trace.h>
#include <utee_defines.h>
#include <util.h>

#include "pas_subsys.h"

/* Bounds check in u64 to avoid narrowing 64-bit ELF fields. */
static bool check_range(uint64_t off, uint64_t len, uint64_t total)
{
	uint64_t end = 0;

	if (ADD_OVERFLOW(off, len, &end))
		return false;

	return end <= total;
}

#define MI_PBT_PAGE_MODE_MASK		0x00100000
#define MI_PBT_PAGE_MODE_SHIFT		20
#define MI_PBT_ACCESS_TYPE_MASK		0x00E00000
#define MI_PBT_ACCESS_TYPE_SHIFT	21
#define MI_PBT_SEGMENT_TYPE_MASK	0x07000000
#define MI_PBT_SEGMENT_TYPE_SHIFT	24

#define MI_PBT_NON_PAGED_SEGMENT	0x0
#define MI_PBT_HASH_SEGMENT		0x2
/*
 * NOTUSED/SHARED are access-type field values (not segment-type values); the
 * MI_PBT_*_SEGMENT naming preserves the wire-format identifier names.
 */
#define MI_PBT_NOTUSED_SEGMENT		0x3
#define MI_PBT_SHARED_SEGMENT		0x4

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
	size_t ehdr_size = 0;
	size_t phent_size = 0;

	if (fw_size < EI_NIDENT)
		return TEE_ERROR_BAD_FORMAT;

	if (ident[EI_MAG0] != ELFMAG0 || ident[EI_MAG1] != ELFMAG1 ||
	    ident[EI_MAG2] != ELFMAG2 || ident[EI_MAG3] != ELFMAG3)
		return TEE_ERROR_BAD_FORMAT;

	switch (ident[EI_CLASS]) {
	case ELFCLASS64:
		info->is_64 = true;
		ehdr_size = sizeof(Elf64_Ehdr);
		phent_size = sizeof(Elf64_Phdr);
		break;
	case ELFCLASS32:
		info->is_64 = false;
		ehdr_size = sizeof(Elf32_Ehdr);
		phent_size = sizeof(Elf32_Phdr);
		break;
	default:
		return TEE_ERROR_BAD_FORMAT;
	}

	if (fw_size < ehdr_size)
		return TEE_ERROR_BAD_FORMAT;

	if (info->is_64) {
		const Elf64_Ehdr *ehdr = (const void *)fw;

		info->ehsize = ehdr->e_ehsize;
		info->phoff = ehdr->e_phoff;
		info->phentsize = ehdr->e_phentsize;
		info->phnum = ehdr->e_phnum;
	} else {
		const Elf32_Ehdr *ehdr = (const void *)fw;

		info->ehsize = ehdr->e_ehsize;
		info->phoff = ehdr->e_phoff;
		info->phentsize = ehdr->e_phentsize;
		info->phnum = ehdr->e_phnum;
	}

	/*
	 * A signer that shrinks e_ehsize below the class's real Ehdr size
	 * would make verify_elf_header() hash fewer bytes than an unmodified
	 * image, silently narrowing entry-0 coverage. Reject up front.
	 */
	if (info->ehsize < ehdr_size)
		return TEE_ERROR_BAD_FORMAT;

	if (info->phentsize < phent_size)
		return TEE_ERROR_BAD_FORMAT;

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

/* min(paddr) over the hashed-segment set. */
static uint64_t reloc_base(const uint8_t *fw, const struct elf_info *info)
{
	uint64_t min_paddr = UINT64_MAX;
	uint64_t paddr = 0;
	uint32_t flags = 0;
	uint32_t type = 0;
	size_t file_len = 0;
	size_t mem_len = 0;
	size_t i = 0;

	for (i = 0; i < info->phnum; i++) {
		get_phdr(fw, info, i, &type, &flags, &paddr, &file_len,
			 &mem_len);

		if (!is_hashed(type, flags) || !file_len)
			continue;

		if (paddr < min_paddr)
			min_paddr = paddr;
	}

	return min_paddr;
}

static TEE_Result hash_verify(uint32_t algo, const uint8_t *data,
			      size_t data_len, const uint8_t *expected,
			      size_t hash_size)
{
	uint8_t dgst[PAS_AUTH_CORE_MAX_HASH_SIZE] = { };
	TEE_Result res = TEE_ERROR_GENERIC;

	if (hash_size > sizeof(dgst))
		return TEE_ERROR_BAD_PARAMETERS;

	res = tee_hash_createdigest(algo, data, data_len, dgst, hash_size);
	if (res)
		return res;

	if (consttime_memcmp(dgst, expected, hash_size))
		res = TEE_ERROR_SECURITY;
	else
		res = TEE_SUCCESS;

	memzero_explicit(dgst, sizeof(dgst));

	return res;
}

static TEE_Result verify_elf_header(const struct pas_auth_core_ctx *ctx,
				    const struct elf_info *info)
{
	size_t hdr_len = 0;

	if (MUL_OVERFLOW(info->phentsize, info->phnum, &hdr_len))
		return TEE_ERROR_BAD_FORMAT;

	if (ADD_OVERFLOW(hdr_len, info->ehsize, &hdr_len))
		return TEE_ERROR_BAD_FORMAT;

	if (hdr_len > ctx->metadata_len)
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
	uint64_t paddr = 0;
	uint32_t flags = 0;
	uint64_t offset = 0;
	uint32_t type = 0;
	size_t file_len = 0;
	size_t mem_len = 0;
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
	 * the ELF header is not present there.
	 */
	res = parse_elf(ctx->metadata, ctx->metadata_len, &info);
	if (res)
		return res;

	if (MUL_OVERFLOW(info.phentsize, info.phnum, &phtab_size) ||
	    !check_range(info.phoff, phtab_size, ctx->metadata_len))
		return TEE_ERROR_BAD_FORMAT;

	if (ctx->num_entries != info.phnum)
		return TEE_ERROR_SECURITY;

	res = verify_elf_header(ctx, &info);
	if (res) {
		EMSG("PAS auth: ELF header hash mismatch");
		return res;
	}

	/*
	 * UINT64_MAX when no hashed segments; harmless since the loop below
	 * skips non-hashed entries and the post-loop "verified" check rejects
	 * a zero-segment image.
	 */
	base = reloc_base(ctx->metadata, &info);

	for (i = 0; i < info.phnum; i++) {
		get_phdr(ctx->metadata, &info, i, &type, &flags, &paddr,
			 &file_len, &mem_len);

		if (!is_hashed(type, flags) || !file_len)
			continue;

		if (paddr < base)
			return TEE_ERROR_BAD_FORMAT;
		offset = paddr - base;

		if (!check_range(offset, file_len, ctx->fw_size))
			return TEE_ERROR_BAD_FORMAT;

		/*
		 * A segment's [file_len, mem_len) ZI tail must be inside the
		 * carveout and zeroed before hashing so stale REE bytes cannot
		 * slip past the digest or run as uninitialized data.
		 * mem_len < file_len is malformed.
		 */
		if (mem_len < file_len)
			return TEE_ERROR_BAD_FORMAT;
		if (!check_range(offset, mem_len, ctx->fw_size))
			return TEE_ERROR_BAD_FORMAT;
		if (mem_len > file_len) {
			void *zi = ctx->fw + offset + file_len;
			size_t zi_len = mem_len - file_len;

			memset(zi, 0, zi_len);

			/*
			 * This range is mapped cached here, but the REE maps
			 * the same carveout uncached and the DSP fetches it
			 * outside the CPU's coherency domain. Unmapping does
			 * no cache maintenance, so without an explicit flush
			 * these zeros could sit dirty in cache and reach DDR
			 * only after AUTH_AND_RESET released the DSP, which
			 * would clobber live DSP state. Clean+invalidate
			 * synchronously instead.
			 */
			dcache_cleaninv_range(zi, zi_len);
		}

		expected = ctx->hash_table + i * ctx->hash_size;

		res = hash_verify(ctx->hash_algo, ctx->fw + offset, file_len,
				  expected, ctx->hash_size);
		if (res) {
			EMSG("PAS auth: segment %zu hash mismatch", i);
			return res;
		}

		verified++;
	}

	/* Entry 0 alone is not enough: some loadable segment must be hashed. */
	if (!verified) {
		EMSG("PAS auth: no loadable segments were hashed");
		return TEE_ERROR_SECURITY;
	}

	return TEE_SUCCESS;
}

TEE_Result pas_platform_verify_image(uint32_t pas_id,
				     const struct pas_fw_region *fw,
				     const struct pas_metadata *metadata,
				     const struct pas_hash_table *hash)
{
	struct qcom_pas_subsys *subsys = pas_lookup(pas_id);
	struct pas_auth_core_ctx ctx = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	struct qcom_pas_data *data = NULL;
	void *fw_va = NULL;
	uint32_t fw_size = 0;

	if (!subsys)
		return TEE_ERROR_NOT_SUPPORTED;

	if (!metadata->data || !metadata->size || !hash->table ||
	    !hash->len || !fw->size)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!hash->entry_size || hash->len % hash->entry_size)
		return TEE_ERROR_BAD_PARAMETERS;

	data = &subsys->data;

	/*
	 * MEM_SETUP must run first: it caches the platform-validated
	 * (fw_base, fw_size). Without it, an REE-supplied fw_base is
	 * unverified and could aim the hash check at attacker-chosen memory.
	 */
	if (!data->fw_base || !data->fw_size) {
		EMSG("PAS auth: no MEM_SETUP for pas_id=%"PRIu32, pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	if (fw->base != data->fw_base) {
		EMSG("PAS auth: base %#"PRIxPA" != MEM_SETUP %#"PRIxPA,
		     fw->base, data->fw_base);
		return TEE_ERROR_SECURITY;
	}
	fw_size = data->fw_size; /* use image span for verification */

	fw_va = core_mmu_add_mapping(MEM_AREA_RAM_NSEC, fw->base, fw_size);
	if (!fw_va) {
		EMSG("PAS auth: can't map carveout %#"PRIxPA"/%#"PRIx32,
		     fw->base, fw_size);
		return TEE_ERROR_GENERIC;
	}

	switch (hash->entry_size) {
	case TEE_SHA256_HASH_SIZE:
		ctx.hash_algo = TEE_ALG_SHA256;
		break;
	case TEE_SHA384_HASH_SIZE:
		ctx.hash_algo = TEE_ALG_SHA384;
		break;
	default:
		res = TEE_ERROR_NOT_SUPPORTED;
		goto out;
	}

	ctx.hash_size = hash->entry_size;
	ctx.hash_table = hash->table;
	ctx.num_entries = hash->len / hash->entry_size;
	ctx.metadata = metadata->data;
	ctx.metadata_len = metadata->size;
	ctx.fw = fw_va;
	ctx.fw_size = fw_size;

	res = pas_auth_core_verify_segments(&ctx);
out:
	if (core_mmu_remove_mapping(MEM_AREA_RAM_NSEC, fw_va, fw_size))
		EMSG("PAS auth: failed to unmap carveout");

	return res;
}
