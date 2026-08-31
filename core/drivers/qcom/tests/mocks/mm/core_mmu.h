/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/mm/core_mmu.h.
 *
 * The real header pulls in the MMU/pager subsystem (SCATTERED_ARRAY-based
 * phys_mem_map registration, arch-specific mmu headers, platform_config.h,
 * etc.) which only exists in a full OP-TEE core build. The RPMh/CmdDb
 * drivers only need enum teecore_memtypes and the register_phys_mem*()
 * macros to compile; here those macros just discard their arguments so the
 * file-scope registration calls remain valid C at global scope.
 *
 * rpmh_client.c calls calloc()/free() without including <malloc.h> itself,
 * relying on this header's real counterpart transitively pulling it in via
 * mm/tee_mm.h. Keep that same transitive availability via the host libc's
 * <stdlib.h>.
 */
#ifndef __MM_CORE_MMU_H
#define __MM_CORE_MMU_H

#include <stdlib.h>

#define CORE_MMU_PGDIR_SIZE	0x200000

enum teecore_memtypes {
	MEM_AREA_END = 0,
	MEM_AREA_TEE_RAM,
	MEM_AREA_TEE_RAM_RX,
	MEM_AREA_TEE_RAM_RO,
	MEM_AREA_TEE_RAM_RW,
	MEM_AREA_INIT_RAM_RO,
	MEM_AREA_INIT_RAM_RX,
	MEM_AREA_NEX_RAM_RO,
	MEM_AREA_NEX_RAM_RW,
	MEM_AREA_NEX_DYN_VASPACE,
	MEM_AREA_TEE_DYN_VASPACE,
	MEM_AREA_TEE_COHERENT,
	MEM_AREA_TEE_ASAN,
	MEM_AREA_IDENTITY_MAP_RX,
	MEM_AREA_NSEC_SHM,
	MEM_AREA_NEX_NSEC_SHM,
	MEM_AREA_RAM_NSEC,
	MEM_AREA_RAM_SEC,
	MEM_AREA_ROM_SEC,
	MEM_AREA_IO_NSEC,
	MEM_AREA_IO_SEC,
	MEM_AREA_EXT_DT,
	MEM_AREA_MANIFEST_DT,
	MEM_AREA_RES_VASPACE,
	MEM_AREA_SHM_VASPACE,
	MEM_AREA_TS_VASPACE,
	MEM_AREA_PAGER_VASPACE,
	MEM_AREA_SDP_MEM,
	MEM_AREA_MAXTYPE,
};

/* Discard file-scope registration calls; the mock register file is set up
 * directly by each test via mock_regs_add_map() instead.
 */
#define __mock_cmu_paste2(a, b)	a ## b
#define __mock_cmu_paste(a, b)	__mock_cmu_paste2(a, b)
#define __mock_cmu_unique	__mock_cmu_paste(__mock_reg_phys_mem_, __LINE__)

#define register_phys_mem(type, addr, size) \
	extern int __mock_cmu_unique
#define register_phys_mem_ul(type, addr, size) \
	extern int __mock_cmu_unique
#define register_phys_mem_pgdir(type, addr, size) \
	extern int __mock_cmu_unique

#endif /* __MM_CORE_MMU_H */
