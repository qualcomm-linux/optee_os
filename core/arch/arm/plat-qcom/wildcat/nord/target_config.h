/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define GENI_UART_REG_BASE		UL(0x884000)

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

/*
 * AOSS_CC reset control, which owns the SOCCP subsystem-restart registers, and
 * the SOCCP CSR window holding its boot-suppress register.
 */
#define AOSS_CC_BASE			UL(0x0c2f0000)
#define AOSS_CC_SIZE			UL(0x00010000)

#define SOCCP_CSR_BASE			UL(0x00d40000)
#define SOCCP_CSR_SIZE			UL(0x0007f000)

/*
 * The cmd_db blob is placed in DDR at boot; its base is published as a word in
 * AOP message RAM rather than being fixed, so the driver reads the pointer and
 * maps the blob at that address.
 */
#define AOP_CMD_DB_PTR_ADDR		(AOP_MSG_RAM_BASE + \
					 AOP_MSG_RAM_SIZE - UL(0xF1000) + \
					 UL(0xC))
#define AOP_CMD_DB_SIZE			UL(0x00020000)

#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x380000000)
#define DRAM1_BASE			ULL(0x800000000)
#define DRAM1_SIZE			ULL(0x800000000)

#endif /* TARGET_CONFIG_H */
