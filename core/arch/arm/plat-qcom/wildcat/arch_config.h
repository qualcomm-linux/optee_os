/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef ARCH_CONFIG_H
#define ARCH_CONFIG_H

#define GICD_BASE			UL(0x17000000)
#define GICR_BASE			UL(0x17080000)

#define TCSR_MUTEX_BASE			UL(0x01F40000)
#define TCSR_MUTEX_SIZE			UL(0x40000)

#endif /* ARCH_CONFIG_H */
