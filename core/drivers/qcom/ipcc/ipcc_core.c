/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <initcall.h>
#include <inttypes.h>
#include <io.h>
#include <kernel/mutex.h>
#include <kernel/spinlock.h>
#include <malloc.h>
#include <mm/core_memprot.h>
#include <trace.h>

#include "ipcc_internal.h"
#include "ipcc_regs.h"

static struct ipcc_drv_ctxt drv_ctxt;

const struct ipcc_drv_ctxt *ipcc_get_drv_ctxt(void)
{
	return &drv_ctxt;
}

static const struct ipcc_client_cfg *
ipcc_get_client(const struct ipcc_proto_cfg *proto, enum ipcc_client cid)
{
	uint32_t i;

	for (i = 0U; i < proto->num_clients; i++) {
		if (proto->clients[i].client == cid) {
			return &proto->clients[i];
		}
	}

	return NULL;
}

static uint32_t ipcc_phys_idx(const struct ipcc_proto_cfg *proto,
			      const struct ipcc_client_cfg *entry,
			      enum ipcc_client cid)
{
	return (drv_ctxt.cfg->flags & IPCC_CFG_HW_MEM_OPT) ?
	       (uint32_t)(entry - proto->clients) : (uint32_t)cid;
}

static TEE_Result ipcc_resolve_phys_idx(const struct ipcc_proto_cfg *proto,
					 enum ipcc_client cid, uint32_t *idx)
{
	const struct ipcc_client_cfg *entry = ipcc_get_client(proto, cid);

	if (!entry) {
		DMSG("IPCC: invalid client %u for protocol %u\n", (uint32_t)cid,
			proto->protocol_id);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	*idx = ipcc_phys_idx(proto, entry, cid);

	return TEE_SUCCESS;
}

static bool ipcc_has_router_window(const struct ipcc_proto_cfg *proto)
{
	return proto->has_router;
}

static TEE_Result ipcc_protocol_init(const struct ipcc_proto_cfg *proto)
{
	struct ipcc_rx_drv_info *rx_drv = &drv_ctxt.rx_drv[proto->protocol_id];
	uint32_t idx = 0U;
	uintptr_t pa;
	vaddr_t va;
	TEE_Result res;

	mutex_init(&rx_drv->lock);

	if (!ipcc_has_router_window(proto)) {
		return TEE_SUCCESS;
	}

	res = ipcc_resolve_phys_idx(proto, drv_ctxt.cfg->client, &idx);
	if (res) {
		EMSG("IPCC: protocol %u: cannot resolve own client %u (0x%x)\n",
		      proto->protocol_id,
		      (uint32_t)drv_ctxt.cfg->client, res);
		return res;
	}

	pa = proto->proto_block_base +
	     ((uintptr_t)idx * proto->client_stride) +
	     ((uintptr_t)proto->protocol_id * proto->proto_stride);

	va = (vaddr_t)phys_to_virt(pa, MEM_AREA_IO_SEC, proto->client_stride);
	if (!va) {
		EMSG("IPCC: protocol %u: client page not mapped\n",
		      proto->protocol_id);
		return TEE_ERROR_BAD_STATE;
	}

	drv_ctxt.router_pages[proto->protocol_id] = va;

	drv_ctxt.hw_version =
		io_read32(va + IPCC_VERSION_OFF) & IPCC_VERSION_MASK;

	DMSG("IPCC: protocol %u initialized, hw_version 0x%x\n",
		proto->protocol_id, drv_ctxt.hw_version);

	return TEE_SUCCESS;
}

static void ipcc_apply_block_cfg(const struct ipcc_cfg *cfg)
{
	if (!cfg->has_ctrl_block) {
		EMSG("IPCC: no ctrl_block, block config not applied\n");
		return;
	}

	io_write32(cfg->ctrl_block_base + IPCC_TOP_MODE_BLOCK_OFF,
		  (cfg->flags & IPCC_CFG_ROUTER_MODE) ? IPCC_TOP_MODE_BIT : 0U);

	io_write32(cfg->ctrl_block_base + IPCC_TRACE_BLOCK_OFF,
		  IPCC_TRACE_ENABLE_BIT);
}

static void ipcc_init_protocols(const struct ipcc_cfg *cfg)
{
	uint32_t unavailable = 0U;
	uint32_t i;

	for (i = 0U; i < cfg->num_protocols; i++) {
		if (ipcc_protocol_init(&cfg->protocols[i])) {
			unavailable++;
		}
	}

	if (unavailable != 0U) {
		EMSG("IPCC: %u/%u protocols unavailable\n",
		      unavailable, cfg->num_protocols);
		return;
	}

	DMSG("IPCC: initialized, %u protocols\n", cfg->num_protocols);
}

static TEE_Result qti_ipcc_init(void)
{
	const struct ipcc_cfg *cfg = ipcc_chipset_cfg;

	if (cfg == NULL) {
		EMSG("IPCC: no chipset config, driver unavailable\n");
		return TEE_ERROR_NOT_SUPPORTED;
	}

	/* base[] holds one slot per protocol; a longer table cannot fit. */
	if (cfg->num_protocols > IPCC_PROTO_TOTAL) {
		EMSG("IPCC: %u protocols described, %u supported\n",
		      cfg->num_protocols, (uint32_t)IPCC_PROTO_TOTAL);
		return TEE_ERROR_BAD_PARAMETERS;
	}
	drv_ctxt.cfg = cfg;

	if (cfg->flags & IPCC_CFG_NO_TME_CFG) {
		ipcc_apply_block_cfg(cfg);
	}

	ipcc_init_protocols(cfg);

	return TEE_SUCCESS;
}

static bool ipcc_proto_valid(enum ipcc_protocol protocol)
{
	return (protocol < IPCC_PROTO_TOTAL) &&
	       ((uint32_t)protocol < drv_ctxt.cfg->num_protocols);
}

static const struct ipcc_backend_ops *
ipcc_get_backend_ops(const struct ipcc_proto_cfg *proto,
		     enum ipcc_backend backend)
{
	if (!ipcc_backend_supported(proto->backends, backend)) {
		return NULL;
	}

