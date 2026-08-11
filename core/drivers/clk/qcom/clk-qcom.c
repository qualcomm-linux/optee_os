// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025, Linaro Ltd
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk.h>
#include <drivers/clk_qcom.h>
#include <io.h>
#include <mm/core_mmu.h>

register_phys_mem(MEM_AREA_IO_NSEC, GCC_BASE, GCC_SIZE);

#ifdef CFG_QCOM_CLK_BSP
#include <config.h>
#include <drivers/clk_qcom_bsp.h>
#include <initcall.h>
#include <malloc.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <string.h>
#include <trace.h>

#include "clk_qcom_vreg.h"
#include "clock_group_qcom.h"
#endif

#define CBCR_BRANCH_ENABLE_BIT		BIT(0)
#define CBCR_HW_CTL_ENABLE_BIT		BIT(1)
#define CBCR_BRANCH_OFF_BIT		BIT(31)

/* Lucid-EVO PLL register offsets, relative to the PLL register block base. */
#define PLL_MODE			0x0
#define PLL_OPMODE			0x4
#define PLL_L_VAL			0x10
#define PLL_ALPHA_VAL			0x14
#define PLL_USER_CTL			0x18
#define PLL_USER_CTL_U			0x1c
#define PLL_CONFIG_CTL			0x20
#define PLL_CONFIG_CTL_U		0x24
#define PLL_CONFIG_CTL_U1		0x28
#define PLL_TEST_CTL			0x2c
#define PLL_TEST_CTL_U			0x30
#define PLL_TEST_CTL_U1			0x34
#define PLL_TEST_CTL_U2			0x38

/* PLL_MODE fields */
#define PLL_MODE_OUTCTRL		BIT(0)
#define PLL_MODE_RESET_N		BIT(2)
#define PLL_MODE_LOCK_DET		BIT(31)

/* PLL_OPMODE values */
#define PLL_OPMODE_RUN			0x1

/* PLL_L_VAL fields */
#define PLL_L_VAL_L_MASK		0x0000ffff
#define PLL_L_VAL_CAL_L_SHIFT		16
#define PLL_L_VAL_CAL_L_MASK		0xffff0000

/*
 * Lucid-OLE PLL_L_VAL fields, packed differently from Lucid-EVO: an 8-bit
 * L value (bits 0-7) and an 8-bit process-cal value (bits 16-23), at the
 * same register offset as EVO's 16-bit L (bits 0-15) + 16-bit cal (bits
 * 16-31) fields. Bits 24-31 (ring-osc cal) are not written by
 * HAL_clk_LucidolePLLConfigPLL/SetCalConfig and are left alone here too.
 */
#define PLL_OLE_L_VAL_L_MASK		0x000000ff
#define PLL_OLE_L_VAL_PROCESS_CAL_SHIFT	16
#define PLL_OLE_L_VAL_PROCESS_CAL_MASK	0x00ff0000

/* PLL_USER_CTL fields */
#define PLL_USER_CTL_PLLOUT_MAIN_EN	BIT(0)
#define PLL_USER_CTL_PRE_DIV_SHIFT	22
#define PLL_USER_CTL_PRE_DIV_MASK	0x01c00000
#define PLL_USER_CTL_POST_DIV_ODD_MASK	0x0003c000
#define PLL_USER_CTL_POST_DIV_EVEN_MASK	0x00003c00
#define PLL_USER_CTL_FRAC_FORMAT_SEL	BIT(28)

/* PLL_USER_CTL_U fields */
#define PLL_USER_CTL_U_FINE_LOCK_DET	BIT(0)

static inline bool cbcr_branch_on(uint32_t val)
{
	return !(val & CBCR_BRANCH_OFF_BIT);
}

TEE_Result qcom_clock_enable_cbc(vaddr_t cbcr)
{
	int ret = 0;

	io_setbits32(cbcr, CBCR_BRANCH_ENABLE_BIT);

	/*
	 * In hardware clock-control mode (HW_CTL set) CLK_OFF is driven by HW,
	 * not the software CLK_ENABLE write, so skip the poll to avoid
	 * spinning.
	 */
	if (io_read32(cbcr) & CBCR_HW_CTL_ENABLE_BIT)
		return TEE_SUCCESS;

	REG_POLL_TIMEOUT(cbcr, 10 * 1000, 10, &ret, cbcr_branch_on);

	if (ret < 0)
		return TEE_ERROR_TIMEOUT;

	return TEE_SUCCESS;
}

