// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk.h>
#include <drivers/clk_qcom.h>
#include <io.h>
#include <kernel/delay.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <stdint.h>
#include <trace.h>
#include <util.h>

#include "clock_group.h"

register_phys_mem(MEM_AREA_IO_NSEC, AOSS_BASE, AOSS_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, TCSR_BASE, TCSR_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, LPASS_BASE, LPASS_SIZE);

static bool boot_imem_reset_completed;
static struct io_pa_va aoss_io = { .pa = AOSS_BASE };
static struct io_pa_va tcsr_io = { .pa = TCSR_BASE };
static struct io_pa_va lpass_io = { .pa = LPASS_BASE };

static const uint32_t lmcu_cbcr_offsets[] = {
	0xa0fc, 0xa210, 0xa184, 0xb124,
	0xa1e8, 0xa288, 0xa274, 0x84b0,
};

static const struct qcom_lucidfastn6rf_pll_config lmcu_pll_cfg = {
	.l_val = 0x1c,
	.cal_l_val = 0x1b,
	.pre_div = 1,
	.config_ctl = 0x20485699,
	.config_ctl_u = 0x00002261,
	.config_ctl_u1 = 0xb2523bbc,
	.test_ctl_u = 0x3,
	.user_ctl_u = 0x805,
};

static const struct qcom_lucidfastn6rf_pll_config lpass_pll_cfg = {
	.l_val = 0x37,
	.cal_l_val = 0x30,
	.pre_div = 1,
	.config_ctl = 0x20485699,
	.config_ctl_u = 0x00002261,
	.config_ctl_u1 = 0xb2523bbc,
	.test_ctl_u = 0x3,
	.user_ctl_u = 0x805,
};

static TEE_Result lpass_reset_sequence(vaddr_t aoss, vaddr_t tcsr)
{
	io_setbits32(aoss + LPASS_SYNC_RESET_CTRL,
		     LPASS_SYNC_RESET_CTRL_REQ_CHL_RSIDE);
	io_setbits32(aoss + RPMH_PDC_AUDIO_SYNC_RESET,
		     RPMH_PDC_AUDIO_SYNC_RESET_AUDIO_SYNC_RESET);
	mdelay(10);

	io_setbits32(aoss + AOSS_CC_LPASS_RESTART,
		     AOSS_CC_LPASS_RESTART_SS_RESTART);
	udelay(200);

	io_setbits32(aoss + LPASS_SYNC_RESET_CTRL,
		     LPASS_SYNC_RESET_CTRL_RSP_CHL_WSIDE);
	udelay(10);

	io_clrbits32(aoss + LPASS_SYNC_RESET_CTRL,
		     LPASS_SYNC_RESET_CTRL_RSP_CHL_WSIDE);
	io_clrbits32(aoss + RPMH_PDC_AUDIO_SYNC_RESET,
		     RPMH_PDC_AUDIO_SYNC_RESET_AUDIO_SYNC_RESET);

	io_write32(tcsr + TCSR_LPASS_HALTREQ, 0);
	io_write32(tcsr + TCSR_LPASS_LPICX_NOC_QCHANNEL_QREQN, 0x3);

	io_clrbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_2, TCSR_UGPU_THROTTLE);
	io_clrbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_2, TCSR_UGPU_CLCLEAR);

	io_clrbits32(aoss + AOSS_CC_LPASS_RESTART,
		     AOSS_CC_LPASS_RESTART_SS_RESTART);
	udelay(200);
	io_clrbits32(aoss + LPASS_SYNC_RESET_CTRL,
		     LPASS_SYNC_RESET_CTRL_REQ_CHL_RSIDE);
	udelay(100);

	return TEE_SUCCESS;
}

static TEE_Result lpass_boot_imem_cleanup(vaddr_t aoss, vaddr_t tcsr)
{
	TEE_Result res;

	if (boot_imem_reset_completed)
		return TEE_SUCCESS;

	io_clrbits32(GCC_BASE + GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR, BIT(0));
	res = lpass_reset_sequence(aoss, tcsr);
	if (res)
		return res;

	boot_imem_reset_completed = true;

	return TEE_SUCCESS;
}

