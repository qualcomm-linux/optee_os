/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <inttypes.h>
#include <io.h>
#include <keep.h>
#include <kernel/interrupt.h>
#include <kernel/mutex.h>
#include <mm/core_memprot.h>
#include <trace.h>

#include "ipcc_internal.h"

static struct mutex ipcc_direct_lock = MUTEX_INITIALIZER;

static struct ipcc_direct_client *ipcc_direct_get_client(enum ipcc_client cid)
{
	const struct ipcc_direct_cfg *cfg = ipcc_chipset_direct_cfg;

	if (!cfg || (uint32_t)cid >= cfg->num_clients)
		return NULL;

	if (!cfg->clients[cid].num_signals || !cfg->clients[cid].signals)
		return NULL;

	return &cfg->clients[cid];
}

static enum itr_return ipcc_direct_isr(struct itr_handler *h)
{
	struct ipcc_direct_signal *dsig = h->data;

	if (dsig->cb)
		dsig->cb(dsig->cb_data, dsig->sender_id, dsig->signal);

	return ITRR_HANDLED;
}
DECLARE_KEEP_PAGER(ipcc_direct_isr);

static void ipcc_direct_free_line(struct ipcc_direct_signal *dsig)
{
	if (dsig->itr_hdl) {
		interrupt_disable(dsig->itr_hdl->chip, dsig->itr_hdl->it);
		interrupt_remove_free_handler(dsig->itr_hdl);
		dsig->itr_hdl = NULL;
	}

	dsig->cb = NULL;
	dsig->cb_data = NULL;
}

TEE_Result ipcc_direct_attach(struct ipcc_handle *h)
{
	/* Direct mode doesn't require attach operations */
	(void)h;
	return TEE_SUCCESS;
}

void ipcc_direct_detach(struct ipcc_handle *h)
{
	/* Direct mode doesn't require detach operations */
	(void)h;
}

TEE_Result ipcc_direct_register_interrupt(struct ipcc_handle *h,
					  enum ipcc_client sender_id,
					  uint16_t signal_low,
					  uint16_t signal_high,
					  ipcc_callback_t cb, void *cb_data)
{
	struct ipcc_direct_client *client = NULL;
	TEE_Result res = TEE_SUCCESS;
	uint16_t sig = 0;
	uint16_t idx = 0;

	(void)h;

	client = ipcc_direct_get_client(sender_id);
	if (!client || signal_high >= client->num_signals)
		return TEE_ERROR_BAD_PARAMETERS;

	mutex_lock(&ipcc_direct_lock);

	/* Take the whole range or none of it */
	for (sig = signal_low; sig <= signal_high; sig++) {
		if (client->signals[sig].interrupt_id == IPCC_DIRECT_INT_NONE) {
			DMSG("client %d signal %u cannot be received",
			     sender_id, sig);
			res = TEE_ERROR_NOT_SUPPORTED;
			goto out;
		}

		if (client->signals[sig].cb) {
			DMSG("client %d signal %u already registered",
			     sender_id, sig);
			res = TEE_ERROR_ACCESS_CONFLICT;
			goto out;
		}
	}

	for (sig = signal_low; sig <= signal_high; sig++) {
		struct ipcc_direct_signal *dsig = &client->signals[sig];

		dsig->cb = cb;
		dsig->cb_data = cb_data;
		dsig->sender_id = sender_id;
		dsig->signal = sig;

		res = interrupt_alloc_add_handler(interrupt_get_main_chip(),
						  dsig->interrupt_id,
						  ipcc_direct_isr,
						  ITRF_TRIGGER_LEVEL, dsig,
						  &dsig->itr_hdl);
		if (res) {
			EMSG("client %d signal %u: interrupt %"PRIu32" failed",
			     sender_id, sig, dsig->interrupt_id);
			goto err;
		}

		interrupt_enable(dsig->itr_hdl->chip, dsig->itr_hdl->it);
	}

	mutex_unlock(&ipcc_direct_lock);

	return TEE_SUCCESS;

err:
	/* Undo the part of the range taken, this signal included */
	for (idx = signal_low; idx <= sig; idx++)
		ipcc_direct_free_line(&client->signals[idx]);
out:
	mutex_unlock(&ipcc_direct_lock);

	return res;
}

/*
 * The caller owns the range it deregisters, so anything the table no longer
 * describes is a range that was never enabled and there is nothing to undo.
 */
TEE_Result ipcc_direct_deregister_interrupt(struct ipcc_handle *h,
					    enum ipcc_client sender_id,
					    uint16_t signal_low,
					    uint16_t signal_high)
{
	struct ipcc_direct_client *client = NULL;
	uint16_t sig = 0;

	(void)h;

	client = ipcc_direct_get_client(sender_id);
	if (!client || signal_high >= client->num_signals)
		return TEE_ERROR_BAD_PARAMETERS;

	mutex_lock(&ipcc_direct_lock);

	for (sig = signal_low; sig <= signal_high; sig++)
		ipcc_direct_free_line(&client->signals[sig]);

	mutex_unlock(&ipcc_direct_lock);

	return TEE_SUCCESS;
}

TEE_Result ipcc_direct_signal(struct ipcc_handle *h, enum ipcc_client target_id,
			      uint16_t signal_low, uint16_t signal_high)
{
	const struct ipcc_direct_client *client = NULL;
	vaddr_t reg_va = 0;
	uint16_t sig = 0;

	(void)h;

	client = ipcc_direct_get_client(target_id);
	if (!client || signal_high >= client->num_signals)
		return TEE_ERROR_BAD_PARAMETERS;

	/* A receive-only client has no trigger register at all */
	if (!client->trigger_reg) {
		DMSG("client %d has no trigger register", target_id);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	/* A signal with no mask cannot be raised; reject before writing any */
	for (sig = signal_low; sig <= signal_high; sig++)
		if (!client->signals[sig].out_mask) {
			DMSG("client %d signal %u cannot be raised", target_id,
			     sig);
			return TEE_ERROR_NOT_SUPPORTED;
		}

	reg_va = (vaddr_t)phys_to_virt(client->trigger_reg, MEM_AREA_IO_SEC,
				       sizeof(uint32_t));
	if (!reg_va) {
		EMSG("client %d: trigger register %#"PRIxPA" is not mapped",
		     target_id, client->trigger_reg);
		return TEE_ERROR_BAD_STATE;
	}

	for (sig = signal_low; sig <= signal_high; sig++)
		io_write32(reg_va, client->signals[sig].out_mask);

	return TEE_SUCCESS;
}
