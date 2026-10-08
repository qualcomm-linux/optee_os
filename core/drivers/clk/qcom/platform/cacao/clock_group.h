/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CLOCK_GROUP_QCOM_H_
#define _CLOCK_GROUP_QCOM_H_

//#include <platform_config.h>

/*
 * TODO: Add PAS-related subsystem offsets for cacao (LPASS, Turing, etc.)
 * when the corresponding PAS driver files are available. The Lemans
 * clock_group.h includes extensive PAS offsets that would need to be
 * derived from cacao-specific PAS documentation.
 */

#ifdef CFG_QCOM_CLK_CFG
/*
 * Full physical addresses for cacao GCC QUP SE clocks.
 * Derived from HALclkHWIOcacao.h (GCC_CLK_CTL_REG_REG_BASE = 0x00100000).
 * All addresses are (GCC_BASE + offset).
 */

/* WRAP0 SE CBCRs and CMD_RCGRs */
#define GCC_QUPV3_WRAP0_S0_CBCR			(GCC_BASE + 0x23130)
#define GCC_QUPV3_WRAP0_S0_CMD_RCGR		(GCC_BASE + 0x23008)
#define GCC_QUPV3_WRAP0_S1_CBCR			(GCC_BASE + 0x23268)
#define GCC_QUPV3_WRAP0_S1_CMD_RCGR		(GCC_BASE + 0x23140)
#define GCC_QUPV3_WRAP0_S2_CBCR			(GCC_BASE + 0x233A0)
#define GCC_QUPV3_WRAP0_S2_CMD_RCGR		(GCC_BASE + 0x23278)
#define GCC_QUPV3_WRAP0_S3_CBCR			(GCC_BASE + 0x234D8)
#define GCC_QUPV3_WRAP0_S3_CMD_RCGR		(GCC_BASE + 0x233B0)
#define GCC_QUPV3_WRAP0_S4_CBCR			(GCC_BASE + 0x23610)
#define GCC_QUPV3_WRAP0_S4_CMD_RCGR		(GCC_BASE + 0x234E8)
#define GCC_QUPV3_WRAP0_S5_CBCR			(GCC_BASE + 0x23748)
#define GCC_QUPV3_WRAP0_S5_CMD_RCGR		(GCC_BASE + 0x23620)

/* WRAP1 SE CBCRs and CMD_RCGRs */
#define GCC_QUPV3_WRAP1_S0_CBCR			(GCC_BASE + 0x24130)
#define GCC_QUPV3_WRAP1_S0_CMD_RCGR		(GCC_BASE + 0x24008)
#define GCC_QUPV3_WRAP1_S1_CBCR			(GCC_BASE + 0x24268)
#define GCC_QUPV3_WRAP1_S1_CMD_RCGR		(GCC_BASE + 0x24140)
#define GCC_QUPV3_WRAP1_S2_CBCR			(GCC_BASE + 0x243A0)
#define GCC_QUPV3_WRAP1_S2_CMD_RCGR		(GCC_BASE + 0x24278)
#define GCC_QUPV3_WRAP1_S3_CBCR			(GCC_BASE + 0x244D8)
#define GCC_QUPV3_WRAP1_S3_CMD_RCGR		(GCC_BASE + 0x243B0)
#define GCC_QUPV3_WRAP1_S4_CBCR			(GCC_BASE + 0x24610)
#define GCC_QUPV3_WRAP1_S4_CMD_RCGR		(GCC_BASE + 0x244E8)
#define GCC_QUPV3_WRAP1_S5_CBCR			(GCC_BASE + 0x24748)
#define GCC_QUPV3_WRAP1_S5_CMD_RCGR		(GCC_BASE + 0x24620)
#define GCC_QUPV3_WRAP1_S6_CBCR			(GCC_BASE + 0x24880)
#define GCC_QUPV3_WRAP1_S6_CMD_RCGR		(GCC_BASE + 0x24758)

/* Wrapper infrastructure clocks (core/core_2x/m_ahb/s_ahb): no RCG backs these. */
#define GCC_QUPV3_WRAP_0_M_AHB_CBCR		(GCC_BASE + 0x23008)
#define GCC_QUPV3_WRAP_0_S_AHB_CBCR		(GCC_BASE + 0x2300C)
#define GCC_QUPV3_WRAP0_CORE_CBCR		(GCC_BASE + 0x23014)
#define GCC_QUPV3_WRAP0_CORE_2X_CBCR		(GCC_BASE + 0x23000)

#define GCC_QUPV3_WRAP_1_M_AHB_CBCR		(GCC_BASE + 0x24008)
#define GCC_QUPV3_WRAP_1_S_AHB_CBCR		(GCC_BASE + 0x2400C)
#define GCC_QUPV3_WRAP1_CORE_CBCR		(GCC_BASE + 0x24014)
#define GCC_QUPV3_WRAP1_CORE_2X_CBCR		(GCC_BASE + 0x24000)

