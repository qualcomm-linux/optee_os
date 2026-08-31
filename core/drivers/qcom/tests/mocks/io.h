/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/io.h.
 *
 * The real io.h is almost pure static-inline C and would be host-portable
 * as-is, except it includes kernel/delay_arch.h (arch-specific, pulls in
 * arm.h) purely for the IO_READ32_POLL_TIMEOUT() helper, which none of the
 * RPMh/CmdDb drivers use. Only io_read32()/io_write32() are exercised by
 * the drivers under test, so this mock keeps just those, backed by the
 * fake register file in mock_regs.h so tests can assert on what a driver
 * wrote and inject what it reads.
 */
#ifndef __IO_H
#define __IO_H

#include <stdint.h>
#include <types_ext.h>

#include "mock_regs.h"

static inline uint32_t io_read32(vaddr_t addr)
{
	return mock_regs_read32(addr);
}

static inline void io_write32(vaddr_t addr, uint32_t val)
{
	mock_regs_write32(addr, val);
}

#endif /* __IO_H */
