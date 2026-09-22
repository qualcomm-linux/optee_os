/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/qcom/ipcc/ipcc.h>

/*
 * Stub implementation for systems without IPCC. All entry points fail with
 * TEE_ERROR_NOT_SUPPORTED so that callers can link either way.
 */

TEE_Result ipcc_attach(struct ipcc_handle **h __unused,
		       enum ipcc_protocol protocol __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

TEE_Result ipcc_detach(struct ipcc_handle **h __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

TEE_Result ipcc_register_interrupt(struct ipcc_handle *h __unused,
				   enum ipcc_client sender_id __unused,
				   uint16_t signal_low __unused,
				   uint16_t signal_high __unused,
				   ipcc_callback_t cb __unused,
				   void *cb_data __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

TEE_Result ipcc_deregister_interrupt(struct ipcc_handle *h __unused,
				     enum ipcc_client sender_id __unused,
				     uint16_t signal_low __unused,
				     uint16_t signal_high __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

TEE_Result ipcc_trigger(struct ipcc_handle *h __unused,
		       enum ipcc_client target_id __unused,
		       uint16_t signal_low __unused,
		       uint16_t signal_high __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

