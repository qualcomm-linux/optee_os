/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x80000000)

#define GENI_UART_REG_BASE		UL(0x04a80000)

#define IMEM_BASE			UL(0x0c100000)
#define IMEM_SIZE			UL(0x00020000)

#define TURING_BASE			UL(0xb000000)
#define TURING_SIZE			UL(0x00300300)

#define CLK_CTL_BASE			UL(0x1400000)
#define CLK_CTL_SIZE			UL(0x000c4000)
#define GCC_BASE			CLK_CTL_BASE
#define GCC_SIZE			CLK_CTL_SIZE

#define CORE_TOP_CSR_BASE		UL(0x300000)
#define CORE_TOP_CSR_SIZE		UL(0x00075000)

#define MCU_MCU_CNOC_MASTER_BASE	UL(0xb800000)
#define MCU_MCU_CNOC_MASTER_SIZE	UL(0x0008d000)

#endif /* TARGET_CONFIG_H */
