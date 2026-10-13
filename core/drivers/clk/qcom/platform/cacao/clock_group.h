/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CLOCK_GROUP_H_
#define _CLOCK_GROUP_H_

#ifdef CFG_QCOM_PAS_PTA
#define GCC_CFG_NOC_LPASS_CBCR                                     0x47174
#define GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR                             0x47110
#define LPASS_QDSP6SS_RET_CFG                                      0xc0001c
#define LPASS_QDSP6SS_BOOT_CORE_START                              0xc00400
#define LPASS_QDSP6SS_BOOT_CMD                                     0xc00404
#define LPASS_QDSP6SS_BOOT_STATUS                                  0xc00408
#define LPASS_QDSP6SS_CORE_CMD_RCGR                                0xc41020
#define LPASS_QDSP6SS_CORE_CFG_RCGR                                0xc41024
#define LPASS_QDSP6SS_CORE_CBCR                                    0xc41040
#define LPASS_Q6_PLL_OFFSET                                        0xc40000
#define LPASS_LPASS_RSC_WAIT_EVENT_OVRD_MASK                       0x12e0018
#define LPASS_LPICX_NOC_CSRUNIT_LPASS_LPI_NOC_HS_CLK_ENABLE_LOW    0x18240a0
#define LPASS_AON_CC_LPI_NOC_HS_CBCR                               0x1221038
#define LPASS_AON_CC_Q6_AXIM_CBCR                                  0x1221034
#define LPASS_AON_CC_LPASS_AUDIO_HM_BCR                            0x121108c
#define LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR                          0x1211090
#define LPASS_AON_CC_SLEEP_CBCR                                    0x1218004
#define LPASS_AON_CC_AUDIO_HM_H_CBCR                               0x1211014
#define LPASS_AON_CC_LPASS_AUDIO_HM_AT_CBCR                        0x1212100
#define LPASS_AON_CC_LPASS_AUDIO_HM_TSCTR_CBCR                     0x1212108
#define LPASS_AUDIO_CC_DIG_PLL_OPMODE                              0xfc9038
#define LPASS_LPASS_LPI_TCM_AXIS_HS_CBCR                           0x166c400
#define LPASS_LPI_TCM_256KB_BLOCK_BASE                             0x166c000
#define LPASS_LPI_TCM_256KB_BLOCK_STRIDE                           0x4
#define LPASS_LPI_TCM_256KB_BLOCK_MAXN                             31
#define LPASS_EE0_LPI_TCM_256KB_BLOCK_RET_VOTE                     0x166cd00
#define LPASS_LPASS_ALT_RESET_Q6SS                                 0x12ea000
#define LPASS_LPASS_EFUSE_Q6SS_EVB_SEL                             0x12eb000
#define Q6RCG_CFG_VAL                                              0x100201
#define AOSS_CC_LPASS_RESTART                                      0x12f7014
#define LPASS_SYNC_RESET_CTRL                                      0x3e11000
#define RPMH_PDC_AUDIO_SYNC_RESET                                  0x5e5000
#define TCSR_LPASS_HALTREQ                                         0xb3000
#define TCSR_LPASS_HALTACK                                         0xb3004
#define TCSR_LPASS_LPICX_NOC_QCHANNEL_QREQN                        0xb3018
#define TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN                     0xb301c

#define LPASS_AUDIO_HM_BCR_BLK_ARES_COMPLETE                       BIT(31)
#define LPASS_AUDIO_HM_BCR_Q_FORCE_RESET                           BIT(2)
#define LPASS_AUDIO_HM_BCR_BLK_ARES                                BIT(0)
#define LPASS_AUDIO_HM_GDSCR_PWR_ON                                BIT(31)
#define LPASS_AUDIO_HM_GDSCR_SW_COLLAPSE                           BIT(0)
#define LPASS_AUDIO_CC_DIG_PLL_OPMODE_MASK                         0x7
#define LPASS_SYNC_RESET_CTRL_REQ_CHL_RSIDE                        BIT(1)
#define LPASS_SYNC_RESET_CTRL_RSP_CHL_WSIDE                        BIT(0)
#define RPMH_PDC_AUDIO_SYNC_RESET_AUDIO_SYNC_RESET                 BIT(0)
#define AOSS_CC_LPASS_RESTART_SS_RESTART                           BIT(0)
#define TCSR_LPASS_HALT_ALL_MASK                                   0x1fff
#define TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN_MASK                BIT(0)
#define TCSR_LPASS_LPICX_NOC_QCHANNEL_QDENY_MASK                   BIT(1)

