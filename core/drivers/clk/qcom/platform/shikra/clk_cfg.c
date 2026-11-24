// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <platform_config.h>
#include <util.h>

#include "clk_cfg.h"
#include "clock_group.h"

static struct clk_regmap gcc_regmap = {
	.io.pa = GCC_BASE,
	.size = GCC_SIZE,
};

static const struct clk_pll_vote pll_votes[] = {
	{ "xo", NULL, 0, 0 },
};

#define BRANCH_CLK(_name, _cbcr)				\
	{							\
		.name = (_name),				\
		.regmap = &gcc_regmap,				\
		.cbcr_addr = GCC_BASE + (_cbcr),		\
	}

static const struct clk_branch_desc branches[] = {
	BRANCH_CLK("gcc_mss_q6_ddrss_axi_clk", GCC_MSS_Q6_DDRSS_AXI_CBCR),
	BRANCH_CLK("gcc_snoc_memnoc_sf_clk", GCC_SNOC_MEMNOC_SF_CBCR),
};

static const struct clk_cfg shikra_clk_cfg = {
	.branches = branches,
	.n_branches = ARRAY_SIZE(branches),
	.pll_votes = pll_votes,
	.n_pll_votes = ARRAY_SIZE(pll_votes),
};

const struct clk_cfg *clk_get_cfg(void)
{
	return &shikra_clk_cfg;
}
