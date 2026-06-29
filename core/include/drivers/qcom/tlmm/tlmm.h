/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Qualcomm TLMM GPIO and pinctrl driver — public API.
 *
 * GPIO consumers:
 *   chip = tlmm_get_chip();
 *   struct gpio g = { .chip = chip, .pin = 42, .dt_flags = 0 };
 *   tlmm_request_pin(42);
 *   gpio_set_direction(&g, GPIO_DIR_OUT);
 *   gpio_set_value(&g, GPIO_LEVEL_HIGH);
 *   tlmm_release_pin(42);
 *
 * Pinctrl consumers — all cases use the same state-based flow:
 *
 *   // Single pin, or multiple pins with the same config (one group):
 *   const struct tlmm_pin_group groups[] = {
 *       { pins, N, func, pull, drive_ma, false },
 *   };
 *
 *   // Multiple pins with different configs (multiple groups):
 *   const struct tlmm_pin_group groups[] = {
 *       { tx_pins, 1, 1, TLMM_PULL_NONE, 6, false },
 *       { rx_pins, 1, 1, TLMM_PULL_UP,   4, false },
 *   };
 *
 *   struct pinctrl_state *state = NULL;
 *   tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
 *   tlmm_apply_pin_state(state);
 *   tlmm_free_pin_state(state);
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __DRIVERS_QCOM_TLMM_TLMM_H
#define __DRIVERS_QCOM_TLMM_TLMM_H

#include <drivers/gpio.h>
#include <drivers/pinctrl.h>
#include <stdbool.h>
#include <stdint.h>
#include <tee_api_types.h>
#include <util.h>

/* Maximum number of tiles a single TLMM controller can have */
#define TLMM_MAX_TILES			U(5)

/*
 * struct tlmm_desc - SoC-specific TLMM hardware descriptor.
 * Defined in tlmm_soc_data.c per SoC. All driver code is parameterized
 * through this struct — nothing is hardcoded in gpio.c or pinctrl.c.
 *
 * @base:          Physical base address (TLMM_BASE).
 * @size:          Physical region size (TLMM_BASE_SIZE).
 * @pin_reg_width: Byte stride between pin register sets (0x1000).
 * @num_tiles:     Number of tiles (1 for LeMans, 3 for Talos).
 * @tile_offsets:  Byte offsets from base to each tile's register window.
 * @num_gpios:     Total GPIO count.
 * @has_egpio:     true if GPIO_CFG has EGPIO_PRESENT[11]/EGPIO_ENABLE[12].
 * @has_strong_pull: true if GPIO_CFG has STRONG_PULL[13].
 * @lp_cfg_mask:   Bitmask of LP_CFG bits to copy into GPIO_CFG on pin
 *                 release.  Bits [12:0] are common to all SoCs.  Bit 13
 *                 is GPIO_VALID in LP_CFG (Lemans) — exclude it since
 *                 GPIO_CFG has no such field — or I2C_PULL (newer SoCs)
 *                 — include it when GPIO_CFG bit 13 exists.
 */
struct tlmm_desc {
	paddr_t      base;
	size_t       size;
	uint32_t     pin_reg_width;
	uint32_t     num_tiles;
	uint32_t     tile_offsets[TLMM_MAX_TILES];
	unsigned int num_gpios;
	bool         has_egpio;
	bool         has_strong_pull;
	uint32_t     lp_cfg_mask;
};

/* Pull configuration values — match Qcom HAL GPIOPullType */
#define TLMM_PULL_NONE		U(0)
#define TLMM_PULL_DOWN		U(1)
#define TLMM_PULL_KEEPER	U(2)
#define TLMM_PULL_UP		U(3)

/* Maximum pins in a single tlmm_pin_group */
#define TLMM_MAX_PINS_PER_GROUP		U(64)

/* GPIO API */
struct gpio_chip *tlmm_get_chip(void);

/*
 * Pin ownership API — must be called before using gpio_ops on a pin.
 *
 * tlmm_request_pin(): claim exclusive ownership of @pin.  Returns
 *   TEE_ERROR_BUSY if another caller already owns it, TEE_ERROR_BAD_PARAMETERS
 *   if @pin is out of range.
 * tlmm_release_pin(): release ownership.  Safe to call on an unowned pin.
 */
TEE_Result tlmm_request_pin(unsigned int pin);
void       tlmm_release_pin(unsigned int pin);

/* Pinctrl API *//*
 * struct tlmm_pin_group - One group of pins sharing the same configuration.
 *
 * Used as an element of the array passed to tlmm_make_pin_state() to express
 * a set of groups where each group may have a different func/pull/drive.
 *
 * @pins:        Array of global pin numbers in this group.
 * @pin_count:   Number of entries in @pins.  Max TLMM_MAX_PINS_PER_GROUP.
 * @func:        FUNC_SEL value (0 = GPIO mode).
 * @pull:        TLMM_PULL_NONE / DOWN / KEEPER / UP.
 * @drive_ma:    Drive strength in mA (2–16).
 * @strong_pull: true = I2C strong pull (only if has_strong_pull && PULL_UP).
 */
struct tlmm_pin_group {
	const unsigned int *pins;
	unsigned int        pin_count;
	uint32_t            func;
	uint32_t            pull;
	unsigned int        drive_ma;
	bool                strong_pull;
};

/*
 * tlmm_make_pin_state() - Build a pinctrl_state from multiple pin groups.
 *
 * Each group may carry a distinct configuration (func/pull/drive), allowing
 * a caller to configure many differently-typed pins in one shot:
 *
 *   tlmm_apply_pin_state(state);   -- claims all pins, writes all configs
 *   tlmm_free_pin_state(state);    -- releases all ownership, frees memory
 *
 * On failure during construction every successfully allocated group config is
 * freed and the state struct is not returned.  Ownership is never touched
 * here; claiming happens inside conf_apply.
 *
 * tlmm_apply_pin_state() does not roll back on partial failure.  If it
 * returns an error, call tlmm_free_pin_state() to release whatever was
 * applied.
 *
 * Returns TEE_ERROR_BAD_PARAMETERS if groups/group_count/out_state are NULL
 * or zero, or TEE_ERROR_OUT_OF_MEMORY on allocation failure.
 */
TEE_Result tlmm_make_pin_state(const struct tlmm_pin_group *groups,
			       unsigned int group_count,
			       struct pinctrl_state **out_state);

/*
 * tlmm_apply_pin_state() / tlmm_free_pin_state() - Apply and release a
 * multi-group pin state built by tlmm_make_pin_state().
 *
 * Non-DT equivalents of pinctrl_apply_state() / pinctrl_free_state().
 * tlmm_apply_pin_state() returns TEE_ERROR_BUSY if any group's pins are
 * already owned; on partial failure call tlmm_free_pin_state() to release
 * whatever was applied.
 */
TEE_Result tlmm_apply_pin_state(struct pinctrl_state *state);
void       tlmm_free_pin_state(struct pinctrl_state *state);

#endif /* __DRIVERS_QCOM_TLMM_TLMM_H */
