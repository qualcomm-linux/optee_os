/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef ARCH_CONFIG_H
#define ARCH_CONFIG_H

#define GICD_BASE			UL(0x17000000)
#define GICR_BASE			UL(0x17080000)

/*
 * AOP message RAM. The RPMh client derives its resource-dictionary address
 * from the top of this window; identical placement to hoya.
 */
#define AOP_MSG_RAM_BASE		UL(0x0C300000)
#define AOP_MSG_RAM_SIZE		UL(0x00100000)

/* APSS RSC (RPMh) window; OP-TEE drives the TZ DRV only. */
#define RPMH_BASE_ADDR			UL(0x18900000)
#define RPMH_RSC_SIZE			UL(0x40000)

#endif /* ARCH_CONFIG_H */
