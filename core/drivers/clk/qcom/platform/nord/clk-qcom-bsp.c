// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Per-target QUP clock BSP for Nord (Wildcat family): the per-SE serial-engine
 * RCGs plus the QUPv3 wrapper-level gating clocks (core/core_2x/m_ahb/s_ahb).
 *
 * Nord spreads its 22 QUP SEs across three clock controllers -- the SE and NE
 * quadrant controllers (wrappers 0/1 and 2) and the central GCC (wrapper 3) --
 * so every domain names the register window its addresses fall inside instead
 * of assuming a single GCC base. Each quadrant controller has its own GPLL0 and
 * its own vote registers.
 */

#include <drivers/clk_qcom_bsp.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <util.h>

#include "clock_group_qcom.h"

/*
 * The quadrant controllers are reached only through this BSP, so their windows
 * are registered here; the central GCC window is registered by the clock
 * driver core, which needs it regardless of the QUP BSP.
 */
register_phys_mem(MEM_AREA_IO_NSEC, SE_GCC_BASE, SE_GCC_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, NE_GCC_BASE, NE_GCC_SIZE);

/*
 * Resolved RCG SRC_SEL mux indices. Every controller reaches XO at index 0.
 * On the quadrant controllers the local GPLL0 main output is index 1; the
 * central GCC exposes both the main output (1) and the /2 even output (6).
 */
#define MUX_XO				0
#define MUX_GPLL0			1
#define MUX_GPLL0_EVEN			6

#define QUP_SE_DFS_STATES		8

/* CX/MX voltage corners (raw rail_voltage_level) each rate requires. */
#define CX_SVS				0x80
#define CX_SVS_L1			0xC0

/*
 * The three register windows Nord's QUP clocks live in. NW_GCC has no QUP
 * serial engines, so it is not mapped.
 */
static const struct qcom_clk_window se_gcc_window = {
	.pa = SE_GCC_BASE,
	.size = SE_GCC_SIZE,
};

static const struct qcom_clk_window ne_gcc_window = {
	.pa = NE_GCC_BASE,
	.size = NE_GCC_SIZE,
};

static const struct qcom_clk_window gcc_window = {
	.pa = GCC_BASE,
	.size = GCC_SIZE,
};

/*
 * 120 MHz-capable QUP SE frequency plan, used by S0-S1 of every quadrant
 * wrapper. Columns: freq_hz, mux_sel, div2x, m, n, dfs_idx, cx_level.
 */
static const struct qcom_clk_mux_config qup_se_120mhz[] = {
	{   7372800, MUX_GPLL0, 2,  192, 15625, QCOM_DFS_NA, CX_SVS },
	{  14745600, MUX_GPLL0, 2,  384, 15625, QCOM_DFS_NA, CX_SVS },
	{  19200000, MUX_XO,    2,    0,     0, 0x00, CX_SVS },
	{  29491200, MUX_GPLL0, 2,  768, 15625, QCOM_DFS_NA, CX_SVS },
	{  32000000, MUX_GPLL0, 2,    4,    75, 0x01, CX_SVS },
	{  48000000, MUX_GPLL0, 2,    2,    25, 0x02, CX_SVS },
	{  51200000, MUX_GPLL0, 2,   32,   375, QCOM_DFS_NA, CX_SVS },
	{  64000000, MUX_GPLL0, 2,    8,    75, 0x03, CX_SVS },
	{  66666667, MUX_GPLL0, 18,   0,     0, 0x04, CX_SVS },
	{  75000000, MUX_GPLL0, 16,   0,     0, QCOM_DFS_NA, CX_SVS },
	{  80000000, MUX_GPLL0, 2,    2,    15, QCOM_DFS_NA, CX_SVS_L1 },
	{  96000000, MUX_GPLL0, 2,    4,    25, 0x05, CX_SVS_L1 },
	{ 100000000, MUX_GPLL0, 12,   0,     0, 0x06, CX_SVS_L1 },
	{ 102400000, MUX_GPLL0, 2,   64,   375, QCOM_DFS_NA, CX_SVS_L1 },
	{ 112000000, MUX_GPLL0, 2,   14,    75, QCOM_DFS_NA, CX_SVS_L1 },
	{ 117964800, MUX_GPLL0, 2, 3072, 15625, QCOM_DFS_NA, CX_SVS_L1 },
	{ 120000000, MUX_GPLL0, 10,   0,     0, 0x07, CX_SVS_L1 },
};

/*
 * 100 MHz-capable QUP SE frequency plan, used by S2-S6 of every quadrant
 * wrapper. The rows it shares with the 120 MHz plan carry different DFS
 * indices, so the two plans cannot be folded into one.
 */
