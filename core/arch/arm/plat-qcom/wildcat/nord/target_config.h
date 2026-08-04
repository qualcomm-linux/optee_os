/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define GENI_UART_REG_BASE		UL(0x884000)

#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x380000000)
#define DRAM1_BASE			ULL(0x800000000)
#define DRAM1_SIZE			ULL(0x800000000)

#define GCC_BASE			UL(0x04100000)
#define GCC_SIZE			UL(0x00200000)

/*
 * QDSP6 (HPASS) subsystem. Base/size cover only the QDSP6SS_PUB register
 * block (IPCatalog HPASS_0_QDSP6SS_QDSP6SS_PUB); the clock-sequencing and
 * AON/TOP/CORE_CC sub-blocks kodiak/lemans map alongside it have not been
 * located on this chip, so fw_start()/fw_shutdown() are NOT_IMPLEMENTED
 * stubs (see platform/nord/subsys.c) rather than reused blind.
 */
#define HPASS_QDSP6_BASE		UL(0x08c00000)
#define HPASS_QDSP6_SIZE		UL(0x00100000)

/* IRIS video-codec subsystem. */
#define IRIS_BASE			UL(0x0ea00000)
#define IRIS_SIZE			UL(0x00100000)

#endif /* TARGET_CONFIG_H */
