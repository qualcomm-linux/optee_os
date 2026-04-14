// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025, Linaro Ltd
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk.h>
#include <drivers/clk_qcom.h>
#include <io.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <trace.h>
#include <platform_config.h>

#include "clock_group_qcom.h"

/* CBCR register fields */
#define CMD_RCGR_UPDATE_BIT		BIT(0)

register_phys_mem(MEM_AREA_IO_NSEC, GCC_BASE, GCC_SIZE);

static inline bool cbcr_branch_on(uint32_t val)
{
	return !(val & CBCR_BRANCH_OFF_BIT);
}

TEE_Result qcom_clock_enable_cbc(vaddr_t cbcr)
{
	int ret = 0;

	io_setbits32(cbcr, CBCR_BRANCH_ENABLE_BIT);

	REG_POLL_TIMEOUT(cbcr, 10 * 1000, 10, &ret, cbcr_branch_on);

	if (ret < 0)
		return TEE_ERROR_TIMEOUT;

	return TEE_SUCCESS;
}

static int clk_rcgr_update(vaddr_t cfg_rcgr, vaddr_t cmd_rcgr,
			   uint32_t cfg_value)
{
	uint64_t timer;

	io_write32(cfg_rcgr, cfg_value);
	io_write32(cmd_rcgr, CMD_RCGR_UPDATE_BIT);

	timer = timeout_init_us(10 * 1000);
	while (io_read32(cmd_rcgr) & CMD_RCGR_UPDATE_BIT) {
		if (timeout_elapsed(timer))
			return -1;
		udelay(1);
	}

	return 0;
}

static int qfprom_clock_config(vaddr_t gcc_base, bool enable)
{
	vaddr_t cfg_rcgr = gcc_base + GCC_SEC_CTRL_CFG_RCGR;
	vaddr_t cmd_rcgr = gcc_base + GCC_SEC_CTRL_CMD_RCGR;
	uint32_t cfg_value = enable ? QFPROM_CLOCK_DIVIDE : 0;

	return clk_rcgr_update(cfg_rcgr, cmd_rcgr, cfg_value);
}

TEE_Result qcom_clock_enable(enum qcom_clk_group group)
{
	switch (group) {
	case QCOM_CLKS_TURING:
	case QCOM_CLKS_LPASS:
	case QCOM_CLKS_WPSS:
		return qcom_clock_enable_pas(group);
	case QCOM_CLKS_QFPROM: {
		struct io_pa_va base = { .pa = GCC_BASE };
		vaddr_t gcc_base = io_pa_or_va(&base, GCC_SIZE);
		int res = qfprom_clock_config(gcc_base, true);

		if (res) {
			EMSG("Timeout trying to enable clock group %d\n",
			     group);
			return TEE_ERROR_TIMEOUT;
		}
		return TEE_SUCCESS;
	}
	default:
		EMSG("Unsupported clock group %d\n", group);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	return TEE_SUCCESS;
}

TEE_Result qcom_clock_disable(enum qcom_clk_group group)
{
	struct io_pa_va base = { .pa = GCC_BASE };
	vaddr_t gcc_base = io_pa_or_va(&base, 0x100000);
	int res = 0;

	switch (group) {
	case QCOM_CLKS_QFPROM:
		res = qfprom_clock_config(gcc_base, false);
		if (res) {
			EMSG("Timeout trying to disable clock group %d\n",
			     group);
			return TEE_ERROR_TIMEOUT;
		}
		break;
	default:
		EMSG("Unsupported clock group %d\n", group);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	return TEE_SUCCESS;
}