static const struct qcom_clk_mux_config qup_se_100mhz[] = {
	{   7372800, MUX_GPLL0, 2, 192, 15625, QCOM_DFS_NA, CX_SVS },
	{  14745600, MUX_GPLL0, 2, 384, 15625, QCOM_DFS_NA, CX_SVS },
	{  19200000, MUX_XO,    2,   0,     0, 0x00, CX_SVS },
	{  29491200, MUX_GPLL0, 2, 768, 15625, QCOM_DFS_NA, CX_SVS },
	{  32000000, MUX_GPLL0, 2,   4,    75, 0x01, CX_SVS },
	{  48000000, MUX_GPLL0, 2,   2,    25, 0x02, CX_SVS },
	{  51200000, MUX_GPLL0, 2,  32,   375, QCOM_DFS_NA, CX_SVS },
	{  64000000, MUX_GPLL0, 2,   8,    75, 0x03, CX_SVS },
	{  66666667, MUX_GPLL0, 18,  0,     0, 0x04, CX_SVS },
	{  75000000, MUX_GPLL0, 16,  0,     0, QCOM_DFS_NA, CX_SVS_L1 },
	{  80000000, MUX_GPLL0, 2,   2,    15, 0x05, CX_SVS_L1 },
	{  96000000, MUX_GPLL0, 2,   4,    25, 0x06, CX_SVS_L1 },
	{ 100000000, MUX_GPLL0, 12,  0,     0, 0x07, CX_SVS_L1 },
};

/*
 * Wrapper 3's QSPI_REF plan (central GCC), reaching 240 MHz. This RCG feeds
 * both gcc_qupv3_wrap3_qspi_ref_clk and gcc_qupv3_wrap3_s0_clk. Its rows
 * source from the central GPLL0's even (/2) output except where only the main
 * output divides cleanly.
 */
static const struct qcom_clk_mux_config qup_se_240mhz[] = {
	{   7372800, MUX_GPLL0_EVEN, 2,  384, 15625, QCOM_DFS_NA, CX_SVS },
	{  14745600, MUX_GPLL0_EVEN, 2,  768, 15625, QCOM_DFS_NA, CX_SVS },
	{  19200000, MUX_XO,         2,    0,     0, 0x00, CX_SVS },
	{  29491200, MUX_GPLL0_EVEN, 2, 1536, 15625, QCOM_DFS_NA, CX_SVS },
	{  32000000, MUX_GPLL0_EVEN, 2,    8,    75, 0x01, CX_SVS },
	{  48000000, MUX_GPLL0_EVEN, 2,    4,    25, 0x02, CX_SVS },
	{  51200000, MUX_GPLL0_EVEN, 2,   64,   375, QCOM_DFS_NA, CX_SVS },
	{  64000000, MUX_GPLL0_EVEN, 2,   16,    75, 0x03, CX_SVS },
	{  75000000, MUX_GPLL0_EVEN, 8,    0,     0, QCOM_DFS_NA, CX_SVS },
	{  80000000, MUX_GPLL0_EVEN, 2,    4,    15, QCOM_DFS_NA, CX_SVS },
	{  96000000, MUX_GPLL0_EVEN, 2,    8,    25, 0x04, CX_SVS },
	{ 100000000, MUX_GPLL0,      12,   0,     0, 0x05, CX_SVS },
	{ 102400000, MUX_GPLL0_EVEN, 2,  128,   375, QCOM_DFS_NA, CX_SVS },
	{ 112000000, MUX_GPLL0_EVEN, 2,   28,    75, QCOM_DFS_NA, CX_SVS },
	{ 117964800, MUX_GPLL0_EVEN, 2, 6144, 15625, QCOM_DFS_NA, CX_SVS },
	{ 120000000, MUX_GPLL0,      10,   0,     0, 0x06, CX_SVS },
	{ 150000000, MUX_GPLL0_EVEN, 4,    0,     0, QCOM_DFS_NA, CX_SVS_L1 },
	{ 240000000, MUX_GPLL0,      5,    0,     0, 0x07, CX_SVS_L1 },
};

/*
 * PLL source-vote table, keyed by window + @mux_sel; XO has no entry and is
 * not voted. Each quadrant controller votes its own GPLL0 through its own
 * register, and the central GCC's main and even outputs share one GPLL0 vote.
 * Direct register write, not RPMh.
 */
