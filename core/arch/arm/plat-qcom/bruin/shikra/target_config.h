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

#define QCOM_RNG_REG_BASE			UL(0x04440000)
#define QCOM_RNG_REG_SIZE			UL(0x00020000)
#define QCOM_RNG_DATA_OUT			0x11000
#define QCOM_RNG_STATUS				0x11004

#endif /* TARGET_CONFIG_H */
