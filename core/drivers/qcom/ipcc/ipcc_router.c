/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <inttypes.h>
#include <io.h>
#include <keep.h>
#include <kernel/interrupt.h>
#include <kernel/mutex.h>
#include <kernel/spinlock.h>
#include <trace.h>
#include <util.h>

#include "ipcc_internal.h"
#include "ipcc_regs.h"

/* v3.0+ addresses a peer by physical client page index, earlier HW by client ID */
static uint32_t ipcc_router_client_id(enum ipcc_client client, uint32_t phys_idx)
{
	if (ipcc_get_drv_ctxt()->hw_version >= IPCC_VERSION(3, 0))
		return phys_idx;

	return (uint32_t)client;
}

static enum itr_return ipcc_rx_isr(struct itr_handler *h)
{
	struct ipcc_rx_drv_info *rx_drv = h->data;
	const struct ipcc_proto_cfg *protocol = rx_drv->protocol;
	uintptr_t virt_base = ipcc_get_drv_ctxt()->router_pages[protocol->protocol_id];
	uint32_t recv_off = IPCC_RECV_ID_OFF;
	bool phys = false;
	uint32_t data = 0;

	if (ipcc_get_drv_ctxt()->hw_version >= IPCC_VERSION(2, 0)) {
		recv_off = IPCC_RECV_ID_PHYS_OFF;
		phys = true;
	}

	data = io_read32(virt_base + recv_off);

	while (data != IPCC_NO_DATA) {
		uint32_t reported_id = (data >> IPCC_RECV_ID_CLIENT_ID_SHIFT) &
				       IPCC_RECV_ID_CLIENT_ID_MASK;
		uint16_t sig = (data >> IPCC_RECV_ID_SIGNAL_ID_SHIFT) &
			       IPCC_RECV_ID_SIGNAL_ID_MASK;
		enum ipcc_client sender = IPCC_CLIENT_AOP;
		struct ipcc_handle *handle = NULL;
		ipcc_callback_t cb = NULL;
		uint32_t exceptions = 0;
		void *cb_data = NULL;

		if (!protocol->clear_on_recv_en)
			io_write32(virt_base + IPCC_RECV_SIGNAL_CLEAR_OFF, data);

		if (phys && (ipcc_get_drv_ctxt()->cfg->flags & IPCC_CFG_HW_MEM_OPT)) {
			if (reported_id < protocol->num_clients)
				sender = protocol->clients[reported_id].client;
			else
				sender = (enum ipcc_client)-1;
		} else {
			sender = (enum ipcc_client)reported_id;
		}

		exceptions = cpu_spin_lock_xsave(&rx_drv->list_lock);

		for (handle = rx_drv->handles; handle && !cb;
		     handle = handle->next) {
			struct ipcc_signal_range *r = NULL;

			for (r = handle->ranges; r; r = r->next) {
				if (r->sender != sender ||
				    sig < r->signal_low ||
				    sig > r->signal_high)
					continue;

				cb = r->cb;
				cb_data = r->cb_data;
				break;
			}
		}

		cpu_spin_unlock_xrestore(&rx_drv->list_lock, exceptions);

		if (cb)
			cb(cb_data, sender, sig);
		else
			DMSG("no handler for client %d signal %u", sender, sig);

		data = io_read32(virt_base + recv_off);
	}

	return ITRR_HANDLED;
}
DECLARE_KEEP_PAGER(ipcc_rx_isr);

/*
 * Router mode receives through this TEE's own client page, so a protocol whose
 * page did not resolve at init has nothing to attach to. The first handle takes
 * the shared interrupt and every later one only adds a reference.
 */
