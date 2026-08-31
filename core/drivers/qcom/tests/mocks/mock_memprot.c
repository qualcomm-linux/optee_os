/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Backing implementation for the mocked mm/core_memprot.h. Kept separate
 * from mock_regs.c so the phys_to_virt()/virt_to_phys() symbol names match
 * exactly what the driver code expects, while the region bookkeeping lives
 * in one place.
 */
#include "mm/core_memprot.h"
#include "mock_regs.h"

void *phys_to_virt(paddr_t pa, enum teecore_memtypes m, size_t len)
{
	(void)m;

	return mock_regs_phys_to_virt(pa, len);
}

paddr_t virt_to_phys(void *va)
{
	return mock_regs_virt_to_phys(va);
}
