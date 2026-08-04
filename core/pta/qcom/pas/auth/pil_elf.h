/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * Minimal ELF32/ELF64 field definitions for PIL image metadata validation.
 * Scoped to exactly the fields the hash-segment discovery in
 * pil_elf_metadata.c needs, not a general-purpose ELF parser.
 */

#ifndef PIL_ELF_H
#define PIL_ELF_H

#include <stdint.h>

#define ELF_EI_MAG0	0
#define ELF_EI_MAG1	1
#define ELF_EI_MAG2	2
#define ELF_EI_MAG3	3
#define ELF_EI_CLASS	4

#define ELF_MAG0	0x7f
#define ELF_MAG1	'E'
#define ELF_MAG2	'L'
#define ELF_MAG3	'F'

#define ELF_CLASS_32	1
#define ELF_CLASS_64	2

/*
 * PIL segment-type encoding packed into Elf{32,64}_Phdr.p_flags. Segment
 * type occupies bits [26:24]; a hash-segment program header carries type
 * 0x2 in that field.
 */
#define PIL_SEGMENT_TYPE_MASK		0x7000000
#define PIL_SEGMENT_TYPE_SHIFT		24
#define PIL_SEGMENT_TYPE_HASH		0x2
#define PIL_SEGMENT_TYPE(flags) \
	(((flags) & PIL_SEGMENT_TYPE_MASK) >> PIL_SEGMENT_TYPE_SHIFT)

struct elf32_hdr {
	uint8_t e_ident[16];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

struct elf32_phdr {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};

struct elf64_hdr {
	uint8_t e_ident[16];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

struct elf64_phdr {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
};

#endif /* PIL_ELF_H */
