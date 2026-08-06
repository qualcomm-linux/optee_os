/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef ARCH_CONFIG_H
#define ARCH_CONFIG_H

#define GICD_BASE			UL(0x17000000)
#define GICR_BASE			UL(0x17080000)

/*
 * TME fuse-controller "sense register" sub-block (FUSE_CONTROLLER_SW_RANGE4
 * in the reference register header), holding SECURE_BOOT, device-identity,
 * OEM_CONFIG* and MRC fuse sense registers read by
 * core/drivers/qcom/qfprom/nord/qfprom_target.h.
 */
#define SECURITY_CONTROL_BASE		UL(0x360d4000)
#define SECURITY_CONTROL_SIZE		UL(0x4000)

#endif /* ARCH_CONFIG_H */
