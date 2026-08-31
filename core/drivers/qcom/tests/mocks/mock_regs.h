/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Fake MMIO register file for Tier 1 host unit tests. Backs io.h's
 * io_read32()/io_write32() and mm/core_memprot.h's phys_to_virt(). Lets a
 * test allocate a fake device region at a chosen physical base, obtain its
 * host-heap virtual address, and read/write/assert on individual offsets
 * exactly as the real hardware register map would be accessed.
 */
#ifndef __MOCK_REGS_H
#define __MOCK_REGS_H

#include <stdint.h>
#include <types_ext.h>

/* Reset all fake regions; call at the start of every test. */
void mock_regs_reset(void);

/*
 * Register a fake device region covering [pa, pa + size). Returns the fake
 * virtual base address phys_to_virt() should hand back for this region, or
 * 0 on failure (region table full / size 0).
 */
vaddr_t mock_regs_add_region(paddr_t pa, size_t size);

/* Backing implementation for the mocked phys_to_virt()/virt_to_phys(). */
void *mock_regs_phys_to_virt(paddr_t pa, size_t len);
paddr_t mock_regs_virt_to_phys(void *va);

/* Backing implementation for the mocked io_read32()/io_write32(). */
uint32_t mock_regs_read32(vaddr_t addr);
void mock_regs_write32(vaddr_t addr, uint32_t val);

/*
 * Queue a value to be returned by the next mock_regs_read32() call at
 * @addr, without going through mock_regs_write32(). Used to script
 * hardware-driven register changes (e.g. AOP boot cookie appearing,
 * IRQ status bits set) independently of what the driver itself wrote.
 */
void mock_regs_inject_read32(vaddr_t addr, uint32_t val);

#endif /* __MOCK_REGS_H */
