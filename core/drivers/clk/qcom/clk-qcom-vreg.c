// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * RPMh CX/MX rail voting for QUP SE rate changes. Kept separate from the RCG
 * walker (clk-qcom.c) so a non-RPMh target can supply its own rail-vote
 * backend behind the same qcom_clk_vreg_vote() contract.
 */

#include <assert.h>
#include <drivers/qcom/cmd_db/cmd_db.h>
#include <drivers/qcom/rpmh/rpmh_client.h>
#include <kernel/mutex.h>
#include <tee_api_types.h>

#include "clk_qcom_vreg.h"

/*
 * Vote a corner before raising the rate and after lowering it, so a rate is
 * never programmed under-volted. RPMh takes an ordinal into the rail's
 * supported-corner list (cmd_db aux data), not the raw corner, so
 * qcom_cx_corner_to_hlvl() resolves that first.
 *
 * A single aggregate corner is voted for the whole QUP BSP: each domain holds
 * a reference count per corner (votes[]) instead of its own vote, and the rail
 * is held at the highest corner with a nonzero count -- so one SE lowering its
 * need never drops the rail below what another SE still requires.
 */
#define QCOM_CLK_VLVL_MAX	16

struct qcom_clk_vreg {
	struct mutex lock;	/* serialises the vote + tracking state */
	struct rpmh_client *rpmh;
	bool ready;
	uint32_t cx_addr;
	uint32_t mx_addr;
	bool have_mx;
	uint16_t vlvls[QCOM_CLK_VLVL_MAX];
	uint32_t n_vlvls;
	uint16_t voted;		/* corner currently voted on the rail */
	bool desync;		/* CX/MX may disagree with @voted */
	uint16_t votes[QCOM_CLK_VLVL_MAX];	/* reference count per corner */
};

static struct qcom_clk_vreg qcom_clk_vreg = {
	.lock = MUTEX_INITIALIZER,
};

static TEE_Result qcom_clk_vreg_init(void)
{
	struct qcom_clk_vreg *v = &qcom_clk_vreg;
	size_t len = sizeof(v->vlvls);
	TEE_Result res = TEE_SUCCESS;

	if (v->ready)
		return TEE_SUCCESS;

	if (!v->rpmh) {
		v->rpmh = rpmh_create_handle(RSC_DRV_SECURE, "clk_qup");
		if (!v->rpmh)
			return TEE_ERROR_GENERIC;
	}

	res = cmd_db_get_addr("cx.lvl", &v->cx_addr);
	if (res)
		return res;

	/*
	 * A rail advertising more corners than vlvls[] holds would leave the
	 * top ones unreachable, so take the short-buffer error rather than a
	 * partial list.
	 */
	res = cmd_db_get_aux("cx.lvl", (uint8_t *)v->vlvls, &len);
	if (res)
		return res;
	v->n_vlvls = len / sizeof(v->vlvls[0]);
	if (!v->n_vlvls)
		return TEE_ERROR_BAD_STATE;

	/* MX tracks CX on this target; vote it too when the rail exists. */
	if (!cmd_db_get_addr("mx.lvl", &v->mx_addr))
		v->have_mx = true;

	v->ready = true;
	return TEE_SUCCESS;
}

static TEE_Result qcom_cx_corner_to_hlvl(uint16_t corner, uint32_t *hlvl)
{
	uint32_t i = 0;

	for (i = 0; i < qcom_clk_vreg.n_vlvls; i++) {
		if (qcom_clk_vreg.vlvls[i] >= corner) {
			*hlvl = i;
			return TEE_SUCCESS;
		}
	}

	return TEE_ERROR_BAD_PARAMETERS;
}

/* Issue the aggregate corner vote to CX (and MX). Caller holds v->lock. */
static TEE_Result qcom_clk_vreg_apply(uint16_t corner)
{
	struct qcom_clk_vreg *v = &qcom_clk_vreg;
	uint32_t hlvl = 0;
	uint32_t req_id = 0;
	TEE_Result res = TEE_SUCCESS;

	if (corner == v->voted && !v->desync)
		return TEE_SUCCESS;

	res = qcom_cx_corner_to_hlvl(corner, &hlvl);
	if (res)
		return res;

	/*
	 * CX and MX are voted as a pair. If one lands and the other fails the
	 * rails disagree with @voted, so flag the mismatch and let the next
	 * call re-vote both instead of taking the no-change shortcut above.
	 */
	v->desync = true;

	res = rpmh_send_command(v->rpmh, RPMH_SET_ACTIVE, true, v->cx_addr,
				hlvl, &req_id);
	if (res)
		return res;
	rpmh_barrier_single(v->rpmh, req_id);

	if (v->have_mx) {
		res = rpmh_send_command(v->rpmh, RPMH_SET_ACTIVE, true,
					v->mx_addr, hlvl, &req_id);
		if (res)
			return res;
		rpmh_barrier_single(v->rpmh, req_id);
	}

	v->voted = corner;
	v->desync = false;
	return TEE_SUCCESS;
}

/* Highest corner with a nonzero reference count, or 0 if none. */
static uint16_t qcom_clk_vreg_peak(void)
{
	struct qcom_clk_vreg *v = &qcom_clk_vreg;
	uint16_t peak = 0;
	uint32_t i = 0;

	for (i = 0; i < v->n_vlvls; i++)
		if (v->votes[i])
			peak = v->vlvls[i];

	return peak;
}

/*
 * Move a domain's vote from corner @old to @new and re-vote the rail at the
 * new aggregate. 0 means "no vote". Called under v->lock.
 */
static TEE_Result qcom_clk_vreg_move(uint16_t old_corner, uint16_t new_corner)
{
	struct qcom_clk_vreg *v = &qcom_clk_vreg;
	uint32_t old_idx = 0;
	uint32_t new_idx = 0;
	TEE_Result res = TEE_SUCCESS;

	if (old_corner == new_corner) {
		/* Refcounts unchanged, but a stranded desync needs a retry. */
		if (v->desync)
			return qcom_clk_vreg_apply(qcom_clk_vreg_peak());
		return TEE_SUCCESS;
	}

	if (new_corner) {
		res = qcom_cx_corner_to_hlvl(new_corner, &new_idx);
		if (res)
			return res;
	}
	if (old_corner) {
		res = qcom_cx_corner_to_hlvl(old_corner, &old_idx);
		if (res)
			return res;
		assert(v->votes[old_idx]);
	}

	if (new_corner)
		v->votes[new_idx]++;
	if (old_corner)
		v->votes[old_idx]--;

	res = qcom_clk_vreg_apply(qcom_clk_vreg_peak());
	if (res) {
		if (new_corner)
			v->votes[new_idx]--;
		if (old_corner)
			v->votes[old_idx]++;
		return res;
	}

	return TEE_SUCCESS;
}

TEE_Result qcom_clk_vreg_vote(uint16_t old_corner, uint16_t new_corner)
{
	struct qcom_clk_vreg *v = &qcom_clk_vreg;
	TEE_Result res = TEE_SUCCESS;

	mutex_lock(&v->lock);
	res = qcom_clk_vreg_init();
	if (!res)
		res = qcom_clk_vreg_move(old_corner, new_corner);
	mutex_unlock(&v->lock);

	return res;
}