static const struct qcom_clk_src_vote qup_se_src_votes[] = {
	{ &se_gcc_window, MUX_GPLL0, SE_GCC_PLL_BRANCH_ENA_VOTE,
	  PLL_VOTE_BIT_GPLL0 },
	{ &ne_gcc_window, MUX_GPLL0, NE_GCC_PLL_BRANCH_ENA_VOTE,
	  PLL_VOTE_BIT_GPLL0 },
	{ &gcc_window, MUX_GPLL0, GCC_PLL_BRANCH_ENA_VOTE,
	  PLL_VOTE_BIT_GPLL0 },
	/* GPLL0_EVEN is the /2 output of GPLL0; voting GPLL0 holds it on. */
	{ &gcc_window, MUX_GPLL0_EVEN, GCC_PLL_BRANCH_ENA_VOTE,
	  PLL_VOTE_BIT_GPLL0 },
};

/*
 * QUPv3 wrapper-level branch clock (core/core_2x/m_ahb/s_ahb): no RCG, no
 * frequency plan -- enable/disable only, via the same shared vote register +
 * CBCR poll qcom_qup_clk_ops already implements for the SE branches. Rate
 * scaling for these is delegated to RPMh BCM/AOP, outside OP-TEE's scope.
 */
#define QUP_BRANCH_CLK(_name, _win, _cbcr, _vote_reg, _vote_bit)	\
	{								\
		.name = (_name),					\
		.window = (_win),					\
		.cbcr_addr = (_cbcr),					\
		.vote_reg_addr = (_vote_reg),				\
		.vote_bit = (_vote_bit),				\
	}

#define QUP_SE_DOMAIN(_name, _win, _cmd_rcgr, _cbcr, _plan, _vote_reg,	\
		      _vote_bit)					\
	{								\
		.name = (_name),					\
		.window = (_win),					\
		.cmd_rcgr_addr = (_cmd_rcgr),				\
		.cbcr_addr = (_cbcr),					\
		.vote_reg_addr = (_vote_reg),				\
		.vote_bit = (_vote_bit),				\
		.dfs_states = QUP_SE_DFS_STATES,			\
		.configs = (_plan),					\
		.n_configs = ARRAY_SIZE(_plan),				\
	}

