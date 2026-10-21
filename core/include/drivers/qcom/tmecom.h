/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __DRIVERS_QCOM_TMECOM_H
#define __DRIVERS_QCOM_TMECOM_H

#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>

/*
 * Transport to the Trust Management Engine (TME). Messages are identified by
 * a tag; the transport owns framing, integrity and mailbox handling.
 *
 * @msg_tag:	TME message tag
 * @req:	request payload, may be NULL when @req_len is 0
 * @req_len:	size of the request payload in bytes
 * @rsp:	buffer receiving the response payload
 * @rsp_cap:	size of @rsp in bytes
 * @rsp_len:	[out] size of the response payload actually received
 *
 * Returns TEE_SUCCESS if the exchange completed. The payload status carried
 * by the response is interpreted by the caller.
 */
TEE_Result tmecom_transceive(uint32_t msg_tag, const void *req, size_t req_len,
			     void *rsp, size_t rsp_cap, size_t *rsp_len);

#endif /* __DRIVERS_QCOM_TMECOM_H */
