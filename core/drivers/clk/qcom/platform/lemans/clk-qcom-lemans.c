// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Per-target QUP serial-engine clock BSP for Lemans (Hoya family).
 *
 * CMD_RCGR offsets are stored GCC-relative: GCC_CLK_CTL_REG_REG_BASE equals
 * the OP-TEE GCC_BASE, so the walker resolves each register as
 * gcc_base + offset.
 *
 * vote_reg_offset/vote_bit are sourced from the target's vote-register
 * bit-field layout, cross-checked against the reference QUP_SE_CLOCK()
 * index.
 */

#include <drivers/clk_qcom_bsp.h>
#include <util.h>

/* Resolved RCG SRC_SEL mux indices. */
#define MUX_XO			0
#define MUX_GPLL0_DIV2		6
#define MUX_GPLL4		5

/* QUP SE RCGs expose 8 DFS performance states. */
#define QUP_SE_DFS_STATES	8

/*
 * CX/MX voltage corners (raw rail_voltage_level) each rate requires. The
 * walker maps the corner to the rail's RPMh command ordinal at runtime.
 */
#define CX_MIN_SVS		0x30
#define CX_LOW_SVS		0x40
#define CX_SVS			0x80

/*
 * 120 MHz-capable QUP SE frequency plan.
 * Columns: freq_hz, mux_sel, div2x, m, n, dfs_idx, cx_level.
 */
static const struct qcom_clk_mux_config qup_se_120mhz[] = {
	{   7372800, MUX_GPLL0_DIV2, 2,  384, 15625, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  14745600, MUX_GPLL0_DIV2, 2,  768, 15625, 0x00, CX_MIN_SVS },
	{  19200000, MUX_XO,         2,    0,     0, 0x01, CX_MIN_SVS },
	{  29491200, MUX_GPLL0_DIV2, 2, 1536, 15625, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  32000000, MUX_GPLL0_DIV2, 2,    8,    75, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  48000000, MUX_GPLL0_DIV2, 2,    4,    25, 0x02, CX_MIN_SVS },
	{  51200000, MUX_GPLL0_DIV2, 2,   64,   375, 0x03, CX_MIN_SVS },
	{  64000000, MUX_GPLL0_DIV2, 2,   16,    75, 0x04, CX_LOW_SVS },
	{  75000000, MUX_GPLL0_DIV2, 8,    0,     0, 0x05, CX_SVS },
	{  80000000, MUX_GPLL0_DIV2, 2,    4,    15, QCOM_CLK_DFS_IDX_NONE, CX_SVS },
	{  96000000, MUX_GPLL0_DIV2, 2,    8,    25, QCOM_CLK_DFS_IDX_NONE, CX_SVS },
	{ 100000000, MUX_GPLL0_DIV2, 6,    0,     0, 0x06, CX_SVS },
	{ 120000000, MUX_GPLL0_DIV2, 5,    0,     0, 0x07, CX_SVS },
};

/*
 * 100 MHz-capable QUP SE frequency plan.
 * Identical to the 120 MHz plan minus its top (120 MHz) row.
 */
static const struct qcom_clk_mux_config qup_se_100mhz[] = {
	{   7372800, MUX_GPLL0_DIV2, 2,  384, 15625, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  14745600, MUX_GPLL0_DIV2, 2,  768, 15625, 0x00, CX_MIN_SVS },
	{  19200000, MUX_XO,         2,    0,     0, 0x01, CX_MIN_SVS },
	{  29491200, MUX_GPLL0_DIV2, 2, 1536, 15625, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  32000000, MUX_GPLL0_DIV2, 2,    8,    75, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  48000000, MUX_GPLL0_DIV2, 2,    4,    25, 0x02, CX_MIN_SVS },
	{  51200000, MUX_GPLL0_DIV2, 2,   64,   375, 0x03, CX_MIN_SVS },
	{  64000000, MUX_GPLL0_DIV2, 2,   16,    75, 0x04, CX_LOW_SVS },
	{  75000000, MUX_GPLL0_DIV2, 8,    0,     0, 0x05, CX_SVS },
	{  80000000, MUX_GPLL0_DIV2, 2,    4,    15, QCOM_CLK_DFS_IDX_NONE, CX_SVS },
	{  96000000, MUX_GPLL0_DIV2, 2,    8,    25, QCOM_CLK_DFS_IDX_NONE, CX_SVS },
	{ 100000000, MUX_GPLL0_DIV2, 6,    0,     0, 0x06, CX_SVS },
};

/*
 * 403.2 MHz-capable QUP SE frequency plan.
 * The 51.2 MHz row is absent in this plan; the DFS index assignment differs
 * from the 100/120 MHz plans, and the top row is sourced from GPLL4.
 */
static const struct qcom_clk_mux_config qup_se_403mhz[] = {
	{   7372800, MUX_GPLL0_DIV2, 2,  384, 15625, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  14745600, MUX_GPLL0_DIV2, 2,  768, 15625, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  19200000, MUX_XO,         2,    0,     0, 0x00, CX_MIN_SVS },
	{  29491200, MUX_GPLL0_DIV2, 2, 1536, 15625, QCOM_CLK_DFS_IDX_NONE, CX_MIN_SVS },
	{  32000000, MUX_GPLL0_DIV2, 2,    8,    75, 0x01, CX_MIN_SVS },
	{  48000000, MUX_GPLL0_DIV2, 2,    4,    25, 0x02, CX_MIN_SVS },
	{  64000000, MUX_GPLL0_DIV2, 2,   16,    75, 0x03, CX_LOW_SVS },
	{  75000000, MUX_GPLL0_DIV2, 8,    0,     0, QCOM_CLK_DFS_IDX_NONE, CX_SVS },
	{  80000000, MUX_GPLL0_DIV2, 2,    4,    15, QCOM_CLK_DFS_IDX_NONE, CX_SVS },
	{  96000000, MUX_GPLL0_DIV2, 2,    8,    25, 0x04, CX_SVS },
	{ 100000000, MUX_GPLL0_DIV2, 6,    0,     0, 0x05, CX_SVS },
	{ 403200000, MUX_GPLL4,      4,    0,     0, 0x06, CX_SVS },
};