static const struct qcom_clk_domain qup_se_domains[] = {
	/*
	 * SE_GCC wrapper 0: S0-S1 at 120 MHz, S2-S6 at 100 MHz. All gate
	 * through SE_GCC's base vote register.
	 */
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap0_s0_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP0_S0_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP0_S0_CBCR, qup_se_120mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_S0_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap0_s1_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP0_S1_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP0_S1_CBCR, qup_se_120mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_S1_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap0_s2_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP0_S2_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP0_S2_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_S2_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap0_s3_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP0_S3_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP0_S3_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_S3_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap0_s4_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP0_S4_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP0_S4_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_S4_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap0_s5_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP0_S5_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP0_S5_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_S5_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap0_s6_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP0_S6_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP0_S6_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_S6_SHFT),

	/*
	 * SE_GCC wrapper 1: same plan split as wrapper 0. S0-S4 fit in the
	 * base vote register; S5-S6 spill into VOTE_1.
	 */
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap1_s0_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP1_S0_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP1_S0_CBCR, qup_se_120mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP1_S0_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap1_s1_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP1_S1_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP1_S1_CBCR, qup_se_120mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP1_S1_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap1_s2_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP1_S2_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP1_S2_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP1_S2_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap1_s3_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP1_S3_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP1_S3_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP1_S3_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap1_s4_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP1_S4_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP1_S4_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP1_S4_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap1_s5_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP1_S5_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP1_S5_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE_1, SE_QUPV3_WRAP1_S5_SHFT),
	QUP_SE_DOMAIN("se_gcc_qupv3_wrap1_s6_clk", &se_gcc_window,
		      SE_GCC_QUPV3_WRAP1_S6_CMD_RCGR,
		      SE_GCC_QUPV3_WRAP1_S6_CBCR, qup_se_100mhz,
		      SE_GCC_CLOCK_BRANCH_ENA_VOTE_1, SE_QUPV3_WRAP1_S6_SHFT),

	/*
	 * NE_GCC wrapper 2: same plan split. All SEs gate through NE_GCC's
	 * VOTE_1; only m_ahb/s_ahb use its base vote register.
	 */
	QUP_SE_DOMAIN("ne_gcc_qupv3_wrap2_s0_clk", &ne_gcc_window,
		      NE_GCC_QUPV3_WRAP2_S0_CMD_RCGR,
		      NE_GCC_QUPV3_WRAP2_S0_CBCR, qup_se_120mhz,
		      NE_GCC_CLOCK_BRANCH_ENA_VOTE_1, NE_QUPV3_WRAP2_S0_SHFT),
	QUP_SE_DOMAIN("ne_gcc_qupv3_wrap2_s1_clk", &ne_gcc_window,
		      NE_GCC_QUPV3_WRAP2_S1_CMD_RCGR,
		      NE_GCC_QUPV3_WRAP2_S1_CBCR, qup_se_120mhz,
		      NE_GCC_CLOCK_BRANCH_ENA_VOTE_1, NE_QUPV3_WRAP2_S1_SHFT),
	QUP_SE_DOMAIN("ne_gcc_qupv3_wrap2_s2_clk", &ne_gcc_window,
		      NE_GCC_QUPV3_WRAP2_S2_CMD_RCGR,
		      NE_GCC_QUPV3_WRAP2_S2_CBCR, qup_se_100mhz,
		      NE_GCC_CLOCK_BRANCH_ENA_VOTE_1, NE_QUPV3_WRAP2_S2_SHFT),
	QUP_SE_DOMAIN("ne_gcc_qupv3_wrap2_s3_clk", &ne_gcc_window,
		      NE_GCC_QUPV3_WRAP2_S3_CMD_RCGR,
		      NE_GCC_QUPV3_WRAP2_S3_CBCR, qup_se_100mhz,
		      NE_GCC_CLOCK_BRANCH_ENA_VOTE_1, NE_QUPV3_WRAP2_S3_SHFT),
	QUP_SE_DOMAIN("ne_gcc_qupv3_wrap2_s4_clk", &ne_gcc_window,
		      NE_GCC_QUPV3_WRAP2_S4_CMD_RCGR,
		      NE_GCC_QUPV3_WRAP2_S4_CBCR, qup_se_100mhz,
		      NE_GCC_CLOCK_BRANCH_ENA_VOTE_1, NE_QUPV3_WRAP2_S4_SHFT),
	QUP_SE_DOMAIN("ne_gcc_qupv3_wrap2_s5_clk", &ne_gcc_window,
		      NE_GCC_QUPV3_WRAP2_S5_CMD_RCGR,
		      NE_GCC_QUPV3_WRAP2_S5_CBCR, qup_se_100mhz,
		      NE_GCC_CLOCK_BRANCH_ENA_VOTE_1, NE_QUPV3_WRAP2_S5_SHFT),
	QUP_SE_DOMAIN("ne_gcc_qupv3_wrap2_s6_clk", &ne_gcc_window,
		      NE_GCC_QUPV3_WRAP2_S6_CMD_RCGR,
		      NE_GCC_QUPV3_WRAP2_S6_CBCR, qup_se_100mhz,
		      NE_GCC_CLOCK_BRANCH_ENA_VOTE_1, NE_QUPV3_WRAP2_S6_SHFT),

	/*
	 * Central GCC wrapper 3: one SE, whose RCG is the wrapper's shared
	 * QSPI_REF RCG -- there is no WRAP3_S0 CMD_RCGR. The SE owns the rate;
	 * gcc_qupv3_wrap3_qspi_ref_clk is the second branch off that same RCG
	 * and appears below as a branch-only clock, since setting a rate
	 * through it would program this domain's RCG behind its back.
	 */
	QUP_SE_DOMAIN("gcc_qupv3_wrap3_s0_clk", &gcc_window,
		      GCC_QUPV3_WRAP3_QSPI_REF_CMD_RCGR,
		      GCC_QUPV3_WRAP3_S0_CBCR, qup_se_240mhz,
		      GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP3_S0_SHFT),

	/* QUPv3 wrapper-level branch clocks: enable/disable only, no RCG. */
	QUP_BRANCH_CLK("se_gcc_qupv3_wrap0_core_2x_clk", &se_gcc_window,
		       SE_GCC_QUPV3_WRAP0_CORE_2X_CBCR,
		       SE_GCC_CLOCK_BRANCH_ENA_VOTE,
		       SE_QUPV3_WRAP0_CORE_2X_SHFT),
	QUP_BRANCH_CLK("se_gcc_qupv3_wrap0_core_clk", &se_gcc_window,
		       SE_GCC_QUPV3_WRAP0_CORE_CBCR,
		       SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_CORE_SHFT),
	QUP_BRANCH_CLK("se_gcc_qupv3_wrap0_m_ahb_clk", &se_gcc_window,
		       SE_GCC_QUPV3_WRAP0_M_AHB_CBCR,
		       SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_M_AHB_SHFT),
	QUP_BRANCH_CLK("se_gcc_qupv3_wrap0_s_ahb_clk", &se_gcc_window,
		       SE_GCC_QUPV3_WRAP0_S_AHB_CBCR,
		       SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP0_S_AHB_SHFT),

	QUP_BRANCH_CLK("se_gcc_qupv3_wrap1_core_2x_clk", &se_gcc_window,
		       SE_GCC_QUPV3_WRAP1_CORE_2X_CBCR,
		       SE_GCC_CLOCK_BRANCH_ENA_VOTE,
		       SE_QUPV3_WRAP1_CORE_2X_SHFT),
	QUP_BRANCH_CLK("se_gcc_qupv3_wrap1_core_clk", &se_gcc_window,
		       SE_GCC_QUPV3_WRAP1_CORE_CBCR,
		       SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP1_CORE_SHFT),
	QUP_BRANCH_CLK("se_gcc_qupv3_wrap1_m_ahb_clk", &se_gcc_window,
		       SE_GCC_QUPV3_WRAP1_M_AHB_CBCR,
		       SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP1_M_AHB_SHFT),
	QUP_BRANCH_CLK("se_gcc_qupv3_wrap1_s_ahb_clk", &se_gcc_window,
		       SE_GCC_QUPV3_WRAP1_S_AHB_CBCR,
		       SE_GCC_CLOCK_BRANCH_ENA_VOTE, SE_QUPV3_WRAP1_S_AHB_SHFT),

	QUP_BRANCH_CLK("ne_gcc_qupv3_wrap2_core_2x_clk", &ne_gcc_window,
		       NE_GCC_QUPV3_WRAP2_CORE_2X_CBCR,
		       NE_GCC_CLOCK_BRANCH_ENA_VOTE_1,
		       NE_QUPV3_WRAP2_CORE_2X_SHFT),
	QUP_BRANCH_CLK("ne_gcc_qupv3_wrap2_core_clk", &ne_gcc_window,
		       NE_GCC_QUPV3_WRAP2_CORE_CBCR,
		       NE_GCC_CLOCK_BRANCH_ENA_VOTE_1,
		       NE_QUPV3_WRAP2_CORE_SHFT),
	QUP_BRANCH_CLK("ne_gcc_qupv3_wrap2_m_ahb_clk", &ne_gcc_window,
		       NE_GCC_QUPV3_WRAP2_M_AHB_CBCR,
		       NE_GCC_CLOCK_BRANCH_ENA_VOTE,
		       NE_QUPV3_WRAP2_M_AHB_SHFT),
	QUP_BRANCH_CLK("ne_gcc_qupv3_wrap2_s_ahb_clk", &ne_gcc_window,
		       NE_GCC_QUPV3_WRAP2_S_AHB_CBCR,
		       NE_GCC_CLOCK_BRANCH_ENA_VOTE,
		       NE_QUPV3_WRAP2_S_AHB_SHFT),

	/* Wrapper 3 carries an m_clk where the others have m_ahb_clk. */
	QUP_BRANCH_CLK("gcc_qupv3_wrap3_core_2x_clk", &gcc_window,
		       GCC_QUPV3_WRAP3_CORE_2X_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP3_CORE_2X_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap3_core_clk", &gcc_window,
		       GCC_QUPV3_WRAP3_CORE_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP3_CORE_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap3_m_clk", &gcc_window,
		       GCC_QUPV3_WRAP3_M_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP3_M_SHFT),
	QUP_BRANCH_CLK("gcc_qupv3_wrap3_s_ahb_clk", &gcc_window,
		       GCC_QUPV3_WRAP3_S_AHB_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE_2, QUPV3_WRAP3_S_AHB_SHFT),
	/*
	 * The second branch off wrapper 3's QSPI_REF RCG. Its rate is whatever
	 * gcc_qupv3_wrap3_s0_clk last set on that shared RCG, so it is
	 * enable/disable only here rather than a second rate-owning domain.
	 */
	QUP_BRANCH_CLK("gcc_qupv3_wrap3_qspi_ref_clk", &gcc_window,
		       GCC_QUPV3_WRAP3_QSPI_REF_CBCR,
		       GCC_CLOCK_BRANCH_ENA_VOTE, QUPV3_WRAP3_QSPI_REF_SHFT),
};

static const struct qcom_clk_bsp nord_clk_bsp = {
	.domains = qup_se_domains,
	.n_domains = ARRAY_SIZE(qup_se_domains),
	.src_votes = qup_se_src_votes,
	.n_src_votes = ARRAY_SIZE(qup_se_src_votes),
};

const struct qcom_clk_bsp *qcom_clk_bsp_get(void)
{
	return &nord_clk_bsp;
}
