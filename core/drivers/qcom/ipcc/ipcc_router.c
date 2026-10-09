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
#include <malloc.h>
#include <trace.h>
#include <util.h>

#include "ipcc_internal.h"
#include "ipcc_regs.h"

/*
 * Router mode: this TEE owns one client page per protocol, sends through it by
 * writing SEND, and receives everything the protocol carries on one shared
 * level interrupt. Demultiplexing to a callback is therefore this backend's
 * job, and a (sender, signal) pair has to be enabled in hardware before the
 * controller will route it here.
 *
 * Attaching fails unless the page resolved at init, and a page is never
 * unresolved afterwards, so every handle reaching the calls below has one.
 */

/* v3.0+ addresses a peer by its page index, earlier hardware by client ID */
static uint32_t ipcc_router_peer_id(enum ipcc_client client, uint32_t page_idx)
{
	if (ipcc_get_drv_ctxt()->hw_version >= IPCC_VERSION(3, 0))
		return page_idx;

	return (uint32_t)client;
}

static vaddr_t ipcc_router_page(const struct ipcc_proto_cfg *protocol)
{
	return ipcc_get_drv_ctxt()->router_pages[protocol->protocol_id];
}

/* Resolve the sender the controller reported against the client table */
static TEE_Result ipcc_router_sender(const struct ipcc_proto_cfg *protocol,
				     uint32_t reported_id, bool phys_idx,
				     enum ipcc_client *sender)
{
	if (phys_idx && (ipcc_get_drv_ctxt()->cfg->flags & IPCC_CFG_HW_MEM_OPT)) {
		if (reported_id >= protocol->num_clients)
			return TEE_ERROR_NOT_SUPPORTED;

		*sender = protocol->clients[reported_id].client;
	} else {
		*sender = (enum ipcc_client)reported_id;
	}

	return TEE_SUCCESS;
}

static enum itr_return ipcc_rx_isr(struct itr_handler *h)
{
	struct ipcc_rx_drv_info *rx_drv = h->data;
	const struct ipcc_proto_cfg *protocol = rx_drv->protocol;
	vaddr_t virt_base = ipcc_router_page(protocol);
	uint32_t recv_off = IPCC_RECV_ID_OFF;
	bool phys_idx = false;

	/* From v2.0 the sender is reported as a page index, not a client ID */
	if (ipcc_get_drv_ctxt()->hw_version >= IPCC_VERSION(2, 0)) {
		recv_off = IPCC_RECV_ID_PHYS_OFF;
		phys_idx = true;
	}

