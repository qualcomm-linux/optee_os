// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * ICB configuration query data for the Lemans (QCS9075) platform.
 * Provides icbcfg_info, consumed by the ICB configuration driver.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qcom/chipinfo/chipinfo.h>
#include <util.h>

#include "icbcfg_hwio.h"
#include "icbcfg_query.h"

/* DDR mappable regions */
static struct icbcfg_mem_region map_ddr_regions_lemans_v1[] = {
	{ 0x080000000ULL, SCL_DDR_MAX_ALLOWABLE_ADDR },
};

/* Translation block base addresses (one per LLCC channel) */
static uint8_t *trans_bases_lemans_v1[] = {
	[0] = (uint8_t *)LLCC0_LLCC_BEAC_ADDR_TRANSLATOR_CFG,
	[1] = (uint8_t *)LLCC1_LLCC_BEAC_ADDR_TRANSLATOR_CFG,
	[2] = (uint8_t *)LLCC2_LLCC_BEAC_ADDR_TRANSLATOR_CFG,
	[3] = (uint8_t *)LLCC3_LLCC_BEAC_ADDR_TRANSLATOR_CFG,
	[4] = (uint8_t *)LLCC4_LLCC_BEAC_ADDR_TRANSLATOR_CFG,
	[5] = (uint8_t *)LLCC5_LLCC_BEAC_ADDR_TRANSLATOR_CFG,
};

/* Segment register offsets (10 segments, 6 channels) */
static struct icbcfg_seg_offsets seg_offsets_lemans_v1[] = {
	[0] = { .region_cfg2 = 0x00, .region_cfg3 = 0x00 },
	[1] = { .region_cfg2 = 0x00, .region_cfg3 = 0x00 },
	[2] = { .region_cfg2 = 0x00, .region_cfg3 = 0x00 },
	[3] = { .region_cfg2 = 0x00, .region_cfg3 = 0x00 },
	[4] = { .region_cfg2 = 0x00, .region_cfg3 = 0x00 },
	[5] = { .region_cfg2 = 0x00, .region_cfg3 = 0x00 },
	[6] = { .region_cfg2 = 0x84, .region_cfg3 = 0x8c },
	[7] = { .region_cfg2 = 0x84, .region_cfg3 = 0x8c },
	[8] = { .region_cfg2 = 0x84, .region_cfg3 = 0x84 },
	[9] = { .region_cfg2 = 0x7c, .region_cfg3 = 0x7c },
};

/* 6-channel variant (default, no fuse check) */
static struct icbcfg_device_config lemans_v1 = {
	.family               = (uint32_t)QTI_CHIPINFO_FAMILY_LEMANSAU,
	.match                = false,
	.version              = CHIPINFO_VERSION(1, 0),
	.map_ddr_region_count = ARRAY_SIZE(map_ddr_regions_lemans_v1),
	.map_ddr_regions      = map_ddr_regions_lemans_v1,
	.num_segments         = 10U,
	.num_channels         = 6U,
	.addr_width           = 36U,
	.trans_type           = ICBCFG_ADDR_TRANS_LLCC_6CH,
	.trans_bases          = trans_bases_lemans_v1,
	.seg_offsets          = seg_offsets_lemans_v1,
};

/* 4-channel variant: DDR_CH_SEL_MODE == 1 */
static struct icbcfg_device_config lemans_v1_4ch = {
	.family               = (uint32_t)QTI_CHIPINFO_FAMILY_LEMANSAU,
	.match                = false,
	.version              = CHIPINFO_VERSION(1, 0),
	.reg_addr             =
		(uint8_t *)TCSR_TIMEOUT_INTR_CH2_CH4_OR_CH6_STATUS,
	.reg_mask             = TCSR_DDR_CH_SEL_MODE_BMSK,
	.reg_val              = 0x1U, /* DDR_CH_SEL_MODE = 1 for 4-channel */
	.map_ddr_region_count = ARRAY_SIZE(map_ddr_regions_lemans_v1),
	.map_ddr_regions      = map_ddr_regions_lemans_v1,
	.num_segments         = 10U,
	.num_channels         = 4U,
	.addr_width           = 36U,
	.trans_type           = ICBCFG_ADDR_TRANS_LLCC_6CH,
	.trans_bases          = trans_bases_lemans_v1,
	.seg_offsets          = seg_offsets_lemans_v1,
};

/* 2-channel variant: DDR_CH_SEL_MODE == 3 */
static struct icbcfg_device_config lemans_v1_2ch = {
	.family               = (uint32_t)QTI_CHIPINFO_FAMILY_LEMANSAU,
	.match                = false,
	.version              = CHIPINFO_VERSION(1, 0),
	.reg_addr             =
		(uint8_t *)TCSR_TIMEOUT_INTR_CH2_CH4_OR_CH6_STATUS,
	.reg_mask             = TCSR_DDR_CH_SEL_MODE_BMSK,
	.reg_val              = 0x3U, /* DDR_CH_SEL_MODE = 3 for 2-channel */
	.map_ddr_region_count = ARRAY_SIZE(map_ddr_regions_lemans_v1),
	.map_ddr_regions      = map_ddr_regions_lemans_v1,
	.num_segments         = 10U,
	.num_channels         = 2U,
	.addr_width           = 36U,
	.trans_type           = ICBCFG_ADDR_TRANS_LLCC_6CH,
	.trans_bases          = trans_bases_lemans_v1,
	.seg_offsets          = seg_offsets_lemans_v1,
};

/*
 * Target configuration list.
 * Ordered from most-constrained (fuse-matched) to least-constrained
 * (fallback), so the driver selects the first matching entry.
 */
static struct icbcfg_device_config *configs_lemans[] = {
	[0] = &lemans_v1_2ch,
	[1] = &lemans_v1_4ch,
	[2] = &lemans_v1,
};

/**
 * icbcfg_info - exported ICB configuration descriptor for Lemans.
 *
 * Consumed by the common icbcfg_query driver to select the correct
 * DDR topology and apply one-time NOC register writes at boot.
 */
struct icbcfg_info icbcfg_info = {
	.num_configs = ARRAY_SIZE(configs_lemans),
	.configs     = configs_lemans,
};
