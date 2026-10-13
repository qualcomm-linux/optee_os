// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <platform_config.h>
#include <pta_qcom_pas.h>
#include <stddef.h>
#include <util.h>

#include "lmcu.h"
#include "lpass.h"
#include "pas_subsys.h"

static struct qcom_pas_subsys subsystems[] = {
	{
		.data = {
			.pas_id = DTB_ID_LMCU0,
		},
		.ops = &lmcu0_dtb_ops,
		.reset_seq = QCOM_PAS_RESET_NONE,
	},
	{
		.data = {
			.pas_id = DTB_ID_LMCU1,
		},
		.ops = &lmcu1_dtb_ops,
		.reset_seq = QCOM_PAS_RESET_NONE,
	},
	{
		.data = {
			.pas_id = PAS_ID_LPASS,
			.base.pa = LPASS_BASE,
			.size = LPASS_SIZE,
			.clk_group = QCOM_CLKS_LPASS,
		},
		.ops = &lpass_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
	{
		.data = {
			.pas_id = PAS_ID_LMCU0,
			.base.pa = LPASS_BASE,
			.size = LPASS_SIZE,
			.clk_group = QCOM_CLKS_LMCU0,
		},
		.ops = &lmcu0_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
	{
		.data = {
			.pas_id = PAS_ID_LMCU1,
			.base.pa = LPASS_BASE,
			.size = LPASS_SIZE,
			.clk_group = QCOM_CLKS_LMCU1,
		},
		.ops = &lmcu1_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
};

struct qcom_pas_subsys *qcom_pas_platform_subsys(size_t *count)
{
	*count = ARRAY_SIZE(subsystems);

	return subsystems;
}
