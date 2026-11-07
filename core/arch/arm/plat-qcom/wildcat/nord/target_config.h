/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

/*
 * Clock controllers. Nord splits its QUPv3 serial engines across the SE and NE
 * quadrant controllers, each with its own GPLL0 and vote registers, alongside
 * the central GCC that owns QUPv3 wrapper 3. The NW quadrant controller has no
 * QUP serial engines, so it is not mapped.
 */
#define GCC_BASE			UL(0x00110000)
#define GCC_SIZE			UL(0x001e0000)

#define SE_GCC_BASE			UL(0x08a10000)
#define SE_GCC_SIZE			UL(0x000e0000)

#define NE_GCC_BASE			UL(0x08910000)
#define NE_GCC_SIZE			UL(0x000e0000)

#define GENI_UART_REG_BASE		UL(0x884000)

#define GICD_BASE			UL(0x17000000)
#define GICR_BASE			UL(0x17080000)

#define AOP_CMD_DB_BASE                 UL(0x87148000)
#define AOP_CMD_DB_SIZE                 UL(0x2000)

#define MSG_RAM_SECTION_SIZE            UL(0x00001000)

#define RPMH_BASE_ADDR                  UL(0x18900000)
#define RPMH_RSC_SIZE                   UL(0x10000)

#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x80000000)
#define DRAM1_BASE			ULL(0x880000000)
#define DRAM1_SIZE			ULL(0x780000000)
#define DRAM2_BASE			ULL(0x8800000000)
#define DRAM2_SIZE			ULL(0x3800000000)

#endif /* TARGET_CONFIG_H */
