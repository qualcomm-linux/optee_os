/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Qualcomm TLMM GPIO and pinctrl driver - public API.
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

#define TLMM_MAX_TILES			U(5)

struct tlmm_desc {
	paddr_t      base;
	size_t       size;
	uint32_t     pin_reg_width;
	uint32_t     num_tiles;
	uint32_t     tile_offsets[TLMM_MAX_TILES];
	unsigned int num_gpios;
	bool         has_egpio;
	bool         has_strong_pull;
};

/* SoC-specific hardware descriptor, defined per platform in tlmm_soc_data.c */
extern const struct tlmm_desc tlmm_soc_desc;

#define TLMM_PULL_NONE		U(0)
#define TLMM_PULL_DOWN		U(1)
#define TLMM_PULL_KEEPER	U(2)
#define TLMM_PULL_UP		U(3)

#define TLMM_MAX_PINS_PER_GROUP		U(64)

struct gpio_chip *tlmm_get_chip(void);

TEE_Result tlmm_request_pin(unsigned int pin);
void tlmm_release_pin(unsigned int pin);

struct tlmm_pin_group {
	const unsigned int *pins;
	unsigned int        pin_count;
	uint32_t            func;
	uint32_t            pull;
	unsigned int        drive_ma;
	bool                strong_pull;
};

TEE_Result tlmm_make_pin_state(const struct tlmm_pin_group *groups,
			       unsigned int group_count,
			       struct pinctrl_state **out_state);
TEE_Result tlmm_apply_pin_state(struct pinctrl_state *state);
void tlmm_free_pin_state(struct pinctrl_state *state);

#endif /* __DRIVERS_QCOM_TLMM_TLMM_H */
