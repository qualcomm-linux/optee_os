/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/mm/core_memprot.h.
 *
 * Real phys_to_virt()/virt_to_phys() walk the live MMU translation tables.
 * The mock register file (mock_regs.h) keeps a small table of
 * (pa, type) -> fake VA mappings that tests populate directly, so driver
 * code that calls phys_to_virt() during init gets a valid pointer into a
 * host heap buffer instead of a real hardware mapping.
 */
#ifndef __MM_CORE_MEMPROT_H
#define __MM_CORE_MEMPROT_H

#include <mm/core_mmu.h>
#include <types_ext.h>
#include <stddef.h>

void *phys_to_virt(paddr_t pa, enum teecore_memtypes m, size_t len);
paddr_t virt_to_phys(void *va);

#endif /* __MM_CORE_MEMPROT_H */
