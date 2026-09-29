/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __IPCC_INTERNAL_H__
#define __IPCC_INTERNAL_H__

#include <drivers/qcom/ipcc/ipcc.h>
#include <kernel/interrupt.h>
#include <kernel/mutex.h>
#include <kernel/spinlock.h>
#include <stdbool.h>
#include <stdint.h>
#include <types_ext.h>
#include <util.h>

/*
 * The controller runs one of two mechanisms for the whole block, selected by
 * IPC_CONFIG.TOP_MODE, and the two differ in where an interrupt comes from and
 * what a sender writes:
 *
 * Router mode multiplexes. This TEE owns one client page per protocol and the
 * whole protocol shares a single level interrupt. Tx writes (target, signal)
 * to SEND in our own page; Rx drains RECV_ID in the ISR and demultiplexes
 * (sender, signal) in software to the callback that registered the range, so a
 * range has to be enabled in hardware per (sender, signal) pair.
 *
 * Direct mode does not multiplex. Every (client, signal) pair owns a dedicated
 * interrupt line and the sender writes a precomputed mask to the target's own
 * trigger register, both supplied by the chipset table. Rx installs a handler
 * on the signal's own line, so there is no software demultiplexing and no
 * per-pair enable.
 */
enum ipcc_backend {
	IPCC_BACKEND_ROUTER = 0,
	IPCC_BACKEND_DIRECT = 1,

	IPCC_BACKEND_COUNT,
};

/* Which backends a protocol describes, as a mask of BIT(enum ipcc_backend) */
#define IPCC_CAP_ROUTER  BIT32(IPCC_BACKEND_ROUTER)
#define IPCC_CAP_DIRECT  BIT32(IPCC_BACKEND_DIRECT)

/*
 * IPCC_CFG_NO_TME_CFG:   no TME owns IPC_CONFIG/IPC_TRACE, so this driver
 *                        programs them itself.
 * IPCC_CFG_ROUTER_MODE:  the controller runs router mode; without it, direct.
 * IPCC_CFG_HW_MEM_OPT:   client pages are packed, so a page is addressed by
 *                        its row in the client table rather than by client ID.
 */
#define IPCC_CFG_NO_TME_CFG   BIT32(0)
#define IPCC_CFG_ROUTER_MODE  BIT32(1)
#define IPCC_CFG_HW_MEM_OPT   BIT32(2)

/* Only the clients a chipset instantiates are listed; absence rejects */
struct ipcc_client_cfg {
	enum ipcc_client client;
};

struct ipcc_handle;

struct ipcc_backend_ops {
	TEE_Result (*attach)(struct ipcc_handle *h);
	void (*detach)(struct ipcc_handle *h);
	TEE_Result (*register_interrupt)(struct ipcc_handle *h,
					 enum ipcc_client sender_id,
					 uint16_t signal_low,
					 uint16_t signal_high,
					 ipcc_callback_t cb, void *cb_data);
	TEE_Result (*deregister_interrupt)(struct ipcc_handle *h,
					   enum ipcc_client sender_id,
					   uint16_t signal_low,
					   uint16_t signal_high);
	TEE_Result (*signal)(struct ipcc_handle *h, enum ipcc_client target_id,
			     uint16_t signal_low, uint16_t signal_high);
};

/*
 * A protocol slot is described only if it has a client table; a chipset may
 * leave holes when it initialises the array by protocol ID.
 */
struct ipcc_proto_cfg {
	uint16_t protocol_id;
	uint16_t num_sigs;
	uint32_t num_clients;
	const struct ipcc_client_cfg *clients;
	paddr_t proto_block_base;
	uint32_t proto_stride;
	uint32_t client_stride;
	uint32_t backends;
	const struct ipcc_backend_ops *backend_ops[IPCC_BACKEND_COUNT];
	/* Router mode: this TEE owns a client page on a shared interrupt */
	bool has_router;
	uint32_t interrupt;
	/* Router mode: reading RECV_ID clears the signal for the ISR */
	bool clear_on_recv_en;
};