static inline bool pll_locked(uint32_t val)
{
	return val & PLL_MODE_LOCK_DET;
}

TEE_Result qcom_lucidevo_pll_enable(vaddr_t pll_base,
				    const struct qcom_lucidevo_pll_config *cfg)
{
	uint32_t user_val = 0;
	int ret = 0;

	/* Reg settings: program the static PLL trim/config registers. */
	io_write32(pll_base + PLL_CONFIG_CTL, cfg->config_ctl);
	io_write32(pll_base + PLL_CONFIG_CTL_U, cfg->config_ctl_u);
	io_write32(pll_base + PLL_CONFIG_CTL_U1, cfg->config_ctl_u1);
	io_write32(pll_base + PLL_USER_CTL, cfg->user_ctl);
	io_write32(pll_base + PLL_USER_CTL_U, cfg->user_ctl_u);

	/* ConfigPLL: program L value and fractional value. */
	io_mask32(pll_base + PLL_L_VAL, cfg->l_val, PLL_L_VAL_L_MASK);
	io_write32(pll_base + PLL_ALPHA_VAL, cfg->alpha_val);

	/* Select fractional format and program the pre-/post-div ratios. */
	user_val = io_read32(pll_base + PLL_USER_CTL);
	if (cfg->frac_mode_mn)
		user_val |= PLL_USER_CTL_FRAC_FORMAT_SEL;
	else
		user_val &= ~PLL_USER_CTL_FRAC_FORMAT_SEL;

	user_val &= ~(PLL_USER_CTL_PRE_DIV_MASK |
		      PLL_USER_CTL_POST_DIV_ODD_MASK |
		      PLL_USER_CTL_POST_DIV_EVEN_MASK);
	if (cfg->pre_div >= 1 && cfg->pre_div <= 8)
		user_val |= SHIFT_U32(cfg->pre_div - 1,
				      PLL_USER_CTL_PRE_DIV_SHIFT) &
			    PLL_USER_CTL_PRE_DIV_MASK;
	io_write32(pll_base + PLL_USER_CTL, user_val);

	/* Always use fine-grained lock detection. */
	io_setbits32(pll_base + PLL_USER_CTL_U, PLL_USER_CTL_U_FINE_LOCK_DET);

	/* SetCalConfig: program the calibration L value. */
	io_mask32(pll_base + PLL_L_VAL,
		  SHIFT_U32(cfg->cal_l_val, PLL_L_VAL_CAL_L_SHIFT),
		  PLL_L_VAL_CAL_L_MASK);

	/* Enable: select RUN opmode and take the PLL out of reset. */
	io_write32(pll_base + PLL_OPMODE, PLL_OPMODE_RUN);
	io_setbits32(pll_base + PLL_MODE, PLL_MODE_RESET_N);

	/* Wait for the PLL to lock. */
	REG_POLL_TIMEOUT(pll_base + PLL_MODE, 10 * 1000, 10, &ret, pll_locked);
	if (ret < 0)
		return TEE_ERROR_TIMEOUT;

	/* Enable PLL outputs and the main output. */
	io_setbits32(pll_base + PLL_MODE, PLL_MODE_OUTCTRL);
	io_setbits32(pll_base + PLL_USER_CTL, PLL_USER_CTL_PLLOUT_MAIN_EN);

	return TEE_SUCCESS;
}