/*
 * Shared branch-enable vote registers; branches gate through one of these,
 * not their own CBCR CLK_ENABLE bit.
 */
#define GCC_CLOCK_BRANCH_ENA_VOTE		(GCC_BASE + 0x52004)
#define GCC_CLOCK_BRANCH_ENA_VOTE_1		(GCC_BASE + 0x73000)
#define GCC_CLOCK_BRANCH_ENA_VOTE_2		(GCC_BASE + 0x7C000)

/* Vote-bit positions, named after their HWIO_..._CLK_ENA_SHFT counterparts. */

/* Bits within GCC_CLOCK_BRANCH_ENA_VOTE_1. */
#define QUPV3_WRAP1_CORE_2X_SHFT		18
#define QUPV3_WRAP1_CORE_SHFT			19
#define QUPV3_WRAP_1_M_AHB_SHFT			20
#define QUPV3_WRAP_1_S_AHB_SHFT			21
#define QUPV3_WRAP1_S0_SHFT			22
#define QUPV3_WRAP1_S1_SHFT			23
#define QUPV3_WRAP1_S2_SHFT			24
#define QUPV3_WRAP1_S3_SHFT			25
#define QUPV3_WRAP1_S4_SHFT			26
#define QUPV3_WRAP1_S5_SHFT			27
#define QUPV3_WRAP1_S6_SHFT			28

/* Bits within GCC_CLOCK_BRANCH_ENA_VOTE_2. */
#define QUPV3_WRAP0_CORE_SHFT			0
#define QUPV3_WRAP_0_S_AHB_SHFT			1
#define QUPV3_WRAP_0_M_AHB_SHFT			2
#define QUPV3_WRAP0_CORE_2X_SHFT		3
#define QUPV3_WRAP0_S0_SHFT			4
#define QUPV3_WRAP0_S1_SHFT			5
#define QUPV3_WRAP0_S2_SHFT			6
#define QUPV3_WRAP0_S3_SHFT			7
#define QUPV3_WRAP0_S4_SHFT			8
#define QUPV3_WRAP0_S5_SHFT			9

/*
 * PLL branch-enable vote register and bits for the PLLs QUP SE RCGs source
 * from; direct GCC write, not RPMh.
 */
#define GCC_PLL_BRANCH_ENA_VOTE			(GCC_BASE + 0x52000)
#define GCC_PLL_VOTE_BIT_GPLL0			0
#define GCC_PLL_VOTE_BIT_GPLL2			2

/* RCG register layout; offsets relative to clk_rcg_desc.cmd_rcgr_addr. */
#define QCOM_RCG_CFG_REG_OFFSET			0x4
#define QCOM_RCG_M_REG_OFFSET			0x8
#define QCOM_RCG_N_REG_OFFSET			0xC
#define QCOM_RCG_D_REG_OFFSET			0x10
#define QCOM_RCG_CMD_DFSR_REG_OFFSET		0x14
#define QCOM_RCG_PERF_DFSR_REG_OFFSET		0x1C
#define QCOM_RCG_PERF_M_DFSR_REG_OFFSET		0x5C
#define QCOM_RCG_PERF_N_DFSR_REG_OFFSET		0x9C
#define QCOM_RCG_PERF_D_DFSR_REG_OFFSET		0xDC

/* CMD_RCGR fields. */
#define QCOM_RCG_CMD_CFG_UPDATE_FMSK		0x00000001

/* CFG_RCGR fields. */
#define QCOM_RCG_CFG_HW_CLK_CONTROL_FMSK	0x00100000
#define QCOM_RCG_CFG_MODE_FMSK			0x00003000
#define QCOM_RCG_CFG_MODE_SHFT			0xC
#define QCOM_RCG_CFG_SRC_SEL_FMSK		0x00000700
#define QCOM_RCG_CFG_SRC_SEL_SHFT		0x8
#define QCOM_RCG_CFG_SRC_DIV_FMSK		0x0000001F
#define QCOM_RCG_CFG_SRC_DIV_SHFT		0
#define QCOM_RCG_CFG_DUAL_EDGE_MODE_VAL		0x2

/* CMD_DFSR fields. */
#define QCOM_RCG_CMD_DFSR_HW_CLK_CONTROL_FMSK	0x00000020
#define QCOM_RCG_CMD_DFSR_DFS_EN_FMSK		0x00000001
#define QCOM_RCG_CMD_DFSR_SW_PERF_STATE_FMSK	0x00007800
#define QCOM_RCG_CMD_DFSR_SW_PERF_STATE_SHFT	11
#endif /* CFG_QCOM_CLK_CFG */

#endif /* _CLOCK_GROUP_QCOM_H_ */