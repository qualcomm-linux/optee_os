/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <initcall.h>
#include <inttypes.h>
#include <io.h>
#include <kernel/mutex.h>
#include <malloc.h>
#include <mm/core_memprot.h>
#include <trace.h>
#include <util.h>

#include "ipcc_internal.h"
#include "ipcc_regs.h"

static struct ipcc_drv_ctxt drv_ctxt;

const struct ipcc_drv_ctxt *ipcc_get_drv_ctxt(void)
{
	return &drv_ctxt;
}

TEE_Result ipcc_client_page_idx(const struct ipcc_proto_cfg *proto,
				enum ipcc_client client, uint32_t *idx)
{
	uint32_t i = 0;

	for (i = 0; i < proto->num_clients; i++) {
		if (proto->clients[i].client != client)
			continue;

		/* Packed pages are addressed by row, all others by client ID */
		*idx = (drv_ctxt.cfg->flags & IPCC_CFG_HW_MEM_OPT) ?
		       i : (uint32_t)client;

		return TEE_SUCCESS;
	}

	DMSG("client %u not described by protocol %u", (uint32_t)client,
	     proto->protocol_id);

	return TEE_ERROR_NOT_SUPPORTED;
}

/*
 * A chipset may initialise its protocol array by protocol ID and leave holes,
 * so a slot counts as described only once it has a client table.
 */
static const struct ipcc_proto_cfg *ipcc_find_proto(enum ipcc_protocol protocol)
{
	const struct ipcc_proto_cfg *proto = NULL;
	uint32_t i = 0;

	for (i = 0; i < drv_ctxt.cfg->num_protocols; i++) {
		proto = &drv_ctxt.cfg->protocols[i];

		if (proto->clients && proto->num_clients &&
		    proto->protocol_id == (uint16_t)protocol)
			return proto;
	}

	return NULL;
}

/* The mode the controller runs decides which backend carries every protocol */
static enum ipcc_backend ipcc_active_backend(void)
{
	return (drv_ctxt.cfg->flags & IPCC_CFG_ROUTER_MODE) ?
	       IPCC_BACKEND_ROUTER : IPCC_BACKEND_DIRECT;
}

static TEE_Result ipcc_protocol_init(const struct ipcc_proto_cfg *proto)
{
	struct ipcc_rx_drv_info *rx_drv = &drv_ctxt.rx_drv[proto->protocol_id];
	uint32_t idx = 0;
	paddr_t pa = 0;
	vaddr_t va = 0;
	TEE_Result res = TEE_SUCCESS;

	mutex_init(&rx_drv->lock);

	if (!proto->has_router)
		return TEE_SUCCESS;

	res = ipcc_client_page_idx(proto, drv_ctxt.cfg->client, &idx);
	if (res) {
		EMSG("protocol %u: own client %u not described",
		     proto->protocol_id, (uint32_t)drv_ctxt.cfg->client);
		return res;
	}

	pa = proto->proto_block_base +
	     (paddr_t)idx * proto->client_stride +
	     (paddr_t)proto->protocol_id * proto->proto_stride;

	va = (vaddr_t)phys_to_virt(pa, MEM_AREA_IO_SEC, proto->client_stride);
	if (!va) {
		EMSG("protocol %u: client page %#"PRIxPA" is not mapped",
		     proto->protocol_id, pa);
		return TEE_ERROR_BAD_STATE;
	}

	drv_ctxt.router_pages[proto->protocol_id] = va;

	/* One controller, one version; every client page reports the same */
	if (!drv_ctxt.hw_version)
		drv_ctxt.hw_version = io_read32(va + IPCC_VERSION_OFF) &
				      IPCC_VERSION_MASK;

	return TEE_SUCCESS;
}

/*
 * IPC_CONFIG.TOP_MODE and IPC_TRACE.ENABLE are one register each at a block
 * offset from the controller base, and are programmed once.
 */
static TEE_Result ipcc_apply_block_cfg(const struct ipcc_cfg *cfg)
{
	vaddr_t mode_va = 0;
	vaddr_t trace_va = 0;

	if (!cfg->has_ctrl_block) {
		EMSG("no control block described, cannot select the mode");
		return TEE_ERROR_BAD_STATE;
	}

	mode_va = (vaddr_t)phys_to_virt(cfg->ctrl_block_base +
					IPCC_TOP_MODE_BLOCK_OFF,
					MEM_AREA_IO_SEC, sizeof(uint32_t));
	trace_va = (vaddr_t)phys_to_virt(cfg->ctrl_block_base +
					 IPCC_TRACE_BLOCK_OFF,
					 MEM_AREA_IO_SEC, sizeof(uint32_t));
	if (!mode_va || !trace_va) {
		EMSG("control block %#"PRIxPA" is not mapped",
		     cfg->ctrl_block_base);
		return TEE_ERROR_BAD_STATE;
	}

	io_write32(mode_va, (cfg->flags & IPCC_CFG_ROUTER_MODE) ?
			    IPCC_TOP_MODE_BIT : 0);
	io_write32(trace_va, IPCC_TRACE_ENABLE_BIT);

	return TEE_SUCCESS;
}

