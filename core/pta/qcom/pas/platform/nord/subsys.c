// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <compiler.h>
#include <pta_qcom_pas.h>
#include <stddef.h>
#include <util.h>

#include "pas_subsys.h"

/*
 * Bring-up (fw_start/fw_shutdown) for these subsystems is not yet ported: the
 * boot-control register layout and clock sequencing on this chip have not
 * been verified against the kodiak/lemans QDSP6 and IRIS bring-up code they
 * would otherwise be modelled on. Until that is done, every operation is
 * explicitly NOT_IMPLEMENTED rather than reusing unverified register offsets.
 */
static TEE_Result nord_fw_not_implemented(struct qcom_pas_data *data __unused)
{
	return TEE_ERROR_NOT_IMPLEMENTED;
}

static const struct qcom_pas_ops nord_stub_ops = {
	.fw_start = nord_fw_not_implemented,
	.fw_shutdown = nord_fw_not_implemented,
};

static struct qcom_pas_subsys subsystems[] = {
	{
		.data = {
			.pas_id = PAS_ID_QDSP6,
			.base.pa = HPASS_QDSP6_BASE,
			.size = HPASS_QDSP6_SIZE,
		},
		.ops = &nord_stub_ops,
		.reset_seq = QCOM_PAS_RESET_NONE,
	},
	{
		.data = {
			.pas_id = PAS_ID_IRIS,
			.base.pa = IRIS_BASE,
			.size = IRIS_SIZE,
		},
		.ops = &nord_stub_ops,
		.reset_seq = QCOM_PAS_RESET_NONE,
	},
};

struct qcom_pas_subsys *qcom_pas_platform_subsys(size_t *count)
{
	*count = ARRAY_SIZE(subsystems);

	return subsystems;
}