static TEE_Result lpass_setup(vaddr_t aoss, vaddr_t tcsr, vaddr_t lpass)
{
	TEE_Result res;
	unsigned int i;

	res = qcom_clock_enable_cbc(GCC_BASE + GCC_CFG_NOC_LPASS_CBCR);
	if (res)
		return res;

	res = lpass_boot_imem_cleanup(aoss, tcsr);
	if (res)
		return res;

	if (!(io_read32(lpass + LPASS_LPICX_NOC_CSRUNIT_LPASS_LPI_NOC_HS_CLK_ENABLE_LOW) & BIT(0))) {
		res = qcom_clock_enable_cbc(lpass + LPASS_AON_CC_LPI_NOC_HS_CBCR);
		if (res)
			return res;
	}
	else
		io_clrbits32(lpass + LPASS_AON_CC_LPI_NOC_HS_CBCR,
			     CBCR_BRANCH_ENABLE_BIT);

	res = qcom_clock_enable_cbc(lpass + LPASS_AON_CC_Q6_AXIM_CBCR);
	if (res)
		return res;

	io_setbits32(lpass + LPASS_QDSP6SS_CORE_CBCR, CBCR_BRANCH_ENABLE_BIT);
	io_write32(lpass + LPASS_LPASS_RSC_WAIT_EVENT_OVRD_MASK, 0x8);

	res = qcom_clock_enable_cbc(lpass + LPASS_LPASS_LPI_TCM_AXIS_HS_CBCR);
	if (res)
		return res;

	io_write32(lpass + LPASS_EE0_LPI_TCM_256KB_BLOCK_RET_VOTE, 0xffffffff);
	for (i = 0; i <= LPASS_LPI_TCM_256KB_BLOCK_MAXN; i++)
		io_setbits32(lpass + LPASS_LPI_TCM_256KB_BLOCK_BASE +
				 i * LPASS_LPI_TCM_256KB_BLOCK_STRIDE, BIT(0));

	res = qcom_lucidfastn6rf_pll_enable(lpass + LPASS_Q6_PLL_OFFSET, &lpass_pll_cfg);
	if (res)
		return res;

	return qcom_clock_set_rate(lpass + LPASS_QDSP6SS_CORE_CFG_RCGR,
				   lpass + LPASS_QDSP6SS_CORE_CMD_RCGR,
				   Q6RCG_CFG_VAL);
}

static TEE_Result lpass_enable_processor(vaddr_t lpass)
{
	uint64_t timeout = timeout_init_us(200000 * 5);

	io_write32(lpass + LPASS_QDSP6SS_RET_CFG, 0x0);
	io_clrbits32(lpass + LPASS_LPASS_ALT_RESET_Q6SS, BIT(0));
	io_setbits32(lpass + LPASS_QDSP6SS_BOOT_CORE_START, BIT(0));
	io_write32(lpass + LPASS_QDSP6SS_BOOT_CMD, 0x1);

	while (!timeout_elapsed(timeout)) {
		if (io_read32(lpass + LPASS_QDSP6SS_BOOT_STATUS) & BIT(0))
			return TEE_SUCCESS;

		udelay(5);
	}

	return TEE_ERROR_TIMEOUT;
}

