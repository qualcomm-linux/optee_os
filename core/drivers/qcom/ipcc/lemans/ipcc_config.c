/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qcom/ipcc/ipcc.h>
#include <mm/core_mmu.h>
#include <util.h>

#include "ipcc_internal.h"
#include <target_config.h>

/*
 * The window covers every client page as well as the IPC_CONFIG and IPC_TRACE
 * blocks, which no TME owns here. Registering it is the chipset's job: the
 * driver only resolves what these tables point at.
 */
register_phys_mem(MEM_AREA_IO_SEC, IPCC_BASE, IPCC_SIZE);

/*
 * lemans instantiates the MPROC protocol only, in router mode, and reaches
 * every client through the controller.
 *
 * These are the clients this TEE may exchange signals with; anything absent is
 * rejected. The client pages sit at their client ID positions across the 0..36
 * span and only the rows below are real, so hw_mem_opt stays false, a page is
 * addressed by client ID, and the row order carries no meaning. APSS_NS4 (36)
 * is inside the span but not permitted on this protocol.
 */
static const struct ipcc_client_cfg mproc_clients[] = {
	{ .client = IPCC_CLIENT_AOP },
	{ .client = IPCC_CLIENT_TZ },
	{ .client = IPCC_CLIENT_LPASS },
	{ .client = IPCC_CLIENT_NSP0 },
	{ .client = IPCC_CLIENT_APPS },
	{ .client = IPCC_CLIENT_GPU },
	{ .client = IPCC_CLIENT_CVP },
	{ .client = IPCC_CLIENT_CAM },
	{ .client = IPCC_CLIENT_VPU },
	{ .client = IPCC_CLIENT_NSP1 },
	{ .client = IPCC_CLIENT_SAIL0 },
	{ .client = IPCC_CLIENT_SAIL1 },
	{ .client = IPCC_CLIENT_SAIL2 },
	{ .client = IPCC_CLIENT_SAIL3 },
	{ .client = IPCC_CLIENT_GPDSP0 },
	{ .client = IPCC_CLIENT_GPDSP1 },
	{ .client = IPCC_CLIENT_APSS_NS1 },
	{ .client = IPCC_CLIENT_APSS_NS2 },
	{ .client = IPCC_CLIENT_APSS_NS3 },
};

/* Only MPROC is described; protocols 1 to 4 exist but EL3 does not use them. */
static const struct ipcc_proto_cfg protocols[] = {
	[IPCC_PROTO_MPROC] = {
		.protocol_id = IPCC_PROTO_MPROC,
		.num_sigs = IPCC_MPROC_NUM_SIGS,
		.num_clients = ARRAY_SIZE(mproc_clients),
		.proto_block_base = IPCC_BASE,
		.has_router = true,
		.clients = mproc_clients,
		.backends = IPCC_CAP_ROUTER,
		.proto_stride = IPCC_PROTO_STRIDE,
		.client_stride = IPCC_CLIENT_STRIDE,
		.interrupt = IPCC_MPROC_IRQ,
		.backend_ops = {
			[IPCC_BACKEND_ROUTER] = &ipcc_router_ops,
		},
	},
};

static const struct ipcc_cfg ipcc_cfg = {
	.protocols = protocols,
	.num_protocols = ARRAY_SIZE(protocols),
	.client = IPCC_CLIENT_TZ,
	.ctrl_block_base = IPCC_BASE,
	.has_ctrl_block = true,
	.flags = IPCC_CFG_ROUTER_MODE,
};

const struct ipcc_cfg *const ipcc_chipset_cfg = &ipcc_cfg;

/* lemans reaches no client over a direct line */
const struct ipcc_direct_cfg *const ipcc_chipset_direct_cfg;