/*
 * On QUPv3 the SE's branch control register (CBCR) is fixed at 8 bytes below
 * its CMD_RCGR (GCC_QUPV3_WRAP*_S*_CBCR == the SE's GCC_QUPV3_WRAP*_S*_CMD_RCGR
 * - 8 for every wrapper/SE). Derive it here rather than carrying a second
 * offset column.
 */
#define QUP_SE_CBCR_FROM_RCGR(_cmd_rcgr)	((_cmd_rcgr) - 8)

/*
 * Every QUP SE branch on this target is gated through a shared vote register
 * rather than its own CBCR's CLK_ENABLE bit. Offsets/bits below are
 * GCC-relative; vote register/bit assignment is per-target and per-SE, so it
 * is listed explicitly per domain below.
 */
#define GCC_CLOCK_BRANCH_ENA_VOTE	0x3e040
#define GCC_CLOCK_BRANCH_ENA_VOTE_1	0x3e048
#define GCC_CLOCK_BRANCH_ENA_VOTE_2	0x3e050
#define GCC_CLOCK_BRANCH_ENA_VOTE_3	0x3e058

/*
 * PLL source-vote table, keyed by @mux_sel; XO (MUX_XO) has no entry and is
 * not voted. Direct GCC write, not RPMh.
 */
#define GCC_PLL_BRANCH_ENA_VOTE		0x3e068
#define GCC_PLL_VOTE_BIT_GPLL0		0
#define GCC_PLL_VOTE_BIT_GPLL4		4

static const struct qcom_clk_src_vote qup_se_src_votes[] = {
	/* GPLL0_DIV2 is the /2 output of GPLL0; voting GPLL0 holds it on. */
	{ MUX_GPLL0_DIV2, GCC_PLL_BRANCH_ENA_VOTE, GCC_PLL_VOTE_BIT_GPLL0 },
	{ MUX_GPLL4,      GCC_PLL_BRANCH_ENA_VOTE, GCC_PLL_VOTE_BIT_GPLL4 },
};

#define QUP_SE_DOMAIN(_name, _cmd_rcgr, _plan, _vote_reg, _vote_bit)	\
	{								\
		.name = (_name),					\
		.cmd_rcgr_offset = (_cmd_rcgr),				\
		.cbcr_offset = QUP_SE_CBCR_FROM_RCGR(_cmd_rcgr),	\
		.vote_reg_offset = (_vote_reg),			\
		.vote_bit = (_vote_bit),				\
		.dfs_states = QUP_SE_DFS_STATES,			\
		.configs = (_plan),					\
		.n_configs = ARRAY_SIZE(_plan),				\
	}

static const struct qcom_clk_domain qup_se_domains[] = {
	/* QUPv3 wrapper 0: S0-S1 at 120 MHz, S2-S6 at 100 MHz. All VOTE_1. */
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s0_clk", 0x13154, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 10),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s1_clk", 0x13288, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 11),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s2_clk", 0x133bc, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 12),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s3_clk", 0x134f0, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 13),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s4_clk", 0x13624, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 14),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s5_clk", 0x13758, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 15),
	QUP_SE_DOMAIN("gcc_qupv3_wrap0_s6_clk", 0x1388c, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 16),

	/*
	 * QUPv3 wrapper 1: S0-S1 at 120 MHz, S2-S6 at 100 MHz. S0-S5 VOTE_1,
	 * S6 VOTE_3. S3 is the console UART's QUP SE and must never be
	 * enabled/disabled/set-rate by this driver or its self-test.
	 */
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s0_clk", 0x14154, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 22),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s1_clk", 0x14288, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 23),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s2_clk", 0x143bc, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 24),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s3_clk", 0x144f0, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 25),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s4_clk", 0x14624, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 26),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s5_clk", 0x14758, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_1, 27),
	QUP_SE_DOMAIN("gcc_qupv3_wrap1_s6_clk", 0x1488c, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_3, 27),

	/* QUPv3 wrap2: S0-S1 120MHz, S2-S6 100MHz. S0-S5 VOTE_2, S6 VOTE_3. */
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s0_clk", 0x1a154, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, 4),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s1_clk", 0x1a288, qup_se_120mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, 5),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s2_clk", 0x1a3bc, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, 6),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s3_clk", 0x1a4f0, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, 7),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s4_clk", 0x1a624, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, 8),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s5_clk", 0x1a758, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_2, 9),
	QUP_SE_DOMAIN("gcc_qupv3_wrap2_s6_clk", 0x1a88c, qup_se_100mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE_3, 29),

	/* QUPv3 wrapper 3: S0 only, at 403.2 MHz. VOTE (base). */
	QUP_SE_DOMAIN("gcc_qupv3_wrap3_s0_clk", 0xb4154, qup_se_403mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE, 25),
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