static void lpass_wdss_lpi_cx_fence(vaddr_t tcsr)
{
	uint64_t timeout;
	uint32_t status;

	io_setbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_2, TCSR_UGPU_THROTTLE);
	io_setbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_2, TCSR_UGPU_CLCLEAR);

	io_clrbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_2, TCSR_UGPU_THROTTLE);
	io_clrbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_2, TCSR_UGPU_CLCLEAR);

	io_clrbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_3, TCSR_WDSS_LPI_CX_QREQN);

	timeout = timeout_init_us(500);
	while (!timeout_elapsed(timeout)) {
		status = io_read32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_1);

		if (!(status & TCSR_WDSS_LPI_CX_QACCEPTN))
			break;

		udelay(1);
	}

	status = io_read32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_1);
	if (!(status & TCSR_WDSS_LPI_CX_QACCEPTN) && !(status & TCSR_WDSS_LPI_CX_QDENY))
		return;

	if (status & TCSR_WDSS_LPI_CX_QDENY) {
		io_setbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_3,
			     TCSR_WDSS_LPI_CX_QREQN);
		udelay(10);
		io_clrbits32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_3,
			     TCSR_WDSS_LPI_CX_QREQN);

		timeout = timeout_init_us(500);
		while (!timeout_elapsed(timeout)) {
			status = io_read32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_1);

			if (!(status & TCSR_WDSS_LPI_CX_QACCEPTN))
				break;

			udelay(1);
		}

		status = io_read32(tcsr + TCSR_SPARE_CC_TCSR_AHB_ARES_1);
		if (!(status & TCSR_WDSS_LPI_CX_QACCEPTN) && !(status & TCSR_WDSS_LPI_CX_QDENY))
			return;
	}
	EMSG("WDSS LPI-CX Q-channel fencing failed: 0x%x", status);
}