TEE_Result qcom_lucidole_pll_enable(vaddr_t pll_base,
				    const struct qcom_lucidole_pll_config *cfg)
{
	uint32_t user_val = 0;
	int ret = 0;

	/* Reg settings: program the static PLL trim/config registers. */
	io_write32(pll_base + PLL_CONFIG_CTL, cfg->config_ctl);
	io_write32(pll_base + PLL_CONFIG_CTL_U, cfg->config_ctl_u);
	io_write32(pll_base + PLL_CONFIG_CTL_U1, cfg->config_ctl_u1);
	io_write32(pll_base + PLL_TEST_CTL, cfg->test_ctl);
	io_write32(pll_base + PLL_TEST_CTL_U, cfg->test_ctl_u);
	io_write32(pll_base + PLL_TEST_CTL_U1, cfg->test_ctl_u1);
	io_write32(pll_base + PLL_TEST_CTL_U2, cfg->test_ctl_u2);
	io_write32(pll_base + PLL_USER_CTL, cfg->user_ctl);
	io_write32(pll_base + PLL_USER_CTL_U, cfg->user_ctl_u);

	/* ConfigPLL: program L value and fractional value. */
	io_mask32(pll_base + PLL_L_VAL, cfg->l_val, PLL_OLE_L_VAL_L_MASK);
	io_write32(pll_base + PLL_ALPHA_VAL, cfg->alpha_val);

	/* Select fractional format and program the pre-/post-div ratios. */
	user_val = io_read32(pll_base + PLL_USER_CTL);
	if (cfg->frac_mode_mn)
		user_val |= PLL_USER_CTL_FRAC_FORMAT_SEL;
	else
		user_val &= ~PLL_USER_CTL_FRAC_FORMAT_SEL;

	user_val &= ~(PLL_USER_CTL_PRE_DIV_MASK |
		      PLL_USER_CTL_POST_DIV_ODD_MASK |
		      PLL_USER_CTL_POST_DIV_EVEN_MASK);
	if (cfg->pre_div >= 1 && cfg->pre_div <= 8)
		user_val |= SHIFT_U32(cfg->pre_div - 1,
				      PLL_USER_CTL_PRE_DIV_SHIFT) &
			    PLL_USER_CTL_PRE_DIV_MASK;
	io_write32(pll_base + PLL_USER_CTL, user_val);

	/* Always use fine-grained lock detection. */
	io_setbits32(pll_base + PLL_USER_CTL_U, PLL_USER_CTL_U_FINE_LOCK_DET);

	/* SetCalConfig: program the process-cal L value. */
	io_mask32(pll_base + PLL_L_VAL,
		  SHIFT_U32(cfg->cal_l_val, PLL_OLE_L_VAL_PROCESS_CAL_SHIFT),
		  PLL_OLE_L_VAL_PROCESS_CAL_MASK);

	/* Enable: select RUN opmode and take the PLL out of reset. */
	io_write32(pll_base + PLL_OPMODE, PLL_OPMODE_RUN);
	io_setbits32(pll_base + PLL_MODE, PLL_MODE_RESET_N);

	/* Wait for the PLL to lock. */
	REG_POLL_TIMEOUT(pll_base + PLL_MODE, 10 * 1000, 10, &ret, pll_locked);
	if (ret < 0)
		return TEE_ERROR_TIMEOUT;

	/* Enable PLL outputs and the main output. */
	io_setbits32(pll_base + PLL_MODE, PLL_MODE_OUTCTRL);
	io_setbits32(pll_base + PLL_USER_CTL, PLL_USER_CTL_PLLOUT_MAIN_EN);

	return TEE_SUCCESS;
}

TEE_Result qcom_clock_set_rate(vaddr_t cfg_rcgr, vaddr_t cmd_rcgr,
			       uint32_t cfg_value)
{
	uint32_t val = 0;

	io_write32(cfg_rcgr, cfg_value);
	io_write32(cmd_rcgr, CMD_RCGR_UPDATE_BIT);

	if (IO_READ32_POLL_TIMEOUT(cmd_rcgr, val, !(val & CMD_RCGR_UPDATE_BIT),
				   1, 10 * 1000))
		return TEE_ERROR_TIMEOUT;

	return TEE_SUCCESS;
}

