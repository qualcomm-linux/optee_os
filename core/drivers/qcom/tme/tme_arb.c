// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <compiler.h>
#include <drivers/qcom/tme_arb.h>
#include <drivers/qcom/tmecom.h>
#include <inttypes.h>
#include <trace.h>

#define TME_MSG_TAG_UPDATE_ROLLBACK_VERSION	307
#define TME_MSG_TAG_GET_ARB_VERSION		328

struct tme_arb_version_req {
	uint32_t sw_id;
} __packed;

struct tme_arb_version_rsp {
	uint8_t oem_version;
	uint8_t qti_version;
	uint8_t oem_valid;
	uint8_t qti_valid;
} __packed;

/* The update request carries no payload */
struct tme_update_rollback_rsp {
	uint32_t status;
	uint32_t err_addr;
} __packed;

TEE_Result tme_get_arb_oem_version(uint32_t sw_id, uint32_t *version,
				   bool *valid)
{
	struct tme_arb_version_req req = { .sw_id = sw_id };
	struct tme_arb_version_rsp rsp = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	size_t rsp_len = 0;

	if (!version || !valid)
		return TEE_ERROR_BAD_PARAMETERS;

	res = tmecom_transceive(TME_MSG_TAG_GET_ARB_VERSION, &req, sizeof(req),
				&rsp, sizeof(rsp), &rsp_len);
	if (res) {
		EMSG("TME: ARB query for %#"PRIx32" failed: %#"PRIx32, sw_id, res);
		return res;
	}

	if (rsp_len != sizeof(rsp)) {
		EMSG("TME: unexpected ARB version response size %zu", rsp_len);
		return TEE_ERROR_BAD_FORMAT;
	}

	*valid = rsp.oem_valid;
	*version = rsp.oem_version;

	return TEE_SUCCESS;
}

TEE_Result tme_update_rollback_version(void)
{
	struct tme_update_rollback_rsp rsp = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	size_t rsp_len = 0;

	res = tmecom_transceive(TME_MSG_TAG_UPDATE_ROLLBACK_VERSION, NULL, 0,
				&rsp, sizeof(rsp), &rsp_len);
	if (res) {
		EMSG("TME: rollback version update failed: %#"PRIx32, res);
		return res;
	}

	if (rsp_len != sizeof(rsp)) {
		EMSG("TME: unexpected rollback update response size %zu",
		     rsp_len);
		return TEE_ERROR_BAD_FORMAT;
	}

	if (rsp.status) {
		EMSG("TME: rollback version update status %#"PRIx32
		     ", failing fuse %#"PRIx32, rsp.status, rsp.err_addr);
		return TEE_ERROR_GENERIC;
	}

	return TEE_SUCCESS;
}
