/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef ARCH_CONFIG_H
#define ARCH_CONFIG_H

/*
 * GICD_BASE and GICR_BASE differ between Wildcat chips; define them in
 * each chip's target_config.h.
 */

#define RPMH_BASE_ADDR			UL(0x0b280000)
#define RPMH_RSC_SIZE			UL(0x01008000)

#define SECURITY_CONTROL_BASE		UL(0x22200000)
#define SECURITY_CONTROL_SIZE		UL(0x0000c000)

#endif /* ARCH_CONFIG_H */

