/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef PIL_METADATA_H
#define PIL_METADATA_H

#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>

struct pil_elf_metadata {
	uint8_t elf_class;
	size_t elf_hdr_size;
	size_t phdr_table_size;
	size_t hash_seg_size;
};

/*
 * Validate a PIL image's ELF header and program-header table, and locate
 * its hash-segment program header.
 *
 * @elf_hdr:         ELF header buffer.
 * @elf_hdr_size:    Size of @elf_hdr in bytes.
 * @phdr_table:      Program-header table buffer, immediately following the
 *                   ELF header in the image.
 * @phdr_table_size: Size of @phdr_table in bytes.
 * @metadata:        Populated on success with the ELF class and the
 *                    hash-segment size found in the program-header table.
 *
 * Returns TEE_SUCCESS, or an error if the ELF header is malformed, the
 * program-header table size does not match e_phnum, or no hash-segment
 * program header is present.
 */
TEE_Result pil_validate_elf_metadata(const void *elf_hdr, size_t elf_hdr_size,
				     const void *phdr_table,
				     size_t phdr_table_size,
				     struct pil_elf_metadata *metadata);

#endif /* PIL_METADATA_H */