static TEE_Result qti_ipcc_init(void)
{
	const struct ipcc_cfg *cfg = ipcc_chipset_cfg;
	uint32_t described = 0;
	uint32_t ready = 0;
	uint32_t i = 0;
	TEE_Result res = TEE_SUCCESS;

	if (!cfg || !cfg->protocols || !cfg->num_protocols) {
		EMSG("no chipset config, driver unavailable");
		return TEE_ERROR_NOT_SUPPORTED;
	}

	/* rx_drv[] and router_pages[] hold one slot per protocol ID */
	if (cfg->num_protocols > IPCC_PROTO_TOTAL) {
		EMSG("%u protocols described, %u supported", cfg->num_protocols,
		     (uint32_t)IPCC_PROTO_TOTAL);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	drv_ctxt.cfg = cfg;

	if (cfg->flags & IPCC_CFG_NO_TME_CFG) {
		res = ipcc_apply_block_cfg(cfg);
		if (res)
			goto err;
	}

	for (i = 0; i < cfg->num_protocols; i++) {
		const struct ipcc_proto_cfg *proto = &cfg->protocols[i];

		if (!proto->clients || !proto->num_clients)
			continue;

		if (proto->protocol_id >= IPCC_PROTO_TOTAL) {
			EMSG("protocol ID %u out of range", proto->protocol_id);
			res = TEE_ERROR_BAD_PARAMETERS;
			goto err;
		}

		described++;

		if (!ipcc_protocol_init(proto))
			ready++;
	}

	if (!ready) {
		EMSG("none of the %u described protocols came up", described);
		res = TEE_ERROR_BAD_STATE;
		goto err;
	}

	IMSG("IPCC: %u/%u protocols ready, hw version %#"PRIx32, ready,
	     described, drv_ctxt.hw_version);

	return TEE_SUCCESS;

err:
	/* Leave nothing half-initialised for the API to walk into */
	drv_ctxt.cfg = NULL;

	return res;
}

TEE_Result ipcc_attach(struct ipcc_handle **h, enum ipcc_protocol protocol)
{
	const struct ipcc_proto_cfg *proto = NULL;
	const struct ipcc_backend_ops *ops = NULL;
	enum ipcc_backend backend = IPCC_BACKEND_ROUTER;
	struct ipcc_handle *handle = NULL;
	TEE_Result res = TEE_SUCCESS;

	if (!drv_ctxt.cfg)
		return TEE_ERROR_BAD_STATE;

	if (!h)
		return TEE_ERROR_BAD_PARAMETERS;

	proto = ipcc_find_proto(protocol);
	if (!proto) {
		DMSG("protocol %u not described", (uint32_t)protocol);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	backend = ipcc_active_backend();
	ops = proto->backend_ops[backend];

	/*
	 * attach and detach are what the handle's lifetime rests on, so a
	 * protocol that does not provide both cannot be attached to at all.
	 */
	if (!(proto->backends & BIT32(backend)) || !ops || !ops->attach ||
	    !ops->detach) {
		DMSG("protocol %u does not describe backend %u",
		     proto->protocol_id, (uint32_t)backend);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	handle = calloc(1, sizeof(*handle));
	if (!handle)
		return TEE_ERROR_OUT_OF_MEMORY;

	handle->protocol = proto;
	handle->rx_drv = &drv_ctxt.rx_drv[proto->protocol_id];

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
	struct ipcc_handle *handle = NULL;

	if (!drv_ctxt.cfg)
		return TEE_ERROR_BAD_STATE;

	if (!h || !*h)
		return TEE_ERROR_BAD_PARAMETERS;

	handle = *h;

	/* The backend releases whatever the handle still holds */
	handle->backend_ops->detach(handle);

	free(handle);
	*h = NULL;

	return TEE_SUCCESS;
}

TEE_Result ipcc_register_interrupt(struct ipcc_handle *h,
				   enum ipcc_client sender_id,
				   uint16_t signal_low, uint16_t signal_high,
				   ipcc_callback_t cb, void *cb_data)
{
	if (!drv_ctxt.cfg)
		return TEE_ERROR_BAD_STATE;

	if (!h || !cb || signal_low > signal_high)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!h->backend_ops->register_interrupt)
		return TEE_ERROR_NOT_SUPPORTED;

	return h->backend_ops->register_interrupt(h, sender_id, signal_low,
						  signal_high, cb, cb_data);
}

TEE_Result ipcc_deregister_interrupt(struct ipcc_handle *h,
				     enum ipcc_client sender_id,
				     uint16_t signal_low,
				     uint16_t signal_high)
{
	if (!drv_ctxt.cfg)
		return TEE_ERROR_BAD_STATE;

	if (!h || signal_low > signal_high)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!h->backend_ops->deregister_interrupt)
		return TEE_ERROR_NOT_SUPPORTED;

	return h->backend_ops->deregister_interrupt(h, sender_id, signal_low,
						    signal_high);
}

TEE_Result ipcc_trigger(struct ipcc_handle *h, enum ipcc_client target_id,
			uint16_t signal_low, uint16_t signal_high)
{
	if (!drv_ctxt.cfg)
		return TEE_ERROR_BAD_STATE;

	if (!h || signal_low > signal_high)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!h->backend_ops->signal)
		return TEE_ERROR_NOT_SUPPORTED;

	return h->backend_ops->signal(h, target_id, signal_low, signal_high);
}

early_init(qti_ipcc_init);