TEE_Result ipcc_router_attach(struct ipcc_handle *h)
{
	const struct ipcc_proto_cfg *protocol = h->protocol;
	struct ipcc_rx_drv_info *rx_drv = h->rx_drv;
	uintptr_t virt_base = ipcc_get_drv_ctxt()->router_pages[protocol->protocol_id];
	TEE_Result res = TEE_SUCCESS;
	uint32_t exceptions = 0;
	uint32_t val = 0;

	if (!virt_base) {
		DMSG("protocol %u has no client page", protocol->protocol_id);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	mutex_lock(&rx_drv->lock);

	if (rx_drv->refs) {
		rx_drv->refs++;
		mutex_unlock(&rx_drv->lock);

		exceptions = cpu_spin_lock_xsave(&rx_drv->list_lock);
		h->next = rx_drv->handles;
		rx_drv->handles = h;
		cpu_spin_unlock_xrestore(&rx_drv->list_lock, exceptions);

		return TEE_SUCCESS;
	}

	/* The Rx handler reaches the registers through this */
	rx_drv->protocol = protocol;

	/* Drop whatever was left pending on our page before we owned it */
	io_write32(virt_base + IPCC_CLIENT_CLEAR_OFF, IPCC_CLIENT_CLEAR);

	if (protocol->clear_on_recv_en) {
		val = io_read32(virt_base + IPCC_CONFIG_OFF);
		io_write32(virt_base + IPCC_CONFIG_OFF,
			   val | IPCC_CONFIG_CLEAR_ON_RECV_RD);
	}

	res = interrupt_alloc_add_handler(interrupt_get_main_chip(),
					  protocol->interrupt, ipcc_rx_isr,
					  ITRF_TRIGGER_LEVEL, rx_drv,
					  &rx_drv->itr_hdl);
	if (res) {
		EMSG("protocol %u: failed to take interrupt %"PRIu32,
		     protocol->protocol_id, protocol->interrupt);
		mutex_unlock(&rx_drv->lock);

		return res;
	}

	interrupt_enable(rx_drv->itr_hdl->chip, rx_drv->itr_hdl->it);

	rx_drv->refs++;

	mutex_unlock(&rx_drv->lock);

	exceptions = cpu_spin_lock_xsave(&rx_drv->list_lock);
	h->next = rx_drv->handles;
	rx_drv->handles = h;
	cpu_spin_unlock_xrestore(&rx_drv->list_lock, exceptions);

	return TEE_SUCCESS;
}

void ipcc_router_detach(struct ipcc_handle *h)
{
	struct ipcc_rx_drv_info *rx_drv = h->rx_drv;
	struct ipcc_handle *cur = NULL;
	struct ipcc_handle *prev = NULL;
	uint32_t exceptions = 0;

	exceptions = cpu_spin_lock_xsave(&rx_drv->list_lock);

	for (cur = rx_drv->handles; cur; cur = cur->next) {
		if (cur == h) {
			if (prev)
				prev->next = cur->next;
			else
				rx_drv->handles = cur->next;
			break;
		}
		prev = cur;
	}

	cpu_spin_unlock_xrestore(&rx_drv->list_lock, exceptions);

	mutex_lock(&rx_drv->lock);

	/* The last handle off the protocol gives the interrupt back */
	if (rx_drv->refs && !--rx_drv->refs) {
		interrupt_disable(rx_drv->itr_hdl->chip, rx_drv->itr_hdl->it);
		interrupt_remove_free_handler(rx_drv->itr_hdl);
		rx_drv->itr_hdl = NULL;
	}

	mutex_unlock(&rx_drv->lock);
}

TEE_Result ipcc_router_register_interrupt(struct ipcc_handle *h,
					  enum ipcc_client sender_id,
					  uint16_t signal_low,
					  uint16_t signal_high,
					  ipcc_callback_t cb, void *cb_data)
{
	const struct ipcc_proto_cfg *protocol = h->protocol;
	uintptr_t virt_base = ipcc_get_drv_ctxt()->router_pages[protocol->protocol_id];
	struct ipcc_signal_range *range = NULL;
	TEE_Result res = TEE_SUCCESS;
	uint32_t phys_idx = 0;
	uint32_t peer = 0;
	uint16_t sig = 0;

	if (!virt_base)
		return TEE_ERROR_NOT_SUPPORTED;

	if (!ipcc_sig_range_valid(signal_low, signal_high, protocol->num_sigs))
		return TEE_ERROR_BAD_PARAMETERS;

	res = ipcc_find_phys_client_idx(sender_id, protocol, &phys_idx);
	if (res)
		return res;

	peer = ipcc_router_client_id(sender_id, phys_idx);

	/* Allocate range before taking spinlock */
	range = calloc(1, sizeof(*range));
	if (!range)
		return TEE_ERROR_OUT_OF_MEMORY;

	range->sender = sender_id;
	range->signal_low = signal_low;
	range->signal_high = signal_high;
	range->cb = cb;
	range->cb_data = cb_data;

	res = ipcc_add_range(h, range);
	if (res) {
		free(range);
		return res;
	}

	for (sig = signal_low; sig <= signal_high; sig++)
		io_write32(virt_base + IPCC_RECV_SIGNAL_ENABLE_OFF,
			   SHIFT_U32(peer, IPCC_RECV_CLIENT_ID_SHIFT) |
			   SHIFT_U32(sig, IPCC_RECV_SIGNAL_ID_SHIFT));

	return TEE_SUCCESS;
}

TEE_Result ipcc_router_deregister_interrupt(struct ipcc_handle *h,
					    enum ipcc_client sender_id,
					    uint16_t signal_low,
					    uint16_t signal_high)
{
	const struct ipcc_proto_cfg *protocol = h->protocol;
	uintptr_t virt_base = ipcc_get_drv_ctxt()->router_pages[protocol->protocol_id];
	struct ipcc_signal_range *range = NULL;
	uint32_t phys_idx = 0;
	uint32_t peer = 0;
	uint16_t sig = 0;

	if (!virt_base)
		return TEE_ERROR_NOT_SUPPORTED;

	range = ipcc_del_range(h, sender_id, signal_low, signal_high);
	if (!range)
		return TEE_ERROR_ITEM_NOT_FOUND;

	if (ipcc_find_phys_client_idx(sender_id, protocol, &phys_idx))
		goto out;

	peer = ipcc_router_client_id(sender_id, phys_idx);

	for (sig = signal_low; sig <= signal_high; sig++)
		io_write32(virt_base + IPCC_RECV_SIGNAL_DISABLE_OFF,
			   SHIFT_U32(peer, IPCC_RECV_CLIENT_ID_SHIFT) |
			   SHIFT_U32(sig, IPCC_RECV_SIGNAL_ID_SHIFT));

out:
	free(range);
	return TEE_SUCCESS;
}

TEE_Result ipcc_router_signal(struct ipcc_handle *h, enum ipcc_client target_id,
			      uint16_t signal_low, uint16_t signal_high)
{
	const struct ipcc_proto_cfg *protocol = h->protocol;
	uintptr_t virt_base = ipcc_get_drv_ctxt()->router_pages[protocol->protocol_id];
	TEE_Result res = TEE_SUCCESS;
	uint32_t phys_idx = 0;
	uint32_t target = 0;
	uint16_t sig = 0;

	/* Sending goes out through our own page, which has to have resolved */
	if (!virt_base)
		return TEE_ERROR_NOT_SUPPORTED;

	if (!ipcc_sig_range_valid(signal_low, signal_high, protocol->num_sigs))
		return TEE_ERROR_BAD_PARAMETERS;

	res = ipcc_find_phys_client_idx(target_id, protocol, &phys_idx);
	if (res)
		return res;

	target = ipcc_router_client_id(target_id, phys_idx);

	for (sig = signal_low; sig <= signal_high; sig++)
		io_write32(virt_base + IPCC_SEND_OFF,
			   SHIFT_U32(target & IPCC_SEND_CLIENT_ID_MASK,
				     IPCC_SEND_CLIENT_ID_SHIFT) |
			   SHIFT_U32(sig & IPCC_SEND_SIGNAL_ID_MASK,
				     IPCC_SEND_SIGNAL_ID_SHIFT));

	return TEE_SUCCESS;
}

/*
 * Helper function to find physical client index for router operations.
 * This is used by both router and direct backends.
 */
TEE_Result ipcc_find_phys_client_idx(enum ipcc_client client,
				     const struct ipcc_proto_cfg *protocol,
				     uint32_t *phys_idx)
{
	uint32_t idx = 0;

	for (idx = 0; idx < protocol->num_clients; idx++) {
		if (protocol->clients[idx].client != client)
			continue;

		/* Packed pages are addressed by row, all others by client ID */
		*phys_idx = (ipcc_get_drv_ctxt()->cfg->flags & IPCC_CFG_HW_MEM_OPT) ?
			    idx : (uint32_t)client;

		return TEE_SUCCESS;
	}

	return TEE_ERROR_NOT_SUPPORTED;
}

/* Range management helpers */
TEE_Result ipcc_add_range(struct ipcc_handle *h, struct ipcc_signal_range *range)
{
	struct ipcc_handle *cur = NULL;
	uint32_t exceptions = 0;

	exceptions = cpu_spin_lock_xsave(&h->rx_drv->list_lock);

	for (cur = h->rx_drv->handles; cur; cur = cur->next) {
		struct ipcc_signal_range *r = NULL;

		for (r = cur->ranges; r; r = r->next) {
			if (r->sender != range->sender ||
			    r->signal_low > range->signal_high ||
			    range->signal_low > r->signal_high)
				continue;

			cpu_spin_unlock_xrestore(&h->rx_drv->list_lock,
						 exceptions);

			return TEE_ERROR_ACCESS_CONFLICT;
		}
	}

	range->next = h->ranges;
	h->ranges = range;

	cpu_spin_unlock_xrestore(&h->rx_drv->list_lock, exceptions);

	return TEE_SUCCESS;
}

struct ipcc_signal_range *ipcc_del_range(struct ipcc_handle *h,
					 enum ipcc_client sender,
					 uint16_t signal_low,
					 uint16_t signal_high)
{
	struct ipcc_signal_range *range = NULL;
	struct ipcc_signal_range *prev = NULL;
	uint32_t exceptions = 0;

	exceptions = cpu_spin_lock_xsave(&h->rx_drv->list_lock);

	for (range = h->ranges; range; range = range->next) {
		if (range->sender == sender &&
		    range->signal_low == signal_low &&
		    range->signal_high == signal_high)
			break;
		prev = range;
	}

	if (range) {
		if (prev)
			prev->next = range->next;
		else
			h->ranges = range->next;
	}

	cpu_spin_unlock_xrestore(&h->rx_drv->list_lock, exceptions);

	return range;
}
