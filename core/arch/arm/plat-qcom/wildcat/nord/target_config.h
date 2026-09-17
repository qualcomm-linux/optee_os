/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define GENI_UART_REG_BASE		UL(0x884000)

#define GICD_BASE			UL(0x17000000)
#define GICR_BASE			UL(0x17080000)

#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x80000000)
#define DRAM1_BASE			ULL(0x880000000)
#define DRAM1_SIZE			ULL(0x780000000)
#define DRAM2_BASE			ULL(0x8800000000)
#define DRAM2_SIZE			ULL(0x3800000000)

/* IMEM and Diagnostic buffer */
#define TCSR_BOOT_MISC_DETECT		UL(0x1FD3000)
#define IMEM_BASE			UL(0x14680000)
#define IMEM_SIZE			UL(0x19000)

#endif /* TARGET_CONFIG_H */
