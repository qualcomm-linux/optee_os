/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define GENI_UART_REG_BASE		UL(0x884000)

/*
 * TODO: pending AoP confirmation for Nord IQ10 (see to_ask.md #1.1, #2.1,
 * #2.6). Values below are copied from hoya/lemans and are NOT verified
 * against Nord's memory map - do not trust for real hardware until
 * cross-checked with the AoP team. RPMh/CmdDb are not currently compiled
 * for wildcat/nord - CFG_QCOM_RPMH_CLIENT and CFG_QCOM_CMD_DB are unset.
 */
#define AOP_CMD_DB_BASE			UL(0x90860000)
#define AOP_CMD_DB_SIZE			UL(0x00020000)

#define AOP_MSG_RAM_BASE		UL(0x0C300000)
#define AOP_MSG_RAM_SIZE		UL(0x00100000)

#define RPMH_BASE_ADDR			UL(0x18200000)
#define RPMH_RSC_SIZE			UL(0x40000)


#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x380000000)
#define DRAM1_BASE			ULL(0x800000000)
#define DRAM1_SIZE			ULL(0x800000000)

#endif /* TARGET_CONFIG_H */
