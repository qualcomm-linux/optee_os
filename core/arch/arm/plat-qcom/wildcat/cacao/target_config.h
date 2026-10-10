/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define TCSR_BOOT_MISC_DETECT		UL(0x1FD3000)

#define IMEM_BASE			UL(0x14680000)
#define IMEM_SIZE			UL(0x19000)

#define GICD_BASE			UL(0x17200000)
#define GICR_BASE			UL(0x17260000)


#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x10000000)

#define DRAM1_BASE			ULL(0x880000000)
#define DRAM1_SIZE			ULL(0x780000000)

#define GENI_UART_REG_BASE		UL(0xa94000)

#define GCC_BASE                 UL(0x100000)
#define GCC_SIZE                 UL(0x1f0000)

#define AOP_CMD_DB_BASE          UL(0x87148000)
#define AOP_CMD_DB_SIZE          UL(0x2000)

#define AOP_MSG_RAM_BASE         UL(0x0C300000)
#define AOP_MSG_RAM_SIZE         UL(0x00100000)
#define MSG_RAM_SECTION_SIZE     UL(0x00001000)

#define RPMH_BASE_ADDR           UL(0x18900000)
#define RPMH_RSC_SIZE            UL(0x10000)

#define AOSS_BASE                UL(0x0b000000)
#define AOSS_SIZE                UL(0x04000000)

#define TCSR_BASE                UL(0x01f00000)
#define TCSR_SIZE                UL(0x00100000)

#define LPASS_BASE               UL(0x33000000)
#define LPASS_SIZE               UL(0x03c00000)

#endif /* TARGET_CONFIG_H */