	while (true) {
		uint32_t data = io_read32(virt_base + recv_off);
		enum ipcc_client sender = IPCC_CLIENT_AOP;
		struct ipcc_handle *handle = NULL;
		ipcc_callback_t cb = NULL;
		void *cb_data = NULL;
		uint32_t exceptions = 0;
		uint32_t reported_id = 0;
		uint16_t sig = 0;

		if (data == IPCC_NO_DATA)
			break;

		reported_id = (data >> IPCC_RECV_ID_CLIENT_ID_SHIFT) &
			      IPCC_RECV_ID_CLIENT_ID_MASK;
		sig = (data >> IPCC_RECV_ID_SIGNAL_ID_SHIFT) &
		      IPCC_RECV_ID_SIGNAL_ID_MASK;

		if (!protocol->clear_on_recv_en)
			io_write32(virt_base + IPCC_RECV_SIGNAL_CLEAR_OFF, data);

		if (ipcc_router_sender(protocol, reported_id, phys_idx,
				       &sender)) {
			DMSG("protocol %u: signal %u from unknown page %u",
			     protocol->protocol_id, sig, reported_id);
			continue;
		}

		exceptions = cpu_spin_lock_xsave(&rx_drv->list_lock);

		for (handle = rx_drv->handles; handle && !cb;
		     handle = handle->next) {
			struct ipcc_signal_range *r = NULL;

			for (r = handle->ranges; r; r = r->next) {
				if (r->sender != sender || sig < r->signal_low ||
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
			DMSG("no handler for client %u signal %u",
			     (uint32_t)sender, sig);
	}

	return ITRR_HANDLED;
}
DECLARE_KEEP_PAGER(ipcc_rx_isr);

/* Write every signal of a range to RECV_SIGNAL_ENABLE or _DISABLE */
static void ipcc_router_set_sigs(const struct ipcc_handle *h, uint32_t reg_off,
				 enum ipcc_client sender, uint16_t signal_low,
				 uint16_t signal_high)
{
	vaddr_t virt_base = ipcc_router_page(h->protocol);
	uint32_t page_idx = 0;
	uint32_t peer = 0;
	uint16_t sig = 0;

	if (ipcc_client_page_idx(h->protocol, sender, &page_idx))
		return;

	peer = ipcc_router_peer_id(sender, page_idx);

	for (sig = signal_low; sig <= signal_high; sig++)
		io_write32(virt_base + reg_off,
			   SHIFT_U32(peer, IPCC_RECV_CLIENT_ID_SHIFT) |
			   SHIFT_U32(sig, IPCC_RECV_SIGNAL_ID_SHIFT));
}

/*
 * Ranges are visible to the ISR, so the list is ordered by list_lock. A new
 * range may not overlap one another handle already holds for the same sender,
 * otherwise a signal would have two claimants and dispatch would depend on
 * list order.
 */
static TEE_Result ipcc_add_range(struct ipcc_handle *h,
				 struct ipcc_signal_range *range)
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

static struct ipcc_signal_range *ipcc_del_range(struct ipcc_handle *h,
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

static void ipcc_add_handle(struct ipcc_handle *h)
{
	struct ipcc_rx_drv_info *rx_drv = h->rx_drv;
	uint32_t exceptions = 0;

	exceptions = cpu_spin_lock_xsave(&rx_drv->list_lock);
	h->next = rx_drv->handles;
	rx_drv->handles = h;
	cpu_spin_unlock_xrestore(&rx_drv->list_lock, exceptions);
}

static void ipcc_del_handle(struct ipcc_handle *h)
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
}

/*
 * Receiving needs this TEE's own client page, so a protocol whose page did not
 * resolve at init has nothing to attach to. The first handle takes the shared
 * interrupt and every later one only adds a reference.
 */
static TEE_Result ipcc_router_attach(struct ipcc_handle *h)
{
	const struct ipcc_proto_cfg *protocol = h->protocol;
	struct ipcc_rx_drv_info *rx_drv = h->rx_drv;
	vaddr_t virt_base = ipcc_router_page(protocol);
	TEE_Result res = TEE_SUCCESS;
	uint32_t val = 0;

	if (!virt_base) {
		DMSG("protocol %u has no client page", protocol->protocol_id);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	mutex_lock(&rx_drv->lock);

	if (rx_drv->refs) {
		rx_drv->refs++;
		mutex_unlock(&rx_drv->lock);
		ipcc_add_handle(h);

		return TEE_SUCCESS;
	}

	/* The ISR reaches the registers and the handle list through this */
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
		EMSG("protocol %u: cannot take interrupt %"PRIu32,
		     protocol->protocol_id, protocol->interrupt);
		mutex_unlock(&rx_drv->lock);

		return res;
	}

	/*
	 * The handle joins the list before the interrupt is unmasked, so the
	 * ISR never runs against a protocol with no handle to dispatch to.
	 */
	ipcc_add_handle(h);
	rx_drv->refs++;

	interrupt_enable(rx_drv->itr_hdl->chip, rx_drv->itr_hdl->it);

	mutex_unlock(&rx_drv->lock);

	return TEE_SUCCESS;
}

static void ipcc_router_detach(struct ipcc_handle *h)
{
	struct ipcc_rx_drv_info *rx_drv = h->rx_drv;
	struct ipcc_signal_range *range = NULL;

	/*
	 * Off the list first: from here the ISR cannot reach this handle, so
	 * its ranges can be torn down without holding list_lock.
	 */
	ipcc_del_handle(h);

	while (h->ranges) {
		range = h->ranges;
		h->ranges = range->next;

		ipcc_router_set_sigs(h, IPCC_RECV_SIGNAL_DISABLE_OFF,
				     range->sender, range->signal_low,
				     range->signal_high);
		free(range);
	}

	mutex_lock(&rx_drv->lock);

	/* The last handle off the protocol gives the interrupt back */
	if (rx_drv->refs && !--rx_drv->refs) {
		interrupt_disable(rx_drv->itr_hdl->chip, rx_drv->itr_hdl->it);
		interrupt_remove_free_handler(rx_drv->itr_hdl);
		rx_drv->itr_hdl = NULL;
	}

	mutex_unlock(&rx_drv->lock);
}

static TEE_Result ipcc_router_register_interrupt(struct ipcc_handle *h,
						 enum ipcc_client sender_id,
						 uint16_t signal_low,
						 uint16_t signal_high,
						 ipcc_callback_t cb,
						 void *cb_data)
{
	const struct ipcc_proto_cfg *protocol = h->protocol;
	struct ipcc_signal_range *range = NULL;
	uint32_t page_idx = 0;
	TEE_Result res = TEE_SUCCESS;

	if (!ipcc_sig_range_valid(signal_low, signal_high, protocol->num_sigs))
		return TEE_ERROR_BAD_PARAMETERS;

	/* Refuse a sender the protocol does not describe before allocating */
	res = ipcc_client_page_idx(protocol, sender_id, &page_idx);
	if (res)
		return res;

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

	/* Enabled last: the ISR can dispatch as soon as the controller routes */
	ipcc_router_set_sigs(h, IPCC_RECV_SIGNAL_ENABLE_OFF, sender_id,
			     signal_low, signal_high);

	return TEE_SUCCESS;
}

static TEE_Result ipcc_router_deregister_interrupt(struct ipcc_handle *h,
						   enum ipcc_client sender_id,
						   uint16_t signal_low,
						   uint16_t signal_high)
{
	struct ipcc_signal_range *range = NULL;

	range = ipcc_del_range(h, sender_id, signal_low, signal_high);
	if (!range)
		return TEE_ERROR_ITEM_NOT_FOUND;

	ipcc_router_set_sigs(h, IPCC_RECV_SIGNAL_DISABLE_OFF, sender_id,
			     signal_low, signal_high);
	free(range);

	return TEE_SUCCESS;
}

static TEE_Result ipcc_router_signal(struct ipcc_handle *h,
				     enum ipcc_client target_id,
				     uint16_t signal_low, uint16_t signal_high)
{
	const struct ipcc_proto_cfg *protocol = h->protocol;
	vaddr_t virt_base = ipcc_router_page(protocol);
	uint32_t page_idx = 0;
	uint32_t target = 0;
	uint16_t sig = 0;
	TEE_Result res = TEE_SUCCESS;

	if (!ipcc_sig_range_valid(signal_low, signal_high, protocol->num_sigs))
		return TEE_ERROR_BAD_PARAMETERS;

	res = ipcc_client_page_idx(protocol, target_id, &page_idx);
	if (res)
		return res;

	target = ipcc_router_peer_id(target_id, page_idx);

	for (sig = signal_low; sig <= signal_high; sig++)
		io_write32(virt_base + IPCC_SEND_OFF,
			   SHIFT_U32(target & IPCC_SEND_CLIENT_ID_MASK,
				     IPCC_SEND_CLIENT_ID_SHIFT) |
			   SHIFT_U32(sig & IPCC_SEND_SIGNAL_ID_MASK,
				     IPCC_SEND_SIGNAL_ID_SHIFT));

	return TEE_SUCCESS;
}

const struct ipcc_backend_ops ipcc_router_ops = {
	.attach = ipcc_router_attach,
	.detach = ipcc_router_detach,
	.register_interrupt = ipcc_router_register_interrupt,
	.deregister_interrupt = ipcc_router_deregister_interrupt,
	.signal = ipcc_router_signal,
};