	return proto->backend_ops[backend];
}

TEE_Result ipcc_attach(struct ipcc_handle **h, enum ipcc_protocol protocol)
{
	const struct ipcc_proto_cfg *proto;
	const struct ipcc_backend_ops *ops;
	struct ipcc_handle *handle;
	TEE_Result res;

	if (drv_ctxt.cfg == NULL) {
		return TEE_ERROR_BAD_STATE;
	}

	if (!h || (uint32_t)protocol >= IPCC_PROTO_TOTAL) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (!ipcc_proto_valid(protocol)) {
		DMSG("IPCC: protocol %d not supported", protocol);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	proto = &drv_ctxt.cfg->protocols[protocol];

	handle = calloc(1, sizeof(*handle));
	if (!handle) {
		return TEE_ERROR_OUT_OF_MEMORY;
	}

	handle->protocol = proto;
	handle->rx_drv = &drv_ctxt.rx_drv[protocol];

	ops = ipcc_get_backend_ops(proto, IPCC_BACKEND_ROUTER);
	if (!ops || !ops->attach) {
		free(handle);
		DMSG("IPCC: protocol %d has no attach operation", protocol);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	res = ops->attach(handle);
	if (res) {
		free(handle);
		return res;
	}

	handle->backend_ops = ops;
	*h = handle;

	return TEE_SUCCESS;
}

TEE_Result ipcc_detach(struct ipcc_handle **h)
{
	struct ipcc_handle *handle;

	if (drv_ctxt.cfg == NULL) {
		return TEE_ERROR_BAD_STATE;
	}

	if (!h || !*h) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	handle = *h;

	if (handle->backend_ops && handle->backend_ops->detach) {
		handle->backend_ops->detach(handle);
	}

	free(handle);
	*h = NULL;

	return TEE_SUCCESS;
}

TEE_Result ipcc_register_interrupt(struct ipcc_handle *h,
				   enum ipcc_client sender_id,
				   uint16_t signal_low, uint16_t signal_high,
				   ipcc_callback_t cb, void *cb_data)
{
	if (drv_ctxt.cfg == NULL) {
		return TEE_ERROR_BAD_STATE;
	}

	if (!h || !cb || signal_low > signal_high) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (!h->backend_ops || !h->backend_ops->register_interrupt) {
		return TEE_ERROR_NOT_SUPPORTED;
	}

	return h->backend_ops->register_interrupt(h, sender_id, signal_low,
						  signal_high, cb, cb_data);
}

TEE_Result ipcc_deregister_interrupt(struct ipcc_handle *h,
				     enum ipcc_client sender_id,
				     uint16_t signal_low,
				     uint16_t signal_high)
{
	if (drv_ctxt.cfg == NULL) {
		return TEE_ERROR_BAD_STATE;
	}

	if (!h || signal_low > signal_high) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (!h->backend_ops || !h->backend_ops->deregister_interrupt) {
		return TEE_ERROR_NOT_SUPPORTED;
	}

	return h->backend_ops->deregister_interrupt(h, sender_id, signal_low,
						    signal_high);
}

TEE_Result ipcc_trigger(struct ipcc_handle *h, enum ipcc_client target_id,
			uint16_t signal_low, uint16_t signal_high)
{
	if (drv_ctxt.cfg == NULL) {
		return TEE_ERROR_BAD_STATE;
	}

	if (!h || signal_low > signal_high) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (!h->backend_ops || !h->backend_ops->signal) {
		return TEE_ERROR_NOT_SUPPORTED;
	}

	return h->backend_ops->signal(h, target_id, signal_low, signal_high);
}

early_init(qti_ipcc_init);
