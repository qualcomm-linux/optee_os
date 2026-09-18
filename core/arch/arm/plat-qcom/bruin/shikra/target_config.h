/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define GCC_BASE			UL(0x01400000)
#define GCC_SIZE			UL(0x00200000)

#define SECURITY_CONTROL_BASE		UL(0x01b40000)
#define SECURITY_CONTROL_SIZE		UL(0x00010000)

#define TCSR_MUTEX_BASE			UL(0x00340000)
#define TCSR_MUTEX_SIZE			UL(0x00040000)

#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x80000000)

#define GENI_UART_REG_BASE		UL(0x04a80000)

#define IMEM_BASE			UL(0x0c100000)
#define IMEM_SIZE			UL(0x00020000)

#endif /* TARGET_CONFIG_H */
