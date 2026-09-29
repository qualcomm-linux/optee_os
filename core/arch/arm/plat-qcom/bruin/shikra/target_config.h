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

/*
 * IPCC: this target wires direct mode only, so there is no router window or
 * control block, just AOP's own trigger register (APSS_SHARED_TZ_IPC).
 * ipcc_core.c compiles ipcc_apply_block_cfg() unconditionally regardless, so
 * these two still need a value even though this target never calls it.
 */
#define IPCC_TOP_MODE_BLOCK_OFF		UL(0x0)
#define IPCC_TRACE_BLOCK_OFF		UL(0x0)

#define IPCC_DIRECT_BASE		UL(0x0f400000)
#define IPCC_DIRECT_SIZE		UL(0x00001000)
#define IPCC_DIRECT_AOP_TRIG_REG	(IPCC_DIRECT_BASE + UL(0x8))

/* RPM_TZ_IPC[2:0]; three outbound bits is the hardware limit on AOP */
#define IPCC_DIRECT_AOP_SIG0_MASK	BIT32(0)
#define IPCC_DIRECT_AOP_SIG1_MASK	BIT32(1)
#define IPCC_DIRECT_AOP_SIG2_MASK	BIT32(2)

/* AOP's return path to this TEE: one dedicated GIC line per signal */
#define IPCC_DIRECT_AOP_SIG0_IRQ	198
#define IPCC_DIRECT_AOP_SIG1_IRQ	199
#define IPCC_DIRECT_AOP_SIG2_IRQ	200

/* Signals per client on the MPROC protocol */
#define IPCC_MPROC_NUM_SIGS		3

#endif /* TARGET_CONFIG_H */
