// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE HWIO compatibility layer.
 *
 * Provides HWIO-style register access macros and helpers used
 * by the Crypto Engine and BAM drivers. This layer bridges the
 * IPCAT-generated register definitions to the OP-TEE MMIO access
 * framework.
 */

#ifndef QCE_HWIO_H
#define QCE_HWIO_H

#include <io.h>
#include "qce_hwio_regs.h"

/*
 * Physical->Virtual translation supplied by uclib_env.c
 */
vaddr_t ce_hwio_phys_to_virt(uintptr_t phys_addr);

/*-----------------------------------------------------------------------
 * Basic MMIO helpers
 *---------------------------------------------------------------------*/
#define in_dword(addr) \
        io_read32(ce_hwio_phys_to_virt((uintptr_t)(addr)))

#define in_dword_masked(addr, mask) \
        (in_dword(addr) & (uint32_t)(mask))

#define out_dword(addr, val) \
        io_write32(ce_hwio_phys_to_virt((uintptr_t)(addr)), \
                   (uint32_t)(val))

#define out_dword_masked_ns(addr, mask, val, shadow) \
        out_dword((addr), \
                  (((uint32_t)(shadow) & ~(uint32_t)(mask)) | \
                   ((uint32_t)(val) & (uint32_t)(mask))))

/*-----------------------------------------------------------------------
 * Generic HWIO accessors
 *
 * IMPORTANT:
 * These intentionally rely directly on IPCAT symbols.
 *
 * If a register does not exist for the target,
 * compilation must fail.
 *---------------------------------------------------------------------*/

#define HWIO_ADDRX(base, hwiosym) \
        HWIO_##hwiosym##_ADDR

#define HWIO_ADDRXI(base, hwiosym, index) \
        HWIO_##hwiosym##_ADDR(index)

#define HWIO_ADDRXI2(base, hwiosym, index1, index2) \
        HWIO_##hwiosym##_ADDR(index1, index2)


#define HWIO_INX(base, hwiosym) \
        in_dword_masked(HWIO_##hwiosym##_ADDR, \
                        HWIO_##hwiosym##_RMSK)

#define HWIO_INXI(base, hwiosym, index) \
        in_dword_masked(HWIO_##hwiosym##_ADDR(index), \
                        HWIO_##hwiosym##_RMSK)

#define HWIO_INXI2(base, hwiosym, index1, index2) \
        in_dword_masked(HWIO_##hwiosym##_ADDR(index1,index2), \
                        HWIO_##hwiosym##_RMSK)

#define HWIO_OUTX(base, hwiosym, val) \
        out_dword(HWIO_##hwiosym##_ADDR, val)

#define HWIO_OUTXI(base, hwiosym, index, val) \
        out_dword(HWIO_##hwiosym##_ADDR(index), val)

#define HWIO_OUTXI2(base, hwiosym, index1, index2, val) \
        out_dword(HWIO_##hwiosym##_ADDR(index1,index2), val)

#define HWIO_SHFT(hwiosym, field) \
        HWIO_##hwiosym##_##field##_SHFT

#define HWIO_FMSK(hwiosym, field) \
        HWIO_##hwiosym##_##field##_BMSK

#define HWIO_INXF(base, hwiosym, field) \
	((HWIO_INX((base), hwiosym) & \
	  HWIO_##hwiosym##_##field##_BMSK) >> \
	 HWIO_##hwiosym##_##field##_SHFT)

/*-----------------------------------------------------------------------
 * CE wrappers expected by CE_HAL / CE_BAM
 *---------------------------------------------------------------------*/

#define CE_HWIO_ADDR(hwiosym) \
        HWIO_ADDRX(0, hwiosym)

#define CE_HWIO_ADDRI(hwiosym, index) \
        HWIO_ADDRXI(0, hwiosym, index)

#define CE_HWIO_ADDRI2(hwiosym, i1, i2) \
        HWIO_ADDRXI2(0, hwiosym, i1, i2)

#define CE_HWIO_IN(hwiosym) \
        HWIO_INX(0, hwiosym)

#define CE_HWIO_INI(hwiosym, index) \
        HWIO_INXI(0, hwiosym, index)

#define CE_HWIO_INI2(hwiosym, i1, i2) \
        HWIO_INXI2(0, hwiosym, i1, i2)

#define CE_HWIO_OUT(hwiosym, value) \
        HWIO_OUTX(0, hwiosym, value)

#define CE_HWIO_OUTI(hwiosym, index, value) \
        HWIO_OUTXI(0, hwiosym, index, value)

#define CE_HWIO_OUTI2(hwiosym, i1, i2, value) \
        HWIO_OUTXI2(0, hwiosym, i1, i2, value)

#define CE_HWIO_INF(hwiosym, field)     \
        HWIO_INXF(0, hwiosym, field)

#define CE_HWIO_SHFT \
        HWIO_SHFT

#define CE_HWIO_FMSK \
        HWIO_FMSK

#endif