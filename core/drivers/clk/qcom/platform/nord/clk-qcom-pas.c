// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * PAS (peripheral image loader) clock bring-up for Nord (Wildcat family).
 * Only the SOCCP companion processor is supported; the HPASS and Turing/NSP
 * subsystems are not ported yet.
 */

#include <drivers/clk_qcom.h>
#include <io.h>
#include <kernel/delay.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <stdint.h>
#include <util.h>

#include "clock_group_qcom.h"

/*
 * The central GCC window is registered by the clock driver core; SOCCP adds the
 * AOSS reset-control and SOCCP CSR windows its reset sequence needs.
 */
register_phys_mem(MEM_AREA_IO_NSEC, AOSS_CC_BASE, AOSS_CC_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, SOCCP_CSR_BASE, SOCCP_CSR_SIZE);

/*
 * Point the SOCCP core RCG at GPLL0_OUT_EVEN with a div-1 pre-divider, which
 * runs the core at 300 MHz. SRC_SEL and SRC_DIV are updated in place rather
 * than writing a whole CFG_RCGR value, matching the reference driver, which
 * leaves the remaining CFG_RCGR fields at their reset values.
 *
 * The shared qcom_clock_set_rate() helper is not used here: it writes UPDATE as
 * a whole-register store to CMD_RCGR, which would clear ROOT_EN (bit 1). The
 * reference driver does a read-modify-write of the UPDATE bit, so do the same.
 */
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

/*
 * Enable the SOCCP bus and functional clock branches and program its core RCG,
 * in that order, as the reference driver does.
 *
 * CLK_ENABLE is written without polling CLK_OFF (so not via
 * qcom_clock_enable_cbc): the branches in this block are fed by the SOCCP RCG,
 * which is only programmed at the end of this function, so a branch is not
 * expected to report itself on yet and polling here would just time out. The
 * reference driver likewise writes CLK_ENABLE blind for all of them.
 *
 * The debug branch (GCC_SOCCP_DBG_CBCR) is deliberately left off: it is only
 * needed to attach a debugger, and the reference driver gates it on the debug
 * cookies, which OP-TEE does not consume. GCC_SOCCP_PROC_H_CBCR is skipped for
 * the same reason -- the reference only touches it on the debug path, to hold
 * the core halted.
 */
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

/*
 * Release the SOCCP core. Clocks and the RCG are already up (soccp_setup) and
 * the firmware has been authenticated and loaded, so lifting boot suppression
 * is all that is left; the core fetches its first instruction immediately.
 */
static TEE_Result soccp_enable_processor(void)
{
	struct io_pa_va csr_io = { .pa = SOCCP_CSR_BASE };
	vaddr_t csr = io_pa_or_va(&csr_io, SOCCP_CSR_SIZE);

	if (!csr)
		return TEE_ERROR_GENERIC;

	io_write32(csr + SOCCP_RVSSMP_BOOT_SUPPRESS, 0);

	return TEE_SUCCESS;
}

/*
 * Put the SOCCP through a subsystem restart before bring-up. Boot suppression
 * is asserted first because the core would otherwise start executing the moment
 * the AOSS reset is released. The config restart is pulsed inside the subsystem
 * restart to clear the SOCCP cache context.
 *
 * SS_RESTART is bit 0 of an otherwise reserved register, so it is set and
 * cleared read-modify-write, matching the reference.
 */
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

	/*
	 * The subsystem is only under reset after 9 sleep-clock cycles,
	 * (9 / 32768) * 10^6 = 275 us; round up.
	 */
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
