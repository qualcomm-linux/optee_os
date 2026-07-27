/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define PL011_UART_BASE			ULL(0x100C800000)
#define PL011_UART_SIZE			UL(0x1000)
#define CONSOLE_UART_BASE		PL011_UART_BASE

#define DRAM0_BASE			ULL(0x80000000)
#define DRAM0_SIZE			ULL(0x80000000)

#endif /* TARGET_CONFIG_H */