TEE_Result qcom_clock_enable(enum qcom_clk_group group)
{
	switch (group) {
	case QCOM_CLKS_TURING:
	case QCOM_CLKS_TURING1:
	case QCOM_CLKS_TURING2:
	case QCOM_CLKS_TURING3:
	case QCOM_CLKS_LPASS:
	case QCOM_CLKS_WPSS:
	case QCOM_CLKS_GPDSP0:
	case QCOM_CLKS_GPDSP1:
	case QCOM_CLKS_SOCCP:
	case QCOM_CLKS_HPASS0:
	case QCOM_CLKS_HPASS1:
	case QCOM_CLKS_HPASS2:
		return qcom_clock_enable_pas(group);
	default:
		EMSG("Unsupported clock group %d\n", group);
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

#ifdef CFG_QCOM_CLK_BSP
/* Largest offset from CMD_RCGR the walker touches (last DFS perf-D bank). */
static uint32_t qcom_rcg_window(uint16_t dfs_states)
{
	return QCOM_RCG_PERF_D_DFSR_REG_OFFSET +
	       0x4 * (dfs_states ? dfs_states - 1 : 0);
}

/*
 * Resolve a BSP register address to a VA. Windows are registered statically by
 * the platform BSP (register_phys_mem), so this is a lookup, not a mapping.
 * @pa must already have been bounds-checked against @win.
 */
static vaddr_t qcom_clk_reg_va(const struct qcom_clk_window *win, paddr_t pa)
{
	struct io_pa_va io = { };
	vaddr_t base = 0;

	if (!win || !win->pa || !win->size)
		return 0;

	io.pa = win->pa;

	base = io_pa_or_va(&io, win->size);
	if (!base)
		return 0;

	return base + (pa - win->pa);
}

/* True if the word at @pa lies wholly inside @win. */
static bool qcom_clk_reg_in_window(const struct qcom_clk_window *win,
				   paddr_t pa, size_t span)
{
	if (!win || !win->size || pa < win->pa)
		return false;

	return pa - win->pa + span + sizeof(uint32_t) <= win->size;
}

/*
 * Bounds-check every address a domain hands the walker against its own window,
 * so a bad BSP row cannot turn into an out-of-window register access.
 */
static bool qcom_domain_addrs_valid(const struct qcom_clk_domain *domain,
				    bool need_rcg)
{
	const struct qcom_clk_window *win = domain->window;

	if (!win || !win->size)
		return false;

	if (!qcom_clk_reg_in_window(win, domain->cbcr_addr, 0))
		return false;

	if (!qcom_clk_reg_in_window(win, domain->vote_reg_addr, 0))
		return false;

	if (!need_rcg)
		return true;

	if (!domain->cmd_rcgr_addr)
		return false;

	return qcom_clk_reg_in_window(win, domain->cmd_rcgr_addr,
				      qcom_rcg_window(domain->dfs_states));
}

/*
 * Program SRC_SEL/SRC_DIV, and the MND divider when @cfg->m != 0 && m < n.
 */
static void qcom_config_mux_offs(vaddr_t cgr,
				 const struct qcom_clk_mux_config *cfg,
				 uint32_t cfg_off, uint32_t m_off,
				 uint32_t n_off, uint32_t d_off)
{
	uint32_t half_div = cfg->div2x ? cfg->div2x - 1 : 0;
	uint32_t val = io_read32(cgr + cfg_off);

	val &= ~(QCOM_RCG_CFG_SRC_SEL_FMSK | QCOM_RCG_CFG_SRC_DIV_FMSK |
		 QCOM_RCG_CFG_MODE_FMSK | QCOM_RCG_CFG_HW_CLK_CONTROL_FMSK);

	val |= SHIFT_U32(cfg->mux_sel, QCOM_RCG_CFG_SRC_SEL_SHFT) &
	       QCOM_RCG_CFG_SRC_SEL_FMSK;
	val |= SHIFT_U32(half_div, QCOM_RCG_CFG_SRC_DIV_SHFT) &
	       QCOM_RCG_CFG_SRC_DIV_FMSK;

	if (cfg->m != 0 && cfg->m < cfg->n) {
		io_write32(cgr + m_off, cfg->m);
		io_write32(cgr + n_off, ~(cfg->n - cfg->m));
		io_write32(cgr + d_off, ~cfg->n);

		val |= SHIFT_U32(QCOM_RCG_CFG_DUAL_EDGE_MODE_VAL,
				 QCOM_RCG_CFG_MODE_SHFT) &
		       QCOM_RCG_CFG_MODE_FMSK;
	}

	io_write32(cgr + cfg_off, val);
}

/*
 * Scan @domain's frequency plan for the lowest configured rate >= @freq_hz
 * (meet-or-exceed). Returns NULL if the plan's top rate is below @freq_hz.
 */
static const struct qcom_clk_mux_config *
qcom_find_config(const struct qcom_clk_domain *domain, uint32_t freq_hz)
{
	const struct qcom_clk_mux_config *at_least = NULL;
	uint32_t at_least_hz = 0xFFFFFFFF;
	uint32_t i = 0;

	for (i = 0; i < domain->n_configs; i++) {
		const struct qcom_clk_mux_config *c = &domain->configs[i];

		if (c->freq_hz == freq_hz)
			return c;

		if (c->freq_hz > freq_hz && c->freq_hz < at_least_hz) {
			at_least_hz = c->freq_hz;
			at_least = c;
		}
	}

	return at_least;
}

/*
 * Vote the RCG's upcoming PLL source on before switching the mux onto it
 * (the PLL itself is already configured by an earlier boot image). Direct
 * GCC write, not RPMh. XO has no source-vote entry and is skipped; the vote
 * is left in place afterwards (no per-rate un-vote).
 */
static void qcom_clk_src_vote(const struct qcom_clk_window *win,
			      uint32_t mux_sel)
{
	const struct qcom_clk_bsp *bsp = qcom_clk_bsp_get();
	uint32_t i = 0;

	if (!bsp || !bsp->src_votes)
		return;

	for (i = 0; i < bsp->n_src_votes; i++) {
		const struct qcom_clk_src_vote *sv = &bsp->src_votes[i];
		vaddr_t vote = 0;

		/*
		 * Each quadrant controller has its own GPLL0 reachable through
		 * the same mux index, so the window is part of the key.
		 */
		if (sv->mux_sel != mux_sel || sv->window != win)
			continue;

		if (!qcom_clk_reg_in_window(win, sv->vote_reg_addr, 0))
			return;

		vote = qcom_clk_reg_va(win, sv->vote_reg_addr);
		if (!vote)
			return;

		io_setbits32(vote, BIT(sv->vote_bit));
		return;
	}
}

/*
 * Program @domain's RCG to the lowest planned rate >= @freq_hz, voting the
 * CX/MX rail and the row's PLL source around the change. @corner tracks the
 * caller's currently-voted corner and is updated in place.
 */
static TEE_Result qcom_domain_set_rate(const struct qcom_clk_domain *domain,
				       uint32_t freq_hz, uint16_t *corner,
				       uint32_t *res_hz)
{
	const struct qcom_clk_mux_config *cfg = NULL;
	vaddr_t cgr = 0;
	uint32_t val = 0;
	uint16_t prev = corner ? *corner : 0;
	uint16_t next = prev;
	TEE_Result res = TEE_SUCCESS;

	if (!domain || !qcom_domain_addrs_valid(domain, true))
		return TEE_ERROR_BAD_PARAMETERS;

	cfg = qcom_find_config(domain, freq_hz);
	if (!cfg)
		return TEE_ERROR_ITEM_NOT_FOUND;

	cgr = qcom_clk_reg_va(domain->window, domain->cmd_rcgr_addr);
	if (!cgr)
		return TEE_ERROR_GENERIC;

	if (corner && cfg->cx_level)
		next = cfg->cx_level;

	/* Raise the rail before speeding up; abort on failure. */
	if (next > prev) {
		res = qcom_clk_vreg_vote(prev, next);
		if (res)
			return res;
		*corner = next;
	}

	qcom_clk_src_vote(domain->window, cfg->mux_sel);

	qcom_config_mux_offs(cgr, cfg, QCOM_RCG_CFG_REG_OFFSET,
			     QCOM_RCG_M_REG_OFFSET, QCOM_RCG_N_REG_OFFSET,
			     QCOM_RCG_D_REG_OFFSET);

	io_setbits32(cgr, QCOM_RCG_CMD_CFG_UPDATE_FMSK);

	if (IO_READ32_POLL_TIMEOUT(cgr, val,
				   !(val & QCOM_RCG_CMD_CFG_UPDATE_FMSK),
				   1, 10 * 1000))
		return TEE_ERROR_TIMEOUT;

	/* Lower the rail after slowing down; best-effort (stays safe-high). */
	if (next < prev && !qcom_clk_vreg_vote(prev, next))
		*corner = next;

	if (res_hz)
		*res_hz = cfg->freq_hz;

	return TEE_SUCCESS;
}

static TEE_Result
qcom_clock_domain_enable_dfs(const struct qcom_clk_domain *domain)
{
	vaddr_t cgr = 0;
	uint32_t i = 0;

	if (!domain || !domain->dfs_states ||
	    !qcom_domain_addrs_valid(domain, true))
		return TEE_ERROR_BAD_PARAMETERS;

	cgr = qcom_clk_reg_va(domain->window, domain->cmd_rcgr_addr);
	if (!cgr)
		return TEE_ERROR_GENERIC;

	for (i = 0; i < domain->n_configs; i++) {
		const struct qcom_clk_mux_config *c = &domain->configs[i];
		uint32_t perf = 0;

		if (c->dfs_idx == QCOM_DFS_NA ||
		    c->dfs_idx >= domain->dfs_states)
			continue;

		perf = 0x4 * c->dfs_idx;
		qcom_config_mux_offs(cgr, c,
				     QCOM_RCG_PERF_DFSR_REG_OFFSET + perf,
				     QCOM_RCG_PERF_M_DFSR_REG_OFFSET + perf,
				     QCOM_RCG_PERF_N_DFSR_REG_OFFSET + perf,
				     QCOM_RCG_PERF_D_DFSR_REG_OFFSET + perf);
	}

	/* Hand rate control to the hardware DFS machine. */
	io_write32(cgr + QCOM_RCG_CMD_DFSR_REG_OFFSET,
		   QCOM_RCG_CMD_DFSR_HW_CLK_CONTROL_FMSK |
		   QCOM_RCG_CMD_DFSR_DFS_EN_FMSK);

	return TEE_SUCCESS;
}

/*
 * QUP SE clk provider: registers each domain as a struct clk with no parent
 * so a TEE-side bus (SPI/I2C) consumer drives it via the common clk API.
 * enable/disable gate the shared vote register (QUP SEs have no GDSC);
 * set_rate tracks each SE's voted corner in its own priv.
 */
struct qcom_qup_clk {
	const struct qcom_clk_domain *domain;
	struct clk *clk;
	uint16_t corner;	/* CX/MX corner currently voted for this SE */
	uint32_t rate;		/* last resolved output rate, 0 until set */
};

/* Priv array parallel to qcom_clk_bsp_get()->domains. */
static struct qcom_qup_clk *qcom_qup_clks;

static TEE_Result qcom_qup_clk_set_rate(struct clk *clk, unsigned long rate,
					unsigned long parent_rate __unused)
{
	struct qcom_qup_clk *qup = clk->priv;
	uint32_t res_hz = 0;
	TEE_Result res = TEE_SUCCESS;

	res = qcom_domain_set_rate(qup->domain, rate, &qup->corner, &res_hz);
	if (res)
		return res;

	qup->rate = res_hz;
	return TEE_SUCCESS;
}

static unsigned long qcom_qup_clk_get_rate(struct clk *clk,
					   unsigned long parent_rate __unused)
{
	struct qcom_qup_clk *qup = clk->priv;

	return qup->rate;
}

static inline bool cbcr_branch_off(uint32_t val)
{
	return val & CBCR_BRANCH_OFF_BIT;
}

/*
 * Every QUP SE branch on this target gates through a shared vote register
 * rather than its own CBCR's CLK_ENABLE bit. CLK_OFF is still polled on the
 * SE's own CBCR regardless, since the vote register has no status bit.
 */
static TEE_Result qcom_qup_clk_enable(struct clk *clk)
{
	struct qcom_qup_clk *qup = clk->priv;
	const struct qcom_clk_domain *dom = qup->domain;
	vaddr_t cbcr = 0;
	vaddr_t vote = 0;
	int ret = 0;

	if (!dom->cbcr_addr || !dom->vote_reg_addr ||
	    !qcom_domain_addrs_valid(dom, false))
		return TEE_ERROR_BAD_STATE;

	cbcr = qcom_clk_reg_va(dom->window, dom->cbcr_addr);
	vote = qcom_clk_reg_va(dom->window, dom->vote_reg_addr);
	if (!cbcr || !vote)
		return TEE_ERROR_BAD_STATE;

	io_setbits32(vote, BIT(dom->vote_bit));

	if (io_read32(cbcr) & CBCR_HW_CTL_ENABLE_BIT)
		return TEE_SUCCESS;

	REG_POLL_TIMEOUT(cbcr, 10 * 1000, 10, &ret, cbcr_branch_on);

	return ret < 0 ? TEE_ERROR_TIMEOUT : TEE_SUCCESS;
}

static void qcom_qup_clk_disable(struct clk *clk)
{
	struct qcom_qup_clk *qup = clk->priv;
	const struct qcom_clk_domain *dom = qup->domain;
	vaddr_t cbcr = 0;
	vaddr_t vote = 0;
	int ret = 0;

	if (!dom->cbcr_addr || !dom->vote_reg_addr ||
	    !qcom_domain_addrs_valid(dom, false))
		return;

	cbcr = qcom_clk_reg_va(dom->window, dom->cbcr_addr);
	vote = qcom_clk_reg_va(dom->window, dom->vote_reg_addr);
	if (!cbcr || !vote)
		return;

	io_clrbits32(vote, BIT(dom->vote_bit));

	/* HW_CTL drives the off-state; polling CLK_OFF would spin. */
	if (io_read32(cbcr) & CBCR_HW_CTL_ENABLE_BIT)
		return;

	REG_POLL_TIMEOUT(cbcr, 10 * 1000, 10, &ret, cbcr_branch_off);
}

static const struct clk_ops qcom_qup_clk_ops = {
	.enable = qcom_qup_clk_enable,
	.disable = qcom_qup_clk_disable,
	.set_rate = qcom_qup_clk_set_rate,
	.get_rate = qcom_qup_clk_get_rate,
};

static TEE_Result qcom_qup_clocks_register(void)
{
	const struct qcom_clk_bsp *bsp = qcom_clk_bsp_get();
	uint32_t i = 0;
	TEE_Result res = TEE_SUCCESS;

	if (qcom_qup_clks)
		return TEE_SUCCESS;

	if (!bsp || !bsp->n_domains)
		return TEE_ERROR_BAD_STATE;

	qcom_qup_clks = calloc(bsp->n_domains, sizeof(*qcom_qup_clks));
	if (!qcom_qup_clks)
		return TEE_ERROR_OUT_OF_MEMORY;

	for (i = 0; i < bsp->n_domains; i++) {
		struct qcom_qup_clk *qup = &qcom_qup_clks[i];
		struct clk *clk = NULL;

		clk = clk_alloc(bsp->domains[i].name, &qcom_qup_clk_ops,
				NULL, 0);
		if (!clk)
			return TEE_ERROR_OUT_OF_MEMORY;

		qup->domain = &bsp->domains[i];
		qup->clk = clk;
		clk->priv = qup;

		res = clk_register(clk);
		if (res) {
			clk_free(clk);
			return res;
		}
	}

	return TEE_SUCCESS;
}

/*
 * Look up a QUP SE clk by name. This platform has no secure DT, so a bus
 * consumer acquires the clk this way instead of clk_dt_get_by_name().
 */
TEE_Result qcom_clk_get_by_name(const char *name, struct clk **out)
{
	const struct qcom_clk_bsp *bsp = qcom_clk_bsp_get();
	uint32_t i = 0;

	if (!name || !out)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!qcom_qup_clks || !bsp)
		return TEE_ERROR_BAD_STATE;

	for (i = 0; i < bsp->n_domains; i++) {
		if (!strcmp(qcom_qup_clks[i].domain->name, name)) {
			*out = qcom_qup_clks[i].clk;
			return TEE_SUCCESS;
		}
	}

	return TEE_ERROR_ITEM_NOT_FOUND;
}

/* Enable hardware DFS on a registered QUP SE clk (no clk_ops equivalent). */
TEE_Result qcom_clk_enable_dfs(struct clk *clk)
{
	struct qcom_qup_clk *qup = NULL;

	if (!clk || clk->ops != &qcom_qup_clk_ops)
		return TEE_ERROR_BAD_PARAMETERS;

	qup = clk->priv;
	return qcom_clock_domain_enable_dfs(qup->domain);
}

/*
 * Return the frequency plan (domain) backing @clk, or NULL if @clk is not a
 * QUP SE clk this driver registered. A bus consumer walks domain->configs[]
 * to run its own divider search, then drives the clk via the common clk API.
 */
const struct qcom_clk_domain *qcom_clk_get_domain(struct clk *clk)
{
	struct qcom_qup_clk *qup = NULL;

	if (!clk || clk->ops != &qcom_qup_clk_ops)
		return NULL;

	qup = clk->priv;
	return qup->domain;
}

driver_init(qcom_qup_clocks_register);
#endif /* CFG_QCOM_CLK_BSP */