#define TCSR_SPARE_CC_TCSR_AHB_ARES_1                              0x75000
#define TCSR_SPARE_CC_TCSR_AHB_ARES_2                              0x75004
#define TCSR_SPARE_CC_TCSR_AHB_ARES_3                              0x75008

#define TCSR_WDSS_LPI_CX_QACCEPTN                                  BIT(16)
#define TCSR_WDSS_LPI_CX_QDENY                                     BIT(0)

#define TCSR_WDSS_LPI_CX_QREQN                                     BIT(0)
#define TCSR_UGPU_THROTTLE                                         BIT(2)
#define TCSR_UGPU_CLCLEAR                                          BIT(3)


#define TCSR_LMCU_0_RESET_OFFSET                                   0xb3028
#define TCSR_LMCU_FUNC_RESET_ALT_ARES_EN                           BIT(2)
#define TCSR_LMCU_RESET_ARES_CTL                                   0x0
#define TCSR_LMCU_RESET_SSR_HALTREQ                                0x8
#define TCSR_LMCU_RESET_SSR_HALTACK                                0xc

#define LPASS_LMCU_0_EE_MODE                                       0x1688000
#define LPASS_Q6_CURRENT_EE                                        0x1681000
#define LPASS_LMCU_CORE_0_RVCP_TILE0_SPARE_REG0                    0x2080100

#define LPASS_LMCU_0_CC_OFFSET                                     0x2220000

#define TCSR_LMCU_1_RESET_OFFSET                                   0xb303c

#define LPASS_LMCU_1_EE_MODE                                       0x168c000
#define LPASS_LMCU_CORE_1_RVCP_TILE0_SPARE_REG0                    0x2480100

#define LPASS_LMCU_1_CC_OFFSET                                     0x2620000

#define LPASS_LMCU_CC_NOC_HS_CMD_RCGR                              0xa000
#define LPASS_LMCU_CC_NOC_HS_CFG_RCGR                              0xa004
#define LPASS_LMCU_PLL_OFFSET                                      0x9000

#define CBCR_BRANCH_ENABLE_BIT                                     BIT(0)
#define GDSCR_PWR_ON_BIT                                           BIT(31)
#define CMD_RCGR_UPDATE_BIT                                        BIT(0)

#define LMCURCG_CFG_VAL                                            0x100201
#endif /* CFG_QCOM_PAS_PTA */

#ifdef CFG_QCOM_CLK_CFG
/*
 * Full physical addresses for cacao GCC QUP SE clocks.
 * Derived from HALclkHWIOcacao.h (GCC_CLK_CTL_REG_REG_BASE = 0x00100000).
 * All addresses are (GCC_BASE + offset).
 */

/* WRAP0 SE CBCRs and CMD_RCGRs */
#define GCC_QUPV3_WRAP0_S0_CBCR                                    (GCC_BASE + 0x23130)
#define GCC_QUPV3_WRAP0_S0_CMD_RCGR                                (GCC_BASE + 0x23008)
#define GCC_QUPV3_WRAP0_S1_CBCR                                    (GCC_BASE + 0x23268)
#define GCC_QUPV3_WRAP0_S1_CMD_RCGR                                (GCC_BASE + 0x23140)
#define GCC_QUPV3_WRAP0_S2_CBCR                                    (GCC_BASE + 0x233A0)
#define GCC_QUPV3_WRAP0_S2_CMD_RCGR                                (GCC_BASE + 0x23278)
#define GCC_QUPV3_WRAP0_S3_CBCR                                    (GCC_BASE + 0x234D8)
#define GCC_QUPV3_WRAP0_S3_CMD_RCGR                                (GCC_BASE + 0x233B0)
#define GCC_QUPV3_WRAP0_S4_CBCR                                    (GCC_BASE + 0x23610)
#define GCC_QUPV3_WRAP0_S4_CMD_RCGR                                (GCC_BASE + 0x234E8)
#define GCC_QUPV3_WRAP0_S5_CBCR                                    (GCC_BASE + 0x23748)
#define GCC_QUPV3_WRAP0_S5_CMD_RCGR                                (GCC_BASE + 0x23620)

/* WRAP1 SE CBCRs and CMD_RCGRs */
#define GCC_QUPV3_WRAP1_S0_CBCR                                    (GCC_BASE + 0x24130)
#define GCC_QUPV3_WRAP1_S0_CMD_RCGR                                (GCC_BASE + 0x24008)
#define GCC_QUPV3_WRAP1_S1_CBCR                                    (GCC_BASE + 0x24268)
#define GCC_QUPV3_WRAP1_S1_CMD_RCGR                                (GCC_BASE + 0x24140)
#define GCC_QUPV3_WRAP1_S2_CBCR                                    (GCC_BASE + 0x243A0)
#define GCC_QUPV3_WRAP1_S2_CMD_RCGR                                (GCC_BASE + 0x24278)
#define GCC_QUPV3_WRAP1_S3_CBCR                                    (GCC_BASE + 0x244D8)
#define GCC_QUPV3_WRAP1_S3_CMD_RCGR                                (GCC_BASE + 0x243B0)
#define GCC_QUPV3_WRAP1_S4_CBCR                                    (GCC_BASE + 0x24610)
#define GCC_QUPV3_WRAP1_S4_CMD_RCGR                                (GCC_BASE + 0x244E8)
#define GCC_QUPV3_WRAP1_S5_CBCR                                    (GCC_BASE + 0x24748)
#define GCC_QUPV3_WRAP1_S5_CMD_RCGR                                (GCC_BASE + 0x24620)
#define GCC_QUPV3_WRAP1_S6_CBCR                                    (GCC_BASE + 0x24880)
#define GCC_QUPV3_WRAP1_S6_CMD_RCGR                                (GCC_BASE + 0x24758)

/* Wrapper infrastructure clocks (core/core_2x/m_ahb/s_ahb): no RCG backs these. */
#define GCC_QUPV3_WRAP_0_M_AHB_CBCR                                (GCC_BASE + 0x23008)
#define GCC_QUPV3_WRAP_0_S_AHB_CBCR                                (GCC_BASE + 0x2300C)
#define GCC_QUPV3_WRAP0_CORE_CBCR                                  (GCC_BASE + 0x23014)
#define GCC_QUPV3_WRAP0_CORE_2X_CBCR                               (GCC_BASE + 0x23000)

#define GCC_QUPV3_WRAP_1_M_AHB_CBCR                                (GCC_BASE + 0x24008)
#define GCC_QUPV3_WRAP_1_S_AHB_CBCR                                (GCC_BASE + 0x2400C)
#define GCC_QUPV3_WRAP1_CORE_CBCR                                  (GCC_BASE + 0x24014)
#define GCC_QUPV3_WRAP1_CORE_2X_CBCR                               (GCC_BASE + 0x24000)

/*
 * Shared branch-enable vote registers; branches gate through one of these,
 * not their own CBCR CLK_ENABLE bit.
 */
#define GCC_CLOCK_BRANCH_ENA_VOTE                                  (GCC_BASE + 0x52004)
#define GCC_CLOCK_BRANCH_ENA_VOTE_1                                (GCC_BASE + 0x73000)
#define GCC_CLOCK_BRANCH_ENA_VOTE_2                                (GCC_BASE + 0x7C000)

/* Vote-bit positions, named after their HWIO_..._CLK_ENA_SHFT counterparts. */

/* Bits within GCC_CLOCK_BRANCH_ENA_VOTE_1. */
#define QUPV3_WRAP1_CORE_2X_SHFT                                   18
#define QUPV3_WRAP1_CORE_SHFT                                      19
#define QUPV3_WRAP_1_M_AHB_SHFT                                    20
#define QUPV3_WRAP_1_S_AHB_SHFT                                    21
#define QUPV3_WRAP1_S0_SHFT                                        22
#define QUPV3_WRAP1_S1_SHFT                                        23
#define QUPV3_WRAP1_S2_SHFT                                        24
#define QUPV3_WRAP1_S3_SHFT                                        25
#define QUPV3_WRAP1_S4_SHFT                                        26
#define QUPV3_WRAP1_S5_SHFT                                        27
#define QUPV3_WRAP1_S6_SHFT                                        28

/* Bits within GCC_CLOCK_BRANCH_ENA_VOTE_2. */
#define QUPV3_WRAP0_CORE_SHFT                                      0
#define QUPV3_WRAP_0_S_AHB_SHFT                                    1
#define QUPV3_WRAP_0_M_AHB_SHFT                                    2
#define QUPV3_WRAP0_CORE_2X_SHFT                                   3
#define QUPV3_WRAP0_S0_SHFT                                        4
#define QUPV3_WRAP0_S1_SHFT                                        5
#define QUPV3_WRAP0_S2_SHFT                                        6
#define QUPV3_WRAP0_S3_SHFT                                        7
#define QUPV3_WRAP0_S4_SHFT                                        8
#define QUPV3_WRAP0_S5_SHFT                                        9

/*
 * PLL branch-enable vote register and bits for the PLLs QUP SE RCGs source
 * from; direct GCC write, not RPMh.
 */
#define GCC_PLL_BRANCH_ENA_VOTE                                    (GCC_BASE + 0x52000)
#define GCC_PLL_VOTE_BIT_GPLL0                                     0
#define GCC_PLL_VOTE_BIT_GPLL2                                     2

/* RCG register layout; offsets relative to clk_rcg_desc.cmd_rcgr_addr. */
#define QCOM_RCG_CFG_REG_OFFSET                                    0x4
#define QCOM_RCG_M_REG_OFFSET                                      0x8
#define QCOM_RCG_N_REG_OFFSET                                      0xC
#define QCOM_RCG_D_REG_OFFSET                                      0x10
#define QCOM_RCG_CMD_DFSR_REG_OFFSET                               0x14
#define QCOM_RCG_PERF_DFSR_REG_OFFSET                              0x1C
#define QCOM_RCG_PERF_M_DFSR_REG_OFFSET                            0x5C
#define QCOM_RCG_PERF_N_DFSR_REG_OFFSET                            0x9C
#define QCOM_RCG_PERF_D_DFSR_REG_OFFSET                            0xDC

/* CMD_RCGR fields. */
#define QCOM_RCG_CMD_CFG_UPDATE_FMSK                               0x00000001

/* CFG_RCGR fields. */
#define QCOM_RCG_CFG_HW_CLK_CONTROL_FMSK                           0x00100000
#define QCOM_RCG_CFG_MODE_FMSK                                     0x00003000
#define QCOM_RCG_CFG_MODE_SHFT                                     0xC
#define QCOM_RCG_CFG_SRC_SEL_FMSK                                  0x00000700
#define QCOM_RCG_CFG_SRC_SEL_SHFT                                  0x8
#define QCOM_RCG_CFG_SRC_DIV_FMSK                                  0x0000001F
#define QCOM_RCG_CFG_SRC_DIV_SHFT                                  0
#define QCOM_RCG_CFG_DUAL_EDGE_MODE_VAL                            0x2

/* CMD_DFSR fields. */
#define QCOM_RCG_CMD_DFSR_HW_CLK_CONTROL_FMSK                      0x00000020
#define QCOM_RCG_CMD_DFSR_DFS_EN_FMSK                              0x00000001
#define QCOM_RCG_CMD_DFSR_SW_PERF_STATE_FMSK                       0x00007800
#define QCOM_RCG_CMD_DFSR_SW_PERF_STATE_SHFT                       11
#endif /* CFG_QCOM_CLK_CFG */
#endif /* _CLOCK_GROUP_H_ */
