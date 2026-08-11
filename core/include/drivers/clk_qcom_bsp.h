/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Slim data contract for the QUPv3 serial-engine clock BSP consumed by the
 * OP-TEE QUP SE clock provider (clk-qcom.c). Only the fields needed to
 * program an RCG mux/divider/MND and its DFS banks are kept; source
 * selection (mux_sel) is pre-resolved.
 */

#ifndef _CLK_QCOM_BSP_H_
#define _CLK_QCOM_BSP_H_

#include <stdint.h>
#include <types_ext.h>

/*
 * One frequency configuration row for an RCG.
 *
 * @freq_hz  Output frequency in Hz.
 * @mux_sel  RCG source-select index (CFG_RCGR SRC_SEL), pre-resolved.
 * @div2x    Twice the half-integer source divider; the register encodes
 *           SRC_DIV as (div2x - 1), 0 means no divide.
 * @m, @n    MND fractional-divider numerator/denominator; used only when
 *           (m != 0 && m < n).
 * @dfs_idx  DFS performance-state index for this row, or 0xFF if none.
 * @cx_level CX/MX voltage corner this rate requires (raw rail_voltage_level,
 *           e.g. MIN_SVS 0x30); voted on cx.lvl/mx.lvl around the mux
 *           program. 0 means no vote for this row.
 */
struct qcom_clk_mux_config {
	uint32_t freq_hz;
	uint32_t mux_sel;
	uint16_t div2x;
	uint32_t m;
	uint32_t n;
	uint8_t  dfs_idx;
	uint16_t cx_level;
};

#define QCOM_DFS_NA		0xFF

/*
 * One register window a clock domain's registers live in.
 *
 * Lemans routes every QUP register through the single central GCC window, but
 * nord splits its QUP SEs across per-quadrant clock controllers (SE_GCC /
 * NE_GCC) alongside the central GCC. Each domain therefore names the window
 * its offsets are relative to instead of assuming one global base.
 *
 * @pa    Physical base address of the window.
 * @size  Window size; also bounds-checks each domain's offsets.
 */
struct qcom_clk_window {
	paddr_t pa;
	size_t size;
};

/*
 * A clock domain (one RCG) and its frequency plan.
 *
 * Register locations are full physical addresses, matching the
 * HWIO_<reg>_ADDR values in the reference clock driver's HWIO header, so a row
 * can be read against the hardware documentation without first working out
 * which base a number is relative to. @window only bounds-checks them and
 * supplies the mapping.
 *
 * @name             Clock name, e.g. "gcc_qupv3_wrap0_s0_clk".
 * @window           Register window @cmd_rcgr_addr / @cbcr_addr /
 *                   @vote_reg_addr fall inside.
 * @cmd_rcgr_addr    Physical address of the domain's CMD_RCGR; the CFG/M/N/D
 *                   and DFS banks sit at fixed offsets from it.
 *                   0 marks a branch-only domain (no RCG, no frequency plan):
 *                   enable/disable only, set_rate and DFS are rejected.
 * @cbcr_addr        Physical address of the SE's branch control register.
 *                   Its distance from @cmd_rcgr_addr varies by target and even
 *                   by wrapper, so it is never derived. CLK_OFF is always
 *                   polled here regardless of vote-vs-direct gating. QUP SEs
 *                   have no GDSC, so no power domain is modelled.
 * @vote_reg_addr    Physical address of the shared vote register this SE's
 *                   branch enable is gated through. Nonzero on every supported
 *                   target -- no SE gates directly off its own CBCR.
 * @vote_bit         Bit position (0-31) of this SE within @vote_reg_addr.
 *                   Assignment is per-target/per-SE, so both fields are
 *                   supplied rather than derived.
 * @dfs_states       Number of DFS performance states the RCG supports (8 for
 *                   QUP SE); 0 means DFS is not supported.
 * @configs          Frequency-configuration array of @n_configs rows.
 * @n_configs        Number of rows in @configs; the walker iterates by this
 *                   count (the array is not sentinel-terminated).
 */
struct qcom_clk_domain {
	const char *name;
	const struct qcom_clk_window *window;
	paddr_t cmd_rcgr_addr;
	paddr_t cbcr_addr;
	paddr_t vote_reg_addr;
	uint8_t vote_bit;
	uint16_t dfs_states;
	const struct qcom_clk_mux_config *configs;
	uint32_t n_configs;
};

/*
 * Vote entry for one RCG source (upstream PLL) selectable via @mux_sel. RCGs
 * in this driver have no parent clk; before switching onto a PLL source the
 * walker places a branch vote so the (already-configured) PLL is held on.
 * Direct register write, not RPMh.
 *
 * @window           Register window @vote_reg_addr falls inside. Each
 *                   quadrant controller has its own GPLL0 and its own PLL
 *                   vote register, so the window is part of the lookup key.
 * @mux_sel          RCG SRC_SEL index this entry describes.
 * @vote_reg_addr    Physical address of the PLL branch-enable vote register.
 * @vote_bit         Bit position (0-31) of this PLL within that register.
 *
 * A source needing no vote (e.g. XO) has no entry, and the walker skips
 * voting for rows that select it.
 */
struct qcom_clk_src_vote {
	const struct qcom_clk_window *window;
	uint32_t mux_sel;
	paddr_t vote_reg_addr;
	uint8_t vote_bit;
};
/*
 * The per-target BSP: walker-visible clock domains plus the PLL source-vote
 * table keyed by @mux_sel. Provided by the target's platform translation
 * unit. @src_votes may be NULL/0 on a target needing no source voting.
 */
struct qcom_clk_bsp {
	const struct qcom_clk_domain *domains;
	uint32_t n_domains;
	const struct qcom_clk_src_vote *src_votes;
	uint32_t n_src_votes;
};

const struct qcom_clk_bsp *qcom_clk_bsp_get(void);

#endif /* _CLK_QCOM_BSP_H_ */
