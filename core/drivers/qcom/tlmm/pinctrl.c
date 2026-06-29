// SPDX-License-Identifier: BSD-2-Clause
/*
 * Qualcomm TLMM pinctrl driver — non-DT implementation.
 *
 * Implements the OP-TEE pinctrl_ops interface for Qcom TLMM hardware.
 * Works alongside gpio.c (gpio_ops).
 *
 * What pinctrl writes to GPIO_CFG:
 *   FUNC_SEL     [5:2]  — which peripheral function is muxed on this pad
 *   GPIO_PULL    [1:0]  — pull resistor configuration
 *   DRV_STRENGTH [8:6]  — output drive strength
 *   STRONG_PULL  [13]   — strong I2C pull-up (conditional on has_strong_pull)
 *
 * What gpio_ops (gpio.c) writes:
 *   GPIO_OE      [9]    — direction (input/output)
 *   GPIO_IN_OUT  +0x04  — output value / input read
 *
 * Consumer usage (non-DT):
 *   const struct tlmm_pin_group groups[] = {
 *       { pins, N, func, pull, drive_ma, false },
 *   };
 *   struct pinctrl_state *state = NULL;
 *
 *   tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
 *   tlmm_apply_pin_state(state);
 *   tlmm_free_pin_state(state);
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/pinctrl.h>
#include <drivers/qcom/tlmm/tlmm.h>
#include <keep.h>
#include <malloc.h>
#include <string.h>
#include <trace.h>
#include <util.h>

#include "tlmm_private.h"

/*
 * struct tlmm_pin_conf - Private data embedded in a struct pinconf.
 *
 * Single contiguous allocation:
 *   [struct pinconf][struct tlmm_pin_conf][unsigned int pins[pin_count]]
 *
 * conf->priv points to tlmm_pin_conf immediately after conf.
 * conf_free() calls free(conf) to release the entire block.
 *
 * @applied: set to true by conf_apply() on success; conf_free() only
 *           releases pin ownership if this is true, so a failed apply
 *           cannot accidentally free pins owned by another conf.
 */
struct tlmm_pin_conf {
	struct tlmm_chip *chip;
	uint32_t          func;
	uint32_t          pull;
	uint32_t          drive_hw;
	bool              strong_pull;
	bool              applied;
	unsigned int      pin_count;
	unsigned int      pins[];
};

/* -------------------------------------------------------------------------
 * pinctrl_ops callbacks
 * ----------------------------------------------------------------------
 */

static TEE_Result tlmm_conf_apply(struct pinconf *conf)
{
	struct tlmm_pin_conf *pc = conf->priv;
	struct tlmm_chip *chip = pc->chip;
	unsigned int i = 0;
	uint32_t clear = 0;
	uint32_t set = 0;

	/*
	 * Atomically verify none of the pins in this group are already owned,
	 * then claim them all.  A partial failure releases nothing — no pin
	 * was claimed yet since we check first, then claim.
	 */
	mutex_lock(&chip->lock);
	for (i = 0; i < pc->pin_count; i++) {
		unsigned int p = pc->pins[i];

		if (chip->pin_owners[p / 32] & BIT(p % 32)) {
			EMSG("TLMM: pin %u already owned, conf_apply rejected",
			     p);
			mutex_unlock(&chip->lock);
			return TEE_ERROR_BUSY;
		}
	}
	for (i = 0; i < pc->pin_count; i++) {
		unsigned int p = pc->pins[i];

		chip->pin_owners[p / 32] |= BIT(p % 32);
	}
	mutex_unlock(&chip->lock);

	pc->applied = true;

	clear = TLMM_CFG_PULL_MASK  |
		TLMM_CFG_FUNC_MASK   |
		TLMM_CFG_DRIVE_MASK  |
		TLMM_CFG_STRONG_PULL;

	set = (pc->pull & TLMM_CFG_PULL_MASK) |
	      ((pc->func     << TLMM_CFG_FUNC_SHIFT)  & TLMM_CFG_FUNC_MASK)  |
	      ((pc->drive_hw << TLMM_CFG_DRIVE_SHIFT) & TLMM_CFG_DRIVE_MASK);

	if (chip->desc->has_strong_pull &&
	    pc->strong_pull &&
	    pc->pull == TLMM_PULL_UP)
		set |= TLMM_CFG_STRONG_PULL;

	for (i = 0; i < pc->pin_count; i++) {
		DMSG("TLMM pinctrl: pin %u func=%u pull=%u drive_hw=%u",
		     pc->pins[i], pc->func, pc->pull, pc->drive_hw);
		tlmm_write_cfg(chip, pc->pins[i], clear, set);
	}

	return TEE_SUCCESS;
}

static void tlmm_conf_free(struct pinconf *conf)
{
	struct tlmm_pin_conf *pc = conf->priv;
	struct tlmm_chip *chip = pc->chip;
	unsigned int i = 0;

	if (pc->applied) {
		mutex_lock(&chip->lock);
		for (i = 0; i < pc->pin_count; i++) {
			unsigned int p = pc->pins[i];

			tlmm_restore_lp(chip, p);
			chip->pin_owners[p / 32] &= ~BIT(p % 32);
		}
		mutex_unlock(&chip->lock);
	}

	/*
	 * free(conf) releases the entire contiguous block — tlmm_pin_conf
	 * and pins[] are embedded in the same allocation, not separate.
	 */
	free(conf);
}

static const struct pinctrl_ops tlmm_pinctrl_ops = {
	.conf_apply = tlmm_conf_apply,
	.conf_free  = tlmm_conf_free,
};
DECLARE_KEEP_PAGER(tlmm_pinctrl_ops);

