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
 * Router: one client page and one shared interrupt per protocol.
 * Direct: one interrupt line and trigger mask per (client, signal).
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
 * HAS_TME: TME programs IPC_CONFIG/IPC_TRACE before OP-TEE (Wildcat onwards)
 * ROUTER_MODE: router mode, otherwise direct (legacy)
 * HW_MEM_OPT: client pages indexed by table row, not client ID
 */
#define IPCC_CFG_HAS_TME      BIT32(0)
#define IPCC_CFG_ROUTER_MODE  BIT32(1)
#define IPCC_CFG_HW_MEM_OPT   BIT32(2)

/* Clients a chipset instantiates; unlisted clients are rejected */
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
	TEE_Result (*trigger)(struct ipcc_handle *h, enum ipcc_client target_id,
			      uint16_t signal_low, uint16_t signal_high);
};

/* A protocol slot without a client table is not described */
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
	/* Router mode only */
	bool has_router;
	uint32_t interrupt;
	bool clear_on_recv_en;
};

struct ipcc_cfg {
	const struct ipcc_proto_cfg *protocols;
	uint32_t num_protocols;
	/* Client ID of this TEE */
	enum ipcc_client client;
	/* Router mode without TME: holds IPC_CONFIG/IPC_TRACE */
	paddr_t ctrl_block_base;
	bool has_ctrl_block;
	uint32_t flags;
};

/* Per-protocol Rx state */
struct ipcc_rx_drv_info {
	struct mutex lock;		/* Protects refs */
	unsigned int list_lock;		/* Protects handles against the ISR */
	uint32_t refs;
	const struct ipcc_proto_cfg *protocol;
	struct itr_handler *itr_hdl;
	struct ipcc_handle *handles;
};

/* router_pages[] and rx_drv[] are indexed by protocol ID */
struct ipcc_drv_ctxt {
	const struct ipcc_cfg *cfg;
	vaddr_t router_pages[IPCC_PROTO_TOTAL];
	struct ipcc_rx_drv_info rx_drv[IPCC_PROTO_TOTAL];
	uint32_t hw_version;
};

const struct ipcc_drv_ctxt *ipcc_get_drv_ctxt(void);

TEE_Result ipcc_client_page_idx(const struct ipcc_proto_cfg *proto,
				enum ipcc_client client, uint32_t *idx);

/* Router mode: signal range registered by a handle */
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
	struct ipcc_signal_range *ranges;
	struct ipcc_handle *next;
};

extern const struct ipcc_backend_ops ipcc_router_ops;
extern const struct ipcc_backend_ops ipcc_direct_ops;

/* No Rx interrupt line for this signal */
#define IPCC_DIRECT_INT_NONE		0xffffffffU

/*
 * interrupt_id/out_mask come from the chipset table, out_mask == 0 means
 * Tx is not possible. The remaining fields are runtime state.
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

/* signals[] by signal no., clients[] by client ID; trigger_reg 0 = Rx only */
struct ipcc_direct_client {
	uint32_t num_signals;
	struct ipcc_direct_signal *signals;
	paddr_t trigger_reg;
};

struct ipcc_direct_cfg {
	uint32_t num_clients;
	struct ipcc_direct_client *clients;
};

/* Provided by <chipset>/ipcc_config.c */
extern const struct ipcc_cfg *const ipcc_chipset_cfg;
extern const struct ipcc_direct_cfg *const ipcc_chipset_direct_cfg;

static inline bool ipcc_sig_range_valid(uint16_t sig_lo, uint16_t sig_hi,
					uint32_t limit)
{
	return sig_lo <= sig_hi && sig_hi < limit;
}

#endif /* __IPCC_INTERNAL_H__ */
