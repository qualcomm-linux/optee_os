/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <mm/core_mmu.h>
#include <util.h>

#include "ipcc_internal.h"
#include <target_config.h>

/*
 * This target wires direct mode only: AOP is reached through its own
 * trigger register, and there is no router window to map.
 */
register_phys_mem(MEM_AREA_IO_SEC, IPCC_DIRECT_BASE, IPCC_DIRECT_SIZE);

/*
 * RPM_TZ_IPC[2:0] is the outbound field, so this target only wires the three
 * signals AOP can send, each with its own dedicated GIC line back to this TEE.
 */
static struct ipcc_direct_signal aop_signals[] = {
	[0] = {
		.interrupt_id = IPCC_DIRECT_AOP_SIG0_IRQ,
		.out_mask = IPCC_DIRECT_AOP_SIG0_MASK,
	},
	[1] = {
		.interrupt_id = IPCC_DIRECT_AOP_SIG1_IRQ,
		.out_mask = IPCC_DIRECT_AOP_SIG1_MASK,
	},
	[2] = {
		.interrupt_id = IPCC_DIRECT_AOP_SIG2_IRQ,
		.out_mask = IPCC_DIRECT_AOP_SIG2_MASK,
	},
};

static struct ipcc_direct_client direct_clients[] = {
	[IPCC_CLIENT_AOP] = {
		.num_signals = ARRAY_SIZE(aop_signals),
		.signals = aop_signals,
		.trigger_reg = IPCC_DIRECT_AOP_TRIG_REG,
	},
};

static const struct ipcc_direct_cfg direct_cfg = {
	.num_clients = ARRAY_SIZE(direct_clients),
	.clients = direct_clients,
};

const struct ipcc_direct_cfg *const ipcc_chipset_direct_cfg = &direct_cfg;

/* AOP is the only client this TEE reaches on MPROC */
static const struct ipcc_client_cfg mproc_clients[] = {
	{ .client = IPCC_CLIENT_AOP },
};

/* Only MPROC is described; this target has no router to wire */
static const struct ipcc_proto_cfg protocols[] = {
	[IPCC_PROTO_MPROC] = {
		.protocol_id = IPCC_PROTO_MPROC,
		.num_sigs = IPCC_MPROC_NUM_SIGS,
		.num_clients = ARRAY_SIZE(mproc_clients),
		.clients = mproc_clients,
		.backends = IPCC_CAP_DIRECT,
		.backend_ops = {
			[IPCC_BACKEND_DIRECT] = &ipcc_direct_ops,
		},
		.has_router = false,
	},
};

static const struct ipcc_cfg ipcc_cfg = {
	.protocols = protocols,
	.num_protocols = ARRAY_SIZE(protocols),
	.client = IPCC_CLIENT_TZ,
	.has_ctrl_block = false,
};

const struct ipcc_cfg *const ipcc_chipset_cfg = &ipcc_cfg;