/* -------------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------
 */

/* 2mA steps: 0=2mA, 1=4mA ... 7=16mA; values above 16mA clamped to 7 */
static uint32_t tlmm_drive_to_hw(unsigned int drive_ma)
{
	if (drive_ma <= 2)
		return 0;
	if (drive_ma <= 4)
		return 1;
	if (drive_ma <= 6)
		return 2;
	if (drive_ma <= 8)
		return 3;
	if (drive_ma <= 10)
		return 4;
	if (drive_ma <= 12)
		return 5;
	if (drive_ma <= 14)
		return 6;
	return 7;
}

/* Build a pinctrl configuration blob for @pin_count pins.
 *
 * Allocates one contiguous block for all three objects:
 *
 *   +----------------+-------------------+----------------------+
 *   | struct pinconf | tlmm_pin_conf     | unsigned int pins[N] |
 *   | (conf)         | (pc = conf + 1)   | (pc->pins, FAM)      |
 *   +----------------+-------------------+----------------------+
 *   ^
 *   returned as struct pinconf * — free(conf) releases all three.
 *
 * conf->priv is set to pc (immediately after conf in memory).
 * pc->pins is a flexible array member — no separate pointer needed.
 */
static TEE_Result tlmm_make_pinconf(const unsigned int *pins,
				    unsigned int pin_count,
				    uint32_t func, uint32_t pull,
				    unsigned int drive_ma, bool strong_pull,
				    struct pinconf **out_conf)
{
	struct pinconf *conf = NULL;
	struct tlmm_pin_conf *pc = NULL;
	size_t alloc_size = 0;

	if (!pins || !pin_count || !out_conf)
		return TEE_ERROR_BAD_PARAMETERS;

	if (pin_count > TLMM_MAX_PINS_PER_GROUP) {
		EMSG("TLMM: pin_count %u exceeds max %u",
		     pin_count, TLMM_MAX_PINS_PER_GROUP);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	alloc_size = sizeof(*conf) + sizeof(*pc) +
		     pin_count * sizeof(pc->pins[0]);

	conf = calloc(1, alloc_size);
	if (!conf)
		return TEE_ERROR_OUT_OF_MEMORY;

	/* pc sits immediately after conf in the same allocation block */
	pc = (struct tlmm_pin_conf *)(conf + 1);

	pc->chip        = &tlmm;
	pc->func        = func;
	pc->pull        = pull;
	pc->drive_hw    = tlmm_drive_to_hw(drive_ma);
	pc->strong_pull = strong_pull;
	pc->pin_count   = pin_count;
	memcpy(pc->pins, pins, pin_count * sizeof(pc->pins[0]));

	conf->ops  = &tlmm_pinctrl_ops;
	conf->priv = pc;

	*out_conf = conf;
	return TEE_SUCCESS;
}

/*
 * tlmm_make_pin_state() - Build a pinctrl_state from multiple pin groups.
 *
 * Allocates a struct pinctrl_state containing one pinconf per group.  On any
 * allocation failure the already-built confs are freed and the state is not
 * returned.  No pins are owned at this point; claiming happens in conf_apply.
 */
TEE_Result tlmm_make_pin_state(const struct tlmm_pin_group *groups,
			       unsigned int group_count,
			       struct pinctrl_state **out_state)
{
	struct pinctrl_state *state = NULL;
	unsigned int i = 0;
	TEE_Result res = TEE_SUCCESS;

	if (!groups || !group_count || !out_state)
		return TEE_ERROR_BAD_PARAMETERS;

	state = calloc(1, sizeof(*state) +
			  group_count * sizeof(*state->confs));
	if (!state)
		return TEE_ERROR_OUT_OF_MEMORY;

	for (i = 0; i < group_count; i++) {
		res = tlmm_make_pinconf(groups[i].pins,
					groups[i].pin_count,
					groups[i].func,
					groups[i].pull,
					groups[i].drive_ma,
					groups[i].strong_pull,
					&state->confs[i]);
		if (res) {
			/*
			 * Free confs[0..i-1].  Use `i > 0` rather than
			 * `while (i--)` to avoid unsigned wrap when i == 0.
			 */
			for (; i > 0; i--) {
				struct pinconf *c = state->confs[i - 1];

				c->ops->conf_free(c);
			}
			free(state);
			return res;
		}
	}

	state->conf_count = group_count;
	*out_state = state;
	return TEE_SUCCESS;
}

/*
 * tlmm_apply_pin_state() / tlmm_free_pin_state() - Apply and release a
 * multi-group pin state built by tlmm_make_pin_state().
 *
 * These provide the same semantics as the framework's pinctrl_apply_state()
 * and pinctrl_free_state() without requiring CFG_DRIVERS_PINCTRL, which
 * pulls in DT infrastructure not available in non-DT builds.  The underlying
 * struct pinctrl_state type is identical, so callers can switch to the
 * framework functions transparently if DT support is added later.
 */
TEE_Result tlmm_apply_pin_state(struct pinctrl_state *state)
{
	unsigned int i = 0;
	TEE_Result res = TEE_SUCCESS;

	for (i = 0; i < state->conf_count; i++) {
		res = state->confs[i]->ops->conf_apply(state->confs[i]);
		if (res) {
			EMSG("TLMM: conf_apply failed for group %u: %#"PRIx32,
			     i, res);
			return res;
		}
	}

	return TEE_SUCCESS;
}

void tlmm_free_pin_state(struct pinctrl_state *state)
{
	unsigned int i = 0;

	for (i = 0; i < state->conf_count; i++)
		state->confs[i]->ops->conf_free(state->confs[i]);

	free(state);
}
