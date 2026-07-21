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

/*
 * One frequency configuration row for an RCG.
 *
 * @freq_hz  Output frequency in Hz. A terminating row has freq_hz == 0.
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

#define QCOM_CLK_DFS_IDX_NONE		0xFF

/*
 * A clock domain (one RCG) and its frequency plan.
 *
 * @name             Clock name, e.g. "gcc_qupv3_wrap0_s0_clk".
 * @cmd_rcgr_offset  GCC-relative offset of the domain's CMD_RCGR; the
 *                   CFG/M/N/D and DFS banks sit at fixed offsets from it.
 * @cbcr_offset      GCC-relative offset of the SE's branch control register
 *                   (CBCR sits 8 bytes below CMD_RCGR on QUPv3). CLK_OFF is
 *                   always polled here regardless of vote-vs-direct gating.
 *                   QUP SEs have no GDSC, so no power domain is modelled.
 * @vote_reg_offset  GCC-relative offset of the shared vote register this
 *                   SE's branch enable is gated through. Always nonzero on
 *                   lemans -- no SE gates directly off its own CBCR.
 * @vote_bit         Bit position (0-31) of this SE within @vote_reg_offset.
 *                   Assignment is per-target/per-SE, so both fields are
 *                   supplied rather than derived.
 * @dfs_states       Number of DFS performance states the RCG supports (8 for
 *                   QUP SE); 0 means DFS is not supported.
 * @configs          Frequency-configuration array, freq_hz == 0 terminated.
 * @n_configs        Number of usable rows in @configs (excluding terminator).
 */
struct qcom_clk_domain {
	const char *name;
	uint32_t cmd_rcgr_offset;
	uint32_t cbcr_offset;
	uint32_t vote_reg_offset;
	uint8_t vote_bit;
	uint16_t dfs_states;
	const struct qcom_clk_mux_config *configs;
	uint32_t n_configs;
};

/*
 * Vote entry for one RCG source (upstream PLL) selectable via @mux_sel. RCGs
 * in this driver have no parent clk; before switching onto a PLL source the
 * walker places a branch vote so the (already-configured) PLL is held on.
 * Direct GCC register write, not RPMh.
 *
 * @mux_sel          RCG SRC_SEL index this entry describes.
 * @vote_reg_offset  GCC-relative offset of the PLL branch-enable vote
 *                   register.
 * @vote_bit         Bit position (0-31) of this PLL within that register.
 *
 * A source needing no vote (e.g. XO) has no entry, and the walker skips
 * voting for rows that select it.
 */
struct qcom_clk_src_vote {
	uint32_t mux_sel;
	uint32_t vote_reg_offset;
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
