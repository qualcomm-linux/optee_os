// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Per-target QUP clock BSP for Lemans (Hoya family): the per-SE serial-
 * engine RCGs plus the QUPv3 wrapper-level gating clocks (core/core_2x/
 * m_ahb/s_ahb).
 *
 * All register locations are full physical addresses composed from GCC_BASE;
 * the walker resolves each against the domain's window.
 */

#include <drivers/clk_qcom_bsp.h>
#include <platform_config.h>
#include <util.h>

#include "clock_group_qcom.h"

/* Resolved RCG SRC_SEL mux indices. */
#define MUX_XO			0
#define MUX_GPLL0_DIV2		6
#define MUX_GPLL4		5

#define QUP_SE_DFS_STATES	8

/* CX/MX voltage corners (raw rail_voltage_level) each rate requires. */
#define CX_MIN_SVS		0x30
#define CX_LOW_SVS		0x40
#define CX_SVS			0x80

/*
 * 120 MHz-capable QUP SE frequency plan.
 * Columns: freq_hz, mux_sel, div2x, m, n, dfs_idx, cx_level.
 */
static const struct qcom_clk_mux_config qup_se_120mhz[] = {
	{   7372800, MUX_GPLL0_DIV2, 2,  384, 15625, QCOM_DFS_NA, CX_MIN_SVS },
	{  14745600, MUX_GPLL0_DIV2, 2,  768, 15625, 0x00, CX_MIN_SVS },
	{  19200000, MUX_XO,         2,    0,     0, 0x01, CX_MIN_SVS },
	{  29491200, MUX_GPLL0_DIV2, 2, 1536, 15625, QCOM_DFS_NA, CX_MIN_SVS },
	{  32000000, MUX_GPLL0_DIV2, 2,    8,    75, QCOM_DFS_NA, CX_MIN_SVS },
	{  48000000, MUX_GPLL0_DIV2, 2,    4,    25, 0x02, CX_MIN_SVS },
	{  51200000, MUX_GPLL0_DIV2, 2,   64,   375, 0x03, CX_MIN_SVS },
	{  64000000, MUX_GPLL0_DIV2, 2,   16,    75, 0x04, CX_LOW_SVS },
	{  75000000, MUX_GPLL0_DIV2, 8,    0,     0, 0x05, CX_SVS },
	{  80000000, MUX_GPLL0_DIV2, 2,    4,    15, QCOM_DFS_NA, CX_SVS },
	{  96000000, MUX_GPLL0_DIV2, 2,    8,    25, QCOM_DFS_NA, CX_SVS },
	{ 100000000, MUX_GPLL0_DIV2, 6,    0,     0, 0x06, CX_SVS },
	{ 120000000, MUX_GPLL0_DIV2, 5,    0,     0, 0x07, CX_SVS },
};

/*
 * 100 MHz-capable QUP SE frequency plan.
 * Identical to the 120 MHz plan minus its top (120 MHz) row.
 */
static const struct qcom_clk_mux_config qup_se_100mhz[] = {
	{   7372800, MUX_GPLL0_DIV2, 2,  384, 15625, QCOM_DFS_NA, CX_MIN_SVS },
	{  14745600, MUX_GPLL0_DIV2, 2,  768, 15625, 0x00, CX_MIN_SVS },
	{  19200000, MUX_XO,         2,    0,     0, 0x01, CX_MIN_SVS },
	{  29491200, MUX_GPLL0_DIV2, 2, 1536, 15625, QCOM_DFS_NA, CX_MIN_SVS },
	{  32000000, MUX_GPLL0_DIV2, 2,    8,    75, QCOM_DFS_NA, CX_MIN_SVS },
	{  48000000, MUX_GPLL0_DIV2, 2,    4,    25, 0x02, CX_MIN_SVS },
	{  51200000, MUX_GPLL0_DIV2, 2,   64,   375, 0x03, CX_MIN_SVS },
	{  64000000, MUX_GPLL0_DIV2, 2,   16,    75, 0x04, CX_LOW_SVS },
	{  75000000, MUX_GPLL0_DIV2, 8,    0,     0, 0x05, CX_SVS },
	{  80000000, MUX_GPLL0_DIV2, 2,    4,    15, QCOM_DFS_NA, CX_SVS },
	{  96000000, MUX_GPLL0_DIV2, 2,    8,    25, QCOM_DFS_NA, CX_SVS },
	{ 100000000, MUX_GPLL0_DIV2, 6,    0,     0, 0x06, CX_SVS },
};

