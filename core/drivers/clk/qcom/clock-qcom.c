// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025, Linaro Ltd
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk.h>
#include <drivers/clk_qcom.h>
#include <libfdt.h>
#include <malloc.h>
#include <stdint.h>
#include <io.h>
#include <mm/core_memprot.h>
#include <trace.h>
#include <platform_config.h>

/* CBCR register fields */
#define CBCR_BRANCH_ENABLE_BIT  BIT(0)
#define CBCR_BRANCH_OFF_BIT     BIT(31)

#if defined(PLATFORM_FLAVOR_kodiak)
#define GCC_WPSS_AHB_CLK		0x9d154
#define GCC_WPSS_AHB_BDG_MST_CLK	0x9d158
#define GCC_WPSS_RSCP_CLK		0x9d16c
#define GCC_TURING_CFG_AHB_CLK		0x45028

/* Turing */
#define TURING_CC_OFFSET			0x00800000
#define TURING_Q6SS_Q6_AXIM_CLK			0xb000
#define TURING_CENG_CLK				0x4030
#define TURING_NSPNOC_CLK			0x4040
#define TURING_Q6SS_AHBS_AON_CLK		0x10000
#define TURING_VAPSS_GDSCR			0x16000
#define TURING_VAPSS_CFG_GDSCR			0x16004
#define TURING_VAPSS_GDS_HW_CTRL		0x1600c

#define VAPSS_GDSCR_SW_COLLAPSE_MASK		0x1
#define VAPSS_GDSCR_PWR_ON_MASK			0x80000000
#define VAPSS_GDSCR_RETAIN_FF_ENABLE_MASK	0x800
#define VAPSS_CFG_GDSCR_PWR_UP_COMPLETE_MASK	0x10000
#define VAPSS_GDS_HW_STATE_MASK			0x1E
#define VAPSS_GDS_HW_STATE_SHIFT		1

#else
#error "Platform specific clock offsets not defined..."
#endif

/* Enable clock controlled by CBC soft macro */
static int clk_enable_cbc(paddr_t cbcr)
{
	uint64_t timer;

	io_write32(cbcr, CBCR_BRANCH_ENABLE_BIT);

	timer = timeout_init_us(10 * 1000);
	do {
		if (!(io_read32(cbcr) & CBCR_BRANCH_OFF_BIT))
			return 0;
		if (timeout_elapsed(timer))
			return -1;
		udelay(10);
	} while (1);
}

static int compute_cc_enable(void)
{
	struct io_pa_va turing_cc_io = { .pa = TURING_BASE + TURING_CC_OFFSET };
	vaddr_t cc_base = io_pa_or_va(&turing_cc_io, 0x20000);
	uint64_t timer = 0;
	uint32_t state = 0;
	uint32_t val = 0;
	int res = 0;

	io_clrbits32(cc_base + TURING_Q6SS_Q6_AXIM_CLK, 0x1);
	io_setbits32(cc_base + TURING_Q6SS_Q6_AXIM_CLK, 0x2);
	io_setbits32(cc_base + TURING_CENG_CLK, 0x2);
	io_setbits32(cc_base + TURING_NSPNOC_CLK, 0x2);

	res = clk_enable_cbc(cc_base + TURING_Q6SS_AHBS_AON_CLK);
	if (res)
		return res;

	io_clrbits32(cc_base + TURING_VAPSS_GDSCR,
		     VAPSS_GDSCR_SW_COLLAPSE_MASK);

	timer = timeout_init_us(10 * 1000);
	while (true) {
		val = io_read32(cc_base + TURING_VAPSS_GDSCR);
		if (val & VAPSS_GDSCR_PWR_ON_MASK)
			break;
		if (timeout_elapsed(timer))
			return -1;
		udelay(10);
	}

	timer = timeout_init_us(10 * 1000);
	while (true) {
		val = io_read32(cc_base + TURING_VAPSS_CFG_GDSCR);
		if (val & VAPSS_CFG_GDSCR_PWR_UP_COMPLETE_MASK)
			break;
		if (timeout_elapsed(timer))
			return -1;
		udelay(10);
	}

	timer = timeout_init_us(10 * 1000);
	while (true) {
		val = io_read32(cc_base + TURING_VAPSS_GDS_HW_CTRL);
		state = val & VAPSS_GDS_HW_STATE_MASK;
		state = state >> VAPSS_GDS_HW_STATE_SHIFT;
		if (state == 0)
			break;
		if (timeout_elapsed(timer))
			return -1;
		udelay(10);
	}

	io_setbits32(cc_base + TURING_VAPSS_GDSCR,
		     VAPSS_GDSCR_RETAIN_FF_ENABLE_MASK);

	io_setbits32(cc_base + TURING_VAPSS_GDSCR,
		     VAPSS_GDSCR_SW_COLLAPSE_MASK);

	timer = timeout_init_us(10 * 1000);
	while (true) {
		val = io_read32(cc_base + TURING_VAPSS_GDSCR);
		if (!(val & VAPSS_GDSCR_PWR_ON_MASK))
			break;
		if (timeout_elapsed(timer))
			return -1;
		udelay(10);
	}

	timer = timeout_init_us(10 * 1000);
	while (true) {
		val = io_read32(cc_base + TURING_VAPSS_GDS_HW_CTRL);
		state = val & VAPSS_GDS_HW_STATE_MASK;
		state = state >> VAPSS_GDS_HW_STATE_SHIFT;
		if (state == 0xA)
			break;
		if (timeout_elapsed(timer))
			return -1;
		udelay(10);
	}

	return 0;
}

TEE_Result qcom_clock_enable(enum qcom_clk_group group)
{
	struct io_pa_va base = { .pa = GCC_BASE };
	vaddr_t gcc_base = io_pa_or_va(&base, 0x100000);
	int res = 0;

	switch (group) {
	case QCOM_CLKS_WPSS:
		res = clk_enable_cbc(gcc_base + GCC_WPSS_AHB_CLK);
		if (res)
			goto timeout;
		res = clk_enable_cbc(gcc_base + GCC_WPSS_AHB_BDG_MST_CLK);
		if (res)
			goto timeout;
		res = clk_enable_cbc(gcc_base + GCC_WPSS_RSCP_CLK);
		if (res)
			goto timeout;
		break;
	case QCOM_CLKS_TURING:
		res = clk_enable_cbc(gcc_base + GCC_TURING_CFG_AHB_CLK);
		if (res)
			goto timeout;
		res = compute_cc_enable();
		if (res)
			goto timeout;
		break;
	default:
		EMSG("Unsupported clock group %d\n", group);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	return TEE_SUCCESS;

timeout:
	EMSG("Timeout trying to enable clock group %d\n", group);
	return TEE_ERROR_TIMEOUT;
}
