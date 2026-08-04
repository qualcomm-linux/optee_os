// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * ELF metadata validation for PIL firmware images.
 *
 * Before a PIL image's signature can be verified, its ELF header and
 * program-header table are validated, and the hash-segment program header
 * is located within them. The hash segment carries the boot-image header,
 * hash table, and signing metadata that the rest of the authentication
 * flow consumes.
 */

#include <pil_elf.h>
#include <pil_metadata.h>
#include <util.h>

static bool pil_elf_class_is_valid(uint8_t class)
{
	return class == ELF_CLASS_32 || class == ELF_CLASS_64;
}

static TEE_Result pil_elf_get_class(const void *elf_hdr, size_t elf_hdr_size,
				    uint8_t *elf_class)
{
	const uint8_t *e_ident = elf_hdr;

	if (elf_hdr_size < sizeof(struct elf32_hdr))
		return TEE_ERROR_BAD_PARAMETERS;

	if (e_ident[ELF_EI_MAG0] != ELF_MAG0 ||
	    e_ident[ELF_EI_MAG1] != ELF_MAG1 ||
	    e_ident[ELF_EI_MAG2] != ELF_MAG2 ||
	    e_ident[ELF_EI_MAG3] != ELF_MAG3)
		return TEE_ERROR_BAD_FORMAT;

	if (!pil_elf_class_is_valid(e_ident[ELF_EI_CLASS]))
		return TEE_ERROR_BAD_FORMAT;

	*elf_class = e_ident[ELF_EI_CLASS];

	return TEE_SUCCESS;
}

/*
 * Scan the program-header table for the hash-segment entry and return its
 * file size, the size of the hash segment that follows the ELF and
 * program headers in the image.
 */
static TEE_Result pil_elf_find_hash_segment(const void *phdr_table,
					    uint16_t phnum, uint8_t elf_class,
					    size_t *hash_seg_size)
{
	uint16_t i = 0;

	for (i = 0; i < phnum; i++) {
		uint32_t p_flags = 0;
		uint64_t p_filesz = 0;

		if (elf_class == ELF_CLASS_32) {
			const struct elf32_phdr *phdr = phdr_table;

			p_flags = phdr[i].p_flags;
			p_filesz = phdr[i].p_filesz;
		} else {
			const struct elf64_phdr *phdr = phdr_table;

			p_flags = phdr[i].p_flags;
			p_filesz = phdr[i].p_filesz;
		}

		if (PIL_SEGMENT_TYPE(p_flags) == PIL_SEGMENT_TYPE_HASH) {
			*hash_seg_size = p_filesz;
			return TEE_SUCCESS;
		}
	}

	return TEE_ERROR_ITEM_NOT_FOUND;
}

TEE_Result pil_validate_elf_metadata(const void *elf_hdr, size_t elf_hdr_size,
				     const void *phdr_table,
				     size_t phdr_table_size,
				     struct pil_elf_metadata *metadata)
{
	uint8_t elf_class = 0;
	uint16_t phnum = 0;
	size_t phdr_entry_size = 0;
	size_t computed_table_size = 0;
	TEE_Result res = TEE_SUCCESS;

	if (!elf_hdr || !phdr_table || !metadata)
		return TEE_ERROR_BAD_PARAMETERS;

	res = pil_elf_get_class(elf_hdr, elf_hdr_size, &elf_class);
	if (res != TEE_SUCCESS)
		return res;

	if (elf_class == ELF_CLASS_32) {
		const struct elf32_hdr *ehdr = elf_hdr;

		if (elf_hdr_size < sizeof(*ehdr))
			return TEE_ERROR_BAD_PARAMETERS;

		phnum = ehdr->e_phnum;
		phdr_entry_size = sizeof(struct elf32_phdr);
	} else {
		const struct elf64_hdr *ehdr = elf_hdr;

		if (elf_hdr_size < sizeof(*ehdr))
			return TEE_ERROR_BAD_PARAMETERS;

		phnum = ehdr->e_phnum;
		phdr_entry_size = sizeof(struct elf64_phdr);
	}

	if (MUL_OVERFLOW(phnum, phdr_entry_size, &computed_table_size))
		return TEE_ERROR_OVERFLOW;

	if (computed_table_size != phdr_table_size)
		return TEE_ERROR_BAD_PARAMETERS;

	res = pil_elf_find_hash_segment(phdr_table, phnum, elf_class,
					&metadata->hash_seg_size);
	if (res != TEE_SUCCESS)
		return res;

	if (!metadata->hash_seg_size)
		return TEE_ERROR_ITEM_NOT_FOUND;

	metadata->elf_class = elf_class;
	metadata->elf_hdr_size = elf_hdr_size;
	metadata->phdr_table_size = phdr_table_size;

	return TEE_SUCCESS;
}
