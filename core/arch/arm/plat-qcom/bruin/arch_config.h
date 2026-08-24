/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef ARCH_CONFIG_H
#define ARCH_CONFIG_H

#define GICD_BASE			UL(0x0f200000)
#define GICR_BASE			UL(0x0f260000)

#define IMEM_DIAG_OFFSET		UL(0x720)
#define DIAG_SIZE			UL(0x3000)
#define DIAG_BASE			(IMEM_BASE + IMEM_SIZE - DIAG_SIZE)
#define DIAG_LOG_START_INFO		(IMEM_BASE + IMEM_DIAG_OFFSET)

#endif /* ARCH_CONFIG_H */