/*
 * 403.2 MHz-capable QUP SE frequency plan.
 * The 51.2 MHz row is absent in this plan; the DFS index assignment differs
 * from the 100/120 MHz plans, and the top row is sourced from GPLL4.
 */
static const struct qcom_clk_mux_config qup_se_403mhz[] = {
	{   7372800, MUX_GPLL0_DIV2, 2,  384, 15625, QCOM_DFS_NA, CX_MIN_SVS },
	{  14745600, MUX_GPLL0_DIV2, 2,  768, 15625, QCOM_DFS_NA, CX_MIN_SVS },
	{  19200000, MUX_XO,         2,    0,     0, 0x00, CX_MIN_SVS },
	{  29491200, MUX_GPLL0_DIV2, 2, 1536, 15625, QCOM_DFS_NA, CX_MIN_SVS },
	{  32000000, MUX_GPLL0_DIV2, 2,    8,    75, 0x01, CX_MIN_SVS },
	{  48000000, MUX_GPLL0_DIV2, 2,    4,    25, 0x02, CX_MIN_SVS },
	{  64000000, MUX_GPLL0_DIV2, 2,   16,    75, 0x03, CX_LOW_SVS },
	{  75000000, MUX_GPLL0_DIV2, 8,    0,     0, QCOM_DFS_NA, CX_SVS },
	{  80000000, MUX_GPLL0_DIV2, 2,    4,    15, QCOM_DFS_NA, CX_SVS },
	{  96000000, MUX_GPLL0_DIV2, 2,    8,    25, 0x04, CX_SVS },
	{ 100000000, MUX_GPLL0_DIV2, 6,    0,     0, 0x05, CX_SVS },
	{ 403200000, MUX_GPLL4,      4,    0,     0, 0x06, CX_SVS },
};

/*
 * Lemans routes every QUP register -- SE RCGs, wrapper CBCRs and all the
 * shared vote registers -- through the single central GCC window, so one
 * window backs every domain below.
 */
static const struct qcom_clk_window gcc_window = {
	.pa = GCC_BASE,
	.size = GCC_SIZE,
};

/*
 * PLL source-vote table, keyed by @mux_sel; XO (MUX_XO) has no entry and is
 * not voted. Direct GCC write, not RPMh.
 */
static const struct qcom_clk_src_vote qup_se_src_votes[] = {
	/* GPLL0_DIV2 is the /2 output of GPLL0; voting GPLL0 holds it on. */
	{ &gcc_window, MUX_GPLL0_DIV2, GCC_PLL_BRANCH_ENA_VOTE,
	  GCC_PLL_VOTE_BIT_GPLL0 },
	{ &gcc_window, MUX_GPLL4, GCC_PLL_BRANCH_ENA_VOTE,
	  GCC_PLL_VOTE_BIT_GPLL4 },
};

/*
 * QUPv3 wrapper-level branch clock (core/core_2x/m_ahb/s_ahb): no RCG, no
 * frequency plan -- enable/disable only, via the same shared vote register +
 * CBCR poll qcom_qup_clk_ops already implements for the SE branches above.
 * Rate scaling for these is delegated to RPMh BCM/AOP, outside OP-TEE's
 * scope.
 */
#define QUP_BRANCH_CLK(_name, _cbcr, _vote_reg, _vote_bit)		\
	{								\
		.name = (_name),					\
		.window = &gcc_window,					\
		.cbcr_addr = (_cbcr),					\
		.vote_reg_addr = (_vote_reg),				\
		.vote_bit = (_vote_bit),				\
	}

