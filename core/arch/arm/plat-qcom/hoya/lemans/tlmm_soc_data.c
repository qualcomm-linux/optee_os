// SPDX-License-Identifier: BSD-2-Clause
/*
 * LeMans TLMM hardware descriptor.
 *
 * Values sourced from GPIOLemans.c (HALgpioBlockDescType):
 *   nBaseAddr     = 0x0f000000  (TLMM_BASE)
 *   nPinRegWidth  = 0x1000
 *   nNumTiles     = 1
 *   aTiles[0]     = 0x100000
 *   nMaxPinNumber = 148  (num_gpios = 149)
 *   has_egpio     = true
 *   has_strong_pull = false
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <platform_config.h>
#include <drivers/qcom/tlmm/tlmm.h>
#include <util.h>

const struct tlmm_desc tlmm_soc_desc = {
	.base            = TLMM_BASE,
	.size            = TLMM_BASE_SIZE,
	.pin_reg_width   = 0x1000,
	.num_tiles       = 1,
	.tile_offsets    = { 0x100000, 0, 0, 0, 0 },
	.num_gpios       = 149,
	.has_egpio       = true,
	.has_strong_pull = false,
	.lp_cfg_mask     = GENMASK_32(12, 0),
};