static TEE_Result lpass_reset(vaddr_t aoss, vaddr_t tcsr, vaddr_t lpass)
{
	TEE_Result res;
	uint64_t timeout = 0;
	uint32_t qstatus;

	res = qcom_clock_enable_cbc(lpass + LPASS_AON_CC_SLEEP_CBCR);
	if (res)
		return res;

	if (!(io_read32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR) & LPASS_AUDIO_HM_GDSCR_PWR_ON)) {
		io_clrbits32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR,
			     LPASS_AUDIO_HM_GDSCR_SW_COLLAPSE);
		timeout = timeout_init_us(500);
		while (!timeout_elapsed(timeout) &&
		       !(io_read32(lpass +
			 LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR)
			 & LPASS_AUDIO_HM_GDSCR_PWR_ON))
			udelay(1);
		if (!(io_read32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR) &
		      LPASS_AUDIO_HM_GDSCR_PWR_ON))
			return TEE_ERROR_TIMEOUT;
	}

	res = qcom_clock_enable_cbc(lpass + LPASS_AON_CC_AUDIO_HM_H_CBCR);
	if (res)
		return res;

	io_setbits32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_BCR,
		     LPASS_AUDIO_HM_BCR_BLK_ARES);
	timeout = timeout_init_us(200);
	while (!timeout_elapsed(timeout) &&
	       !(io_read32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_BCR) &
		 LPASS_AUDIO_HM_BCR_BLK_ARES_COMPLETE))
		udelay(1);
	if (timeout_elapsed(timeout)) {
		io_setbits32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_BCR,
			     LPASS_AUDIO_HM_BCR_Q_FORCE_RESET);
		udelay(150);
		io_clrbits32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_BCR,
			     LPASS_AUDIO_HM_BCR_Q_FORCE_RESET);
	}
	io_clrbits32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_BCR,
		     LPASS_AUDIO_HM_BCR_BLK_ARES);
	udelay(250);

	io_clrbits32(lpass + LPASS_AUDIO_CC_DIG_PLL_OPMODE,
		     LPASS_AUDIO_CC_DIG_PLL_OPMODE_MASK);

	io_setbits32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR,
		     LPASS_AUDIO_HM_GDSCR_SW_COLLAPSE);
	timeout = timeout_init_us(500);
	while (!timeout_elapsed(timeout) &&
	       (io_read32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR) &
		LPASS_AUDIO_HM_GDSCR_PWR_ON))
		udelay(1);
	if (io_read32(lpass + LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR) & LPASS_AUDIO_HM_GDSCR_PWR_ON)
		return TEE_ERROR_TIMEOUT;

	io_write32(lpass + LPASS_QDSP6SS_RET_CFG, 0x3);
	io_setbits32(lpass + LPASS_LPASS_ALT_RESET_Q6SS, BIT(0));

	io_write32(tcsr + TCSR_LPASS_HALTREQ, 0x1);
	timeout = timeout_init_us(500);
	while (!timeout_elapsed(timeout) &&
	       !(io_read32(tcsr + TCSR_LPASS_HALTACK) & BIT(0)))
		udelay(1);
	if (!(io_read32(tcsr + TCSR_LPASS_HALTACK) & BIT(0)))
		EMSG("LPASS Q6 halt acknowledgement timed out: 0x%x",
		     io_read32(tcsr + TCSR_LPASS_HALTACK));

	io_write32(tcsr + TCSR_LPASS_HALTREQ, TCSR_LPASS_HALT_ALL_MASK);
	timeout = timeout_init_us(500);
	while (!timeout_elapsed(timeout) &&
	       ((io_read32(tcsr + TCSR_LPASS_HALTACK) &
		 TCSR_LPASS_HALT_ALL_MASK) != TCSR_LPASS_HALT_ALL_MASK))
		udelay(1);
	if ((io_read32(tcsr + TCSR_LPASS_HALTACK) & TCSR_LPASS_HALT_ALL_MASK) != TCSR_LPASS_HALT_ALL_MASK)
		EMSG("LPASS master halt acknowledgement timed out: 0x%x",
		     io_read32(tcsr + TCSR_LPASS_HALTACK));

	lpass_wdss_lpi_cx_fence(tcsr);

	io_write32(tcsr + TCSR_LPASS_LPICX_NOC_QCHANNEL_QREQN, 0);
	timeout = timeout_init_us(500);
	while (!timeout_elapsed(timeout) &&
	       (io_read32(tcsr + TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN) &
		TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN_MASK))
		udelay(1);
	if (io_read32(tcsr + TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN) &
	    TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN_MASK) {
		qstatus = io_read32(tcsr +
				     TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN);

		if (qstatus & TCSR_LPASS_LPICX_NOC_QCHANNEL_QDENY_MASK) {
			io_write32(tcsr + TCSR_LPASS_LPICX_NOC_QCHANNEL_QREQN,
				   0x3);
			udelay(10);
			io_write32(tcsr + TCSR_LPASS_LPICX_NOC_QCHANNEL_QREQN,
				   0x0);
			timeout = timeout_init_us(500);
			while (!timeout_elapsed(timeout) &&
			       (io_read32(tcsr +
				 TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN) &
				TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN_MASK))
				udelay(1);
			if (io_read32(tcsr +
				      TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN)
			    & TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN_MASK)
				EMSG("LPASS LPICX Q-channel fencing failed: "
				     "0x%x",
				     io_read32(tcsr +
				     TCSR_LPASS_LPICX_NOC_QCHANNEL_QACCEPTN));
		}
	}

	return lpass_reset_sequence(aoss, tcsr);
}

static TEE_Result lmcu_setup(vaddr_t aoss, vaddr_t tcsr, vaddr_t lpass,
			     uint32_t cc_offset)
{
	vaddr_t cc = lpass + cc_offset;
	TEE_Result res;
	unsigned int i;

	res = qcom_clock_enable_cbc(GCC_BASE + GCC_CFG_NOC_LPASS_CBCR);
	if (res)
		return res;

	res = lpass_boot_imem_cleanup(aoss, tcsr);
	if (res)
		return res;

	for (i = 0; i < ARRAY_SIZE(lmcu_cbcr_offsets); i++) {
		res = qcom_clock_enable_cbc(cc + lmcu_cbcr_offsets[i]);
		if (res)
			return res;
	}

	res = qcom_lucidfastn6rf_pll_enable(cc + LPASS_LMCU_PLL_OFFSET, &lmcu_pll_cfg);
	if (res)
		return res;

	return qcom_clock_set_rate(cc + LPASS_LMCU_CC_NOC_HS_CFG_RCGR,
				   cc + LPASS_LMCU_CC_NOC_HS_CMD_RCGR,
				   LMCURCG_CFG_VAL);
}

