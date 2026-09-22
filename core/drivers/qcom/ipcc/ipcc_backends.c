/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include "ipcc_internal.h"

/* Router mode backend operations - function implementations in ipcc_router.c */
extern TEE_Result ipcc_router_attach(struct ipcc_handle *h);
extern void ipcc_router_detach(struct ipcc_handle *h);
extern TEE_Result ipcc_router_register_interrupt(struct ipcc_handle *h,
						 enum ipcc_client sender_id,
						 uint16_t signal_low,
						 uint16_t signal_high,
						 ipcc_callback_t cb, void *cb_data);
extern TEE_Result ipcc_router_deregister_interrupt(struct ipcc_handle *h,
						   enum ipcc_client sender_id,
						   uint16_t signal_low,
						   uint16_t signal_high);
extern TEE_Result ipcc_router_signal(struct ipcc_handle *h,
				     enum ipcc_client target_id,
				     uint16_t signal_low, uint16_t signal_high);

/* Direct mode backend operations - function implementations in ipcc_direct.c */
extern TEE_Result ipcc_direct_attach(struct ipcc_handle *h);
extern void ipcc_direct_detach(struct ipcc_handle *h);
extern TEE_Result ipcc_direct_register_interrupt(struct ipcc_handle *h,
						 enum ipcc_client sender_id,
						 uint16_t signal_low,
						 uint16_t signal_high,
						 ipcc_callback_t cb, void *cb_data);
extern TEE_Result ipcc_direct_deregister_interrupt(struct ipcc_handle *h,
						   enum ipcc_client sender_id,
						   uint16_t signal_low,
						   uint16_t signal_high);
extern TEE_Result ipcc_direct_signal(struct ipcc_handle *h,
				     enum ipcc_client target_id,
				     uint16_t signal_low, uint16_t signal_high);

/* Router backend operations structure - assign function pointers */
const struct ipcc_backend_ops ipcc_router_ops = {
	.attach = ipcc_router_attach,
	.detach = ipcc_router_detach,
	.register_interrupt = ipcc_router_register_interrupt,
	.deregister_interrupt = ipcc_router_deregister_interrupt,
	.signal = ipcc_router_signal,
};

/* Direct backend operations structure - assign function pointers */
const struct ipcc_backend_ops ipcc_direct_ops = {
	.attach = ipcc_direct_attach,
	.detach = ipcc_direct_detach,
	.register_interrupt = ipcc_direct_register_interrupt,
	.deregister_interrupt = ipcc_direct_deregister_interrupt,
	.signal = ipcc_direct_signal,
};