struct ipcc_cfg {
	const struct ipcc_proto_cfg *protocols;
	uint32_t num_protocols;
	/* The client ID this TEE owns, and so the page it receives on */
	enum ipcc_client client;
	paddr_t ctrl_block_base;
	bool has_ctrl_block;
	uint32_t flags;
};

/*
 * One per protocol slot: owns the shared Rx interrupt and the list of handles
 * attached to that protocol. list_lock orders the ISR against the API; lock
 * serialises the interrupt refcount, which the ISR never touches.
 */
struct ipcc_rx_drv_info {
	struct mutex lock;
	unsigned int list_lock;
	uint32_t refs;
	const struct ipcc_proto_cfg *protocol;
	struct itr_handler *itr_hdl;
	struct ipcc_handle *handles;
};

/*
 * Mutable driver state, kept apart from the const config so that table can
 * live in read-only memory. router_pages[] and rx_drv[] are indexed by
 * protocol ID, not by position in the config table.
 */
struct ipcc_drv_ctxt {
	const struct ipcc_cfg *cfg;
	vaddr_t router_pages[IPCC_PROTO_TOTAL];
	struct ipcc_rx_drv_info rx_drv[IPCC_PROTO_TOTAL];
	uint32_t hw_version;
};

const struct ipcc_drv_ctxt *ipcc_get_drv_ctxt(void);

/*
 * Resolve a client to the index of its page, rejecting a client the protocol
 * does not describe.
 */
TEE_Result ipcc_client_page_idx(const struct ipcc_proto_cfg *proto,
				enum ipcc_client client, uint32_t *idx);

/* One per sender, the range of signals a handle registered a callback for */
struct ipcc_signal_range {
	struct ipcc_signal_range *next;
	enum ipcc_client sender;
	uint16_t signal_low;
	uint16_t signal_high;
	ipcc_callback_t cb;
	void *cb_data;
};

struct ipcc_handle {
	const struct ipcc_proto_cfg *protocol;
	const struct ipcc_backend_ops *backend_ops;
	struct ipcc_rx_drv_info *rx_drv;
	/* Router mode only: the ranges this handle registered */
	struct ipcc_signal_range *ranges;
	struct ipcc_handle *next;
};

/* Backend operation tables, named by the chipset config */
extern const struct ipcc_backend_ops ipcc_router_ops;
extern const struct ipcc_backend_ops ipcc_direct_ops;

/* Sentinel for a signal with no dedicated direct interrupt line */
#define IPCC_DIRECT_INT_NONE		0xffffffffU

/*
 * A signal that cannot be raised has out_mask == 0, one that cannot be
 * received has interrupt_id == IPCC_DIRECT_INT_NONE; both are refused. The
 * fields below interrupt_id/out_mask are driver state, filled in while a
 * callback is registered and cleared when it is released.
 */
struct ipcc_direct_signal {
	uint32_t interrupt_id;
	uint32_t out_mask;
	const struct ipcc_handle *owner;
	enum ipcc_client sender_id;
	uint16_t signal;
	ipcc_callback_t cb;
	void *cb_data;
	struct itr_handler *itr_hdl;
};

/* signals[] is indexed by signal number, clients[] by client ID */
struct ipcc_direct_client {
	uint32_t num_signals;
	struct ipcc_direct_signal *signals;
	paddr_t trigger_reg;
};

struct ipcc_direct_cfg {
	uint32_t num_clients;
	struct ipcc_direct_client *clients;
};

/* Both provided by <chipset>/ipcc_config.c; the direct table may be NULL */
extern const struct ipcc_cfg *const ipcc_chipset_cfg;
extern const struct ipcc_direct_cfg *const ipcc_chipset_direct_cfg;

/* A signal range is valid only when ordered and below the limit in use */
static inline bool ipcc_sig_range_valid(uint16_t sig_lo, uint16_t sig_hi,
					uint32_t limit)
{
	return sig_lo <= sig_hi && sig_hi < limit;
}

#endif /* __IPCC_INTERNAL_H__ */
