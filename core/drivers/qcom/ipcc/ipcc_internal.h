/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __IPCC_INTERNAL_H__
#define __IPCC_INTERNAL_H__

#include <drivers/qcom/ipcc/ipcc.h>
#include <kernel/interrupt.h>
#include <kernel/mutex.h>
#include <stdbool.h>
#include <stdint.h>
#include <types_ext.h>

/* Backend selection: which mechanism to use for signaling. */
enum ipcc_backend {
	IPCC_BACKEND_ROUTER = 0,
	IPCC_BACKEND_DIRECT = 1,
};

/* Backend capability flags for protocols. */
#define IPCC_CAP_DIRECT  (1U << IPCC_BACKEND_DIRECT)
#define IPCC_CAP_ROUTER  (1U << IPCC_BACKEND_ROUTER)

/* Chipset configuration flags. */
#define IPCC_CFG_NO_TME_CFG   (1U << 0)
#define IPCC_CFG_ROUTER_MODE  (1U << 1)
#define IPCC_CFG_HW_MEM_OPT   (1U << 2)

/* Only the clients a chipset instantiates are listed; absence rejects. */
struct ipcc_client_cfg {
	enum ipcc_client client;
	enum ipcc_backend backend;
};

/* One request for either backend; router_page_base and idx are router-only. */
struct ipcc_signal_req {
	uintptr_t router_page_base;
	enum ipcc_client cid;
	uint32_t idx;
	uint16_t sig_lo;
	uint16_t sig_hi;
	uint16_t num_sigs;
};

/* Backend operations: extensible for future requirements. */
struct ipcc_backend_ops {
	TEE_Result (*attach)(struct ipcc_handle *h);
	void (*detach)(struct ipcc_handle *h);
	TEE_Result (*register_interrupt)(struct ipcc_handle *h,
					 enum ipcc_client sender_id,
					 uint16_t signal_low, uint16_t signal_high,
					 ipcc_callback_t cb, void *cb_data);
	TEE_Result (*deregister_interrupt)(struct ipcc_handle *h,
					   enum ipcc_client sender_id,
					   uint16_t signal_low, uint16_t signal_high);
	TEE_Result (*signal)(struct ipcc_handle *h, enum ipcc_client target_id,
			    uint16_t signal_low, uint16_t signal_high);
};

/* Handle forward declaration for function pointers */
struct ipcc_handle;

struct ipcc_proto_cfg {
	uint16_t protocol_id;
	uint16_t num_sigs;
	uint32_t num_clients;
	uintptr_t proto_block_base;
	bool has_router;
	const struct ipcc_client_cfg *clients;
	uint32_t backends;
	uint32_t proto_stride;
	uint32_t client_stride;
	const struct ipcc_backend_ops *backend_ops[2];
	/* Router-mode Rx interrupt for this protocol's shared client page */
	uint32_t interrupt;
	bool clear_on_recv_en;
};

struct ipcc_cfg {
	const struct ipcc_proto_cfg *protocols;
	uint32_t num_protocols;
	enum ipcc_client client;
	uintptr_t ctrl_block_base;
	bool has_ctrl_block;
	uint32_t flags;
};

/*
 * One per protocol slot: owns the shared Rx interrupt handler and the list of
 * handles attached to that protocol's router-mode client page.
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
 * live in read-only memory.
 */
struct ipcc_drv_ctxt {
	const struct ipcc_cfg *cfg;
	/* Resolved virtual address of each protocol's own client page */
	uintptr_t router_pages[IPCC_PROTO_TOTAL];
	struct ipcc_rx_drv_info rx_drv[IPCC_PROTO_TOTAL];
	uint32_t hw_version;
};

/* Provided by <chipset>/ipcc_config.c. */
extern const struct ipcc_cfg *const ipcc_chipset_cfg;

const struct ipcc_drv_ctxt *ipcc_get_drv_ctxt(void);

/* One per sender, the range of signals a handle registered a callback for */
struct ipcc_signal_range {
	struct ipcc_signal_range *next;
	enum ipcc_client sender;
	uint16_t signal_low;
	uint16_t signal_high;
	ipcc_callback_t cb;
	void *cb_data;
};

/* Handle structure for IPCC operations */
struct ipcc_handle {
	const struct ipcc_proto_cfg *protocol;
	const struct ipcc_backend_ops *backend_ops;
	/* Router-mode only: shared protocol Rx state and this handle's ranges */
	struct ipcc_rx_drv_info *rx_drv;
	struct ipcc_signal_range *ranges;
	struct ipcc_handle *next;
};

/* Core API functions declared in <drivers/qcom/ipcc/ipcc.h> */

/* Backend operations exported for protocol configuration. */
extern const struct ipcc_backend_ops ipcc_direct_ops;
extern const struct ipcc_backend_ops ipcc_router_ops;

/* Sentinel for a signal with no dedicated direct interrupt line */
#define IPCC_DIRECT_INT_NONE		0xffffffffU

/*
 * A signal that cannot be raised has out_mask == 0, one that cannot be
 * received has interrupt_id == IPCC_DIRECT_INT_NONE; both are refused.
 * sender_id, signal, cb, cb_data and itr_hdl are filled in by the driver
 * while a callback is registered, and cleared while it is deregistered.
 */
struct ipcc_direct_signal {
	uint32_t interrupt_id;
	uint32_t out_mask;
	enum ipcc_client sender_id;
	uint16_t signal;
	ipcc_callback_t cb;
	void *cb_data;
	struct itr_handler *itr_hdl;
};

struct ipcc_direct_client {
	uint32_t num_signals;
	struct ipcc_direct_signal *signals;
	paddr_t trigger_reg;
};

struct ipcc_direct_cfg {
	uint32_t num_clients;
	struct ipcc_direct_client *clients;
};

/* Provided by <chipset>/ipcc_config.c, NULL when no direct path exists. */
extern const struct ipcc_direct_cfg *const ipcc_chipset_direct_cfg;

/* Helper functions for range management (used by router backend) */
TEE_Result ipcc_add_range(struct ipcc_handle *h, struct ipcc_signal_range *range);
struct ipcc_signal_range *ipcc_del_range(struct ipcc_handle *h,
					 enum ipcc_client sender,
					 uint16_t signal_low,
					 uint16_t signal_high);

/* Helper function to find physical client index */
TEE_Result ipcc_find_phys_client_idx(enum ipcc_client client,
				     const struct ipcc_proto_cfg *protocol,
				     uint32_t *phys_idx);

static inline bool ipcc_backend_supported(uint32_t backends, enum ipcc_backend backend)
{
	return (backends & (1U << backend)) != 0U;
}

/* Shared bounds check: a signal range is valid only below the protocol/client limit */
static inline bool ipcc_sig_range_valid(uint16_t sig_lo, uint16_t sig_hi,
					uint32_t limit)
{
	return sig_lo <= sig_hi && sig_hi < limit;
}

#endif /* __IPCC_INTERNAL_H__ */
