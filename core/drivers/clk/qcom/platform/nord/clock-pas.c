// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk_qcom.h>
#include <io.h>
#include <kernel/delay.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <stdint.h>
#include <util.h>

#include "clock_group.h"

register_phys_mem(MEM_AREA_IO_NSEC, AOSS_CC_BASE, AOSS_CC_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, SOCCP_CSR_BASE, SOCCP_CSR_SIZE);

static TEE_Result soccp_set_core_rate(vaddr_t gcc_base)
{
	vaddr_t cfg_rcgr = gcc_base + GCC_SOCCP_CFG_RCGR;
	vaddr_t cmd_rcgr = gcc_base + GCC_SOCCP_CMD_RCGR;
	uint32_t cfg = io_read32(cfg_rcgr);
	uint32_t val = 0;

	cfg &= ~(QCOM_RCG_CFG_SRC_SEL_FMSK | QCOM_RCG_CFG_SRC_DIV_FMSK);
	cfg |= SHIFT_U32(SOCCP_RCG_SRC_SEL, QCOM_RCG_CFG_SRC_SEL_SHFT) |
	       SHIFT_U32(SOCCP_RCG_SRC_DIV, QCOM_RCG_CFG_SRC_DIV_SHFT);
	io_write32(cfg_rcgr, cfg);

	io_setbits32(cmd_rcgr, CMD_RCGR_UPDATE_BIT);

	if (IO_READ32_POLL_TIMEOUT(cmd_rcgr, val, !(val & CMD_RCGR_UPDATE_BIT),
				   1, 10 * 1000))
		return TEE_ERROR_TIMEOUT;

	return TEE_SUCCESS;
}

static TEE_Result soccp_setup(void)
{
	static const uint32_t branches[] = {
		GCC_SOCCP_ANOC_AXI_CBCR,
		GCC_SOCCP_CNOC_M_AHB_CBCR,
		GCC_SOCCP_CNOC_S_AHB_CBCR,
		GCC_SOCCP_F_CBCR,
		GCC_SOCCP_SS_H_CBCR,
		GCC_SOCCP_TMR_CBCR,
	};
	struct io_pa_va gcc_io = { .pa = GCC_BASE };
	vaddr_t gcc_base = io_pa_or_va(&gcc_io, GCC_SIZE);
	size_t i = 0;

	if (!gcc_base)
		return TEE_ERROR_GENERIC;

	for (i = 0; i < ARRAY_SIZE(branches); i++)
		io_setbits32(gcc_base + branches[i], CBCR_BRANCH_ENABLE_BIT);

	return soccp_set_core_rate(gcc_base);
}

static TEE_Result soccp_enable_processor(void)
{
	struct io_pa_va csr_io = { .pa = SOCCP_CSR_BASE };
	vaddr_t csr = io_pa_or_va(&csr_io, SOCCP_CSR_SIZE);

	if (!csr)
		return TEE_ERROR_GENERIC;

	io_write32(csr + SOCCP_RVSSMP_BOOT_SUPPRESS, 0);

	return TEE_SUCCESS;
}

static TEE_Result soccp_reset_processor(void)
{
	struct io_pa_va aoss_io = { .pa = AOSS_CC_BASE };
	vaddr_t aoss_cc = io_pa_or_va(&aoss_io, AOSS_CC_SIZE);
	struct io_pa_va csr_io = { .pa = SOCCP_CSR_BASE };
	vaddr_t csr = io_pa_or_va(&csr_io, SOCCP_CSR_SIZE);

	if (!aoss_cc || !csr)
		return TEE_ERROR_GENERIC;

	io_write32(csr + SOCCP_RVSSMP_BOOT_SUPPRESS, SOCCP_BOOT_SUPPRESS_BIT);

	io_setbits32(aoss_cc + AOSS_CC_SOCCP_RESTART, AOSS_CC_SS_RESTART_BIT);
	io_setbits32(aoss_cc + AOSS_CC_SOCCP_CONFIG_RESTART,
		     AOSS_CC_SS_RESTART_BIT);

	dsb();
	udelay(300);

	io_clrbits32(aoss_cc + AOSS_CC_SOCCP_CONFIG_RESTART,
		     AOSS_CC_SS_RESTART_BIT);
	io_clrbits32(aoss_cc + AOSS_CC_SOCCP_RESTART, AOSS_CC_SS_RESTART_BIT);

	return TEE_SUCCESS;
}

TEE_Result qcom_clock_enable_pas(enum qcom_clk_group group)
{
	switch (group) {
	case QCOM_CLKS_SOCCP:
		return soccp_setup();
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}

TEE_Result qcom_clock_enable_pas_processor(enum qcom_clk_group group)
{
	switch (group) {
	case QCOM_CLKS_SOCCP:
		return soccp_enable_processor();
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}

TEE_Result qcom_clock_pas_reset(enum qcom_clk_group group)
{
	switch (group) {
	case QCOM_CLKS_SOCCP:
		return soccp_reset_processor();
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