#define QUP_SE_DOMAIN(_name, _cmd_rcgr, _cbcr, _plan, _vote_reg, _vote_bit) \
	{								\
		.name = (_name),					\
		.window = &gcc_window,					\
		.cmd_rcgr_addr = (_cmd_rcgr),				\
		.cbcr_addr = (_cbcr),					\
		.vote_reg_addr = (_vote_reg),				\
		.vote_bit = (_vote_bit),				\
		.dfs_states = QUP_SE_DFS_STATES,			\
		.configs = (_plan),					\
		.n_configs = ARRAY_SIZE(_plan),				\
	}

static const struct qcom_clk_domain qup_se_domains[] = {
	/* QUPv3 wrapper 0: S0-S1 at 120 MHz, S2-S6 at 100 MHz. All VOTE_1. */
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s0_clk",
		      GCC_QUPV3_WRAP0_S0_CMD_RCGR,
		      GCC_QUPV3_WRAP0_S0_CBCR, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_S0_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s1_clk",
		      GCC_QUPV3_WRAP0_S1_CMD_RCGR,
		      GCC_QUPV3_WRAP0_S1_CBCR, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_S1_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s2_clk",
		      GCC_QUPV3_WRAP0_S2_CMD_RCGR,
		      GCC_QUPV3_WRAP0_S2_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_S2_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s3_clk",
		      GCC_QUPV3_WRAP0_S3_CMD_RCGR,
		      GCC_QUPV3_WRAP0_S3_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_S3_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s4_clk",
		      GCC_QUPV3_WRAP0_S4_CMD_RCGR,
		      GCC_QUPV3_WRAP0_S4_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_S4_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s5_clk",
		      GCC_QUPV3_WRAP0_S5_CMD_RCGR,
		      GCC_QUPV3_WRAP0_S5_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_S5_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s6_clk",
		      GCC_QUPV3_WRAP0_S6_CMD_RCGR,
		      GCC_QUPV3_WRAP0_S6_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_S6_SHFT),

	/*
	 * QUPv3 wrapper 1: S0-S1 at 120 MHz, S2-S6 at 100 MHz. S0-S5 VOTE_1,
	 * S6 VOTE_3.
	 */
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s0_clk",
		      GCC_QUPV3_WRAP1_S0_CMD_RCGR,
		      GCC_QUPV3_WRAP1_S0_CBCR, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP1_S0_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s1_clk",
		      GCC_QUPV3_WRAP1_S1_CMD_RCGR,
		      GCC_QUPV3_WRAP1_S1_CBCR, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP1_S1_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s2_clk",
		      GCC_QUPV3_WRAP1_S2_CMD_RCGR,
		      GCC_QUPV3_WRAP1_S2_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP1_S2_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s3_clk",
		      GCC_QUPV3_WRAP1_S3_CMD_RCGR,
		      GCC_QUPV3_WRAP1_S3_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP1_S3_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s4_clk",
		      GCC_QUPV3_WRAP1_S4_CMD_RCGR,
		      GCC_QUPV3_WRAP1_S4_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP1_S4_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s5_clk",
		      GCC_QUPV3_WRAP1_S5_CMD_RCGR,
		      GCC_QUPV3_WRAP1_S5_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP1_S5_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s6_clk",
		      GCC_QUPV3_WRAP1_S6_CMD_RCGR,
		      GCC_QUPV3_WRAP1_S6_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_3, QUPV3_WRAP1_S6_SHFT),

	/* QUPv3 wrap2: S0-S1 120MHz, S2-S6 100MHz. S0-S5 VOTE_2, S6 VOTE_3. */
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s0_clk",
		      GCC_QUPV3_WRAP2_S0_CMD_RCGR,
		      GCC_QUPV3_WRAP2_S0_CBCR, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP2_S0_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s1_clk",
		      GCC_QUPV3_WRAP2_S1_CMD_RCGR,
		      GCC_QUPV3_WRAP2_S1_CBCR, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP2_S1_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s2_clk",
		      GCC_QUPV3_WRAP2_S2_CMD_RCGR,
		      GCC_QUPV3_WRAP2_S2_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP2_S2_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s3_clk",
		      GCC_QUPV3_WRAP2_S3_CMD_RCGR,
		      GCC_QUPV3_WRAP2_S3_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP2_S3_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s4_clk",
		      GCC_QUPV3_WRAP2_S4_CMD_RCGR,
		      GCC_QUPV3_WRAP2_S4_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP2_S4_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s5_clk",
		      GCC_QUPV3_WRAP2_S5_CMD_RCGR,
		      GCC_QUPV3_WRAP2_S5_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP2_S5_SHFT),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s6_clk",
		      GCC_QUPV3_WRAP2_S6_CMD_RCGR,
		      GCC_QUPV3_WRAP2_S6_CBCR, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_3, QUPV3_WRAP2_S6_SHFT),

	/* QUPv3 wrapper 3: S0 only, at 403.2 MHz. VOTE (base). */
	QUP_SE_DOMAIN("gcc_qupv3_wrap3_s0_clk",
		      GCC_QUPV3_WRAP3_S0_CMD_RCGR,
		      GCC_QUPV3_WRAP3_S0_CBCR, qup_se_403mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP3_S0_SHFT),

	/* QUPv3 wrapper-level branch clocks: enable/disable only, no RCG. */
	QUP_BRANCH_CLK("gcc_qupv3_wrap0_core_2x_clk",
		       GCC_QUPV3_WRAP0_CORE_2X_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_CORE_2X_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap0_core_clk",
		       GCC_QUPV3_WRAP0_CORE_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP0_CORE_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap_0_m_ahb_clk",
		       GCC_QUPV3_WRAP_0_M_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP_0_M_AHB_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap_0_s_ahb_clk",
		       GCC_QUPV3_WRAP_0_S_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP_0_S_AHB_SHFT),

	QUP_BRANCH_CLK("gcc_qupv3_wrap1_core_2x_clk",
		       GCC_QUPV3_WRAP1_CORE_2X_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP1_CORE_2X_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap1_core_clk",
		       GCC_QUPV3_WRAP1_CORE_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP1_CORE_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap_1_m_ahb_clk",
		       GCC_QUPV3_WRAP_1_M_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP_1_M_AHB_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap_1_s_ahb_clk",
		       GCC_QUPV3_WRAP_1_S_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_1, QUPV3_WRAP_1_S_AHB_SHFT),

	QUP_BRANCH_CLK("gcc_qupv3_wrap2_core_2x_clk",
		       GCC_QUPV3_WRAP2_CORE_2X_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP2_CORE_2X_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap2_core_clk",
		       GCC_QUPV3_WRAP2_CORE_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP2_CORE_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap_2_m_ahb_clk",
		       GCC_QUPV3_WRAP_2_M_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP_2_M_AHB_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap_2_s_ahb_clk",
		       GCC_QUPV3_WRAP_2_S_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP_2_S_AHB_SHFT),

	QUP_BRANCH_CLK("gcc_qupv3_wrap3_core_2x_clk",
		       GCC_QUPV3_WRAP3_CORE_2X_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP3_CORE_2X_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap3_core_clk",
		       GCC_QUPV3_WRAP3_CORE_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP3_CORE_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap_3_m_ahb_clk",
		       GCC_QUPV3_WRAP_3_M_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP_3_M_AHB_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap_3_s_ahb_clk",
		       GCC_QUPV3_WRAP_3_S_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP_3_S_AHB_SHFT),
};

static const struct qcom_clk_bsp lemans_clk_bsp = {
	.domains = qup_se_domains,
	.n_domains = ARRAY_SIZE(qup_se_domains),
	.src_votes = qup_se_src_votes,
	.n_src_votes = ARRAY_SIZE(qup_se_src_votes),
};

const struct qcom_clk_bsp *qcom_clk_bsp_get(void)
{
	return &lemans_clk_bsp;
}
