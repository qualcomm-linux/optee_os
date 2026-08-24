// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <platform_config.h>
#include <pta_qcom_pas.h>
#include <stddef.h>
#include <util.h>

#include "iris.h"
#include "nspss0.h"
#include "nspss1.h"
#include "nspss2.h"
#include "nspss3.h"
#include "hpass0.h"
#include "hpass1.h"
#include "hpass2.h"
#include "pas_subsys.h"

static struct qcom_pas_subsys subsystems[] = {
	{
		.data = {
			.pas_id = PAS_ID_IRIS,
			.base.pa = IRIS_BASE,
			.size = IRIS_SIZE,
		},
		.ops = &iris_ops,
		.reset_seq = QCOM_PAS_RESET_NONE,
	},
	{
		.data = {
			.pas_id = PAS_ID_TURING,
			.base.pa = CDSP_0_BASE,
			.size = CDSP_0_SIZE,
			.clk_group = QCOM_CLKS_TURING,
		},
		.ops = &nspss0_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
	{
		.data = {
			.pas_id = PAS_ID_TURING1,
			.base.pa = CDSP_1_BASE,
			.size = CDSP_1_SIZE,
			.clk_group = QCOM_CLKS_TURING1,
		},
		.ops = &nspss1_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
	{
		.data = {
			.pas_id = PAS_ID_TURING2,
			.base.pa = CDSP_2_BASE,
			.size = CDSP_2_SIZE,
			.clk_group = QCOM_CLKS_TURING2,
		},
		.ops = &nspss2_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
	{
		.data = {
			.pas_id = PAS_ID_TURING3,
			.base.pa = CDSP_3_BASE,
			.size = CDSP_3_SIZE,
			.clk_group = QCOM_CLKS_TURING3,
		},
		.ops = &nspss3_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
	{
		.data = {
			.pas_id = PAS_ID_QDSP6,
			.base.pa = HPASS_0_BASE,
			.size = HPASS_0_SIZE,
			.clk_group = QCOM_CLKS_HPASS0,
		},
		.ops = &hpass0_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
	{
		.data = {
			.pas_id = PAS_ID_HPASS1,
			.base.pa = HPASS_1_BASE,
			.size = HPASS_1_SIZE,
			.clk_group = QCOM_CLKS_HPASS1,
		},
		.ops = &hpass1_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
	{
		.data = {
			.pas_id = PAS_ID_HPASS2,
			.base.pa = HPASS_2_BASE,
			.size = HPASS_2_SIZE,
			.clk_group = QCOM_CLKS_HPASS2,
		},
		.ops = &hpass2_ops,
		.reset_seq = QCOM_PAS_RESET_CLK_FULL,
	},
};

struct qcom_pas_subsys *qcom_pas_platform_subsys(size_t *count)
{
	*count = ARRAY_SIZE(subsystems);

	return subsystems;
}