static TEE_Result lmcu_reset(vaddr_t lpass, vaddr_t tcsr,
			     uint32_t ee_mode_offset,
			     uint32_t tcsr_reset_offset)
{
	uint64_t timeout = timeout_init_us(500);

	io_write32(lpass + ee_mode_offset, 0x1);
	timeout = timeout_init_us(500);
	while (!timeout_elapsed(timeout)) {
		if (io_read32(lpass + LPASS_Q6_CURRENT_EE) == 0x1)
			break;

		udelay(1);
	}

	io_write32(tcsr + tcsr_reset_offset + TCSR_LMCU_RESET_SSR_HALTREQ, 0x7);
	timeout = timeout_init_us(2000);
	while (!timeout_elapsed(timeout)) {
		if (io_read32(tcsr + tcsr_reset_offset +
				  TCSR_LMCU_RESET_SSR_HALTACK) == 0x7)
			break;

		udelay(5);
	}

	io_setbits32(tcsr + tcsr_reset_offset + TCSR_LMCU_RESET_ARES_CTL,
		     TCSR_LMCU_FUNC_RESET_ALT_ARES_EN);
	udelay(50);
	io_write32(tcsr + tcsr_reset_offset + TCSR_LMCU_RESET_SSR_HALTREQ, 0x0);
	io_clrbits32(tcsr + tcsr_reset_offset + TCSR_LMCU_RESET_ARES_CTL,
		     TCSR_LMCU_FUNC_RESET_ALT_ARES_EN);
	udelay(50);

	return TEE_SUCCESS;
}

TEE_Result qcom_clock_enable_pas(enum qcom_clk_group group)
{
	vaddr_t aoss = io_pa_or_va(&aoss_io, AOSS_SIZE);
	vaddr_t tcsr = io_pa_or_va(&tcsr_io, TCSR_SIZE);
	vaddr_t lpass = io_pa_or_va(&lpass_io, LPASS_SIZE);

	switch (group) {
	case QCOM_CLKS_LPASS:
		return lpass_setup(aoss, tcsr, lpass);
	case QCOM_CLKS_LMCU0:
		return lmcu_setup(aoss, tcsr, lpass, LPASS_LMCU_0_CC_OFFSET);
	case QCOM_CLKS_LMCU1:
		return lmcu_setup(aoss, tcsr, lpass, LPASS_LMCU_1_CC_OFFSET);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

TEE_Result qcom_clock_enable_pas_processor(enum qcom_clk_group group)
{
	vaddr_t lpass = io_pa_or_va(&lpass_io, LPASS_SIZE);

	switch (group) {
	case QCOM_CLKS_LPASS:
		return lpass_enable_processor(lpass);
	case QCOM_CLKS_LMCU0:
	case QCOM_CLKS_LMCU1:
		return TEE_SUCCESS;
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

TEE_Result qcom_clock_pas_reset(enum qcom_clk_group group)
{
	vaddr_t aoss = io_pa_or_va(&aoss_io, AOSS_SIZE);
	vaddr_t tcsr = io_pa_or_va(&tcsr_io, TCSR_SIZE);
	vaddr_t lpass = io_pa_or_va(&lpass_io, LPASS_SIZE);

	switch (group) {
	case QCOM_CLKS_LPASS:
		return lpass_reset(aoss, tcsr, lpass);
	case QCOM_CLKS_LMCU0:
		return lmcu_reset(lpass, tcsr, LPASS_LMCU_0_EE_MODE,
				  TCSR_LMCU_0_RESET_OFFSET);
	case QCOM_CLKS_LMCU1:
		return lmcu_reset(lpass, tcsr, LPASS_LMCU_1_EE_MODE,
				  TCSR_LMCU_1_RESET_OFFSET);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
