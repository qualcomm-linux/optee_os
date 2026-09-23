/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * ICB configuration query driver interface.
 *
 * Defines the data structures, enumerations and helper macros consumed
 * by the platform icbcfg_query_data.c back-end and the common
 * icbcfg_query driver.
 */

#ifndef QTI_ICBCFG_QUERY_H
#define QTI_ICBCFG_QUERY_H

#include <stdbool.h>
#include <stdint.h>
#include <drivers/qcom/chipinfo/chipinfo.h>

/* Chip version encoding: major.minor packed into a uint32_t */
#ifndef CHIPINFO_VERSION
#define CHIPINFO_VERSION(major, minor)	(((major) << 16) | (minor))
#endif

/**
 * enum icbcfg_addr_trans_type - address-translator hardware variants.
 * @ICBCFG_ADDR_TRANS_NONE:     no address translation
 * @ICBCFG_ADDR_TRANS_LLCC:     generic LLCC BEAC address translator
 * @ICBCFG_ADDR_TRANS_LLCC_6CH: 6-channel LLCC BEAC address translator
 *                              (also used for 4-ch and 2-ch SKUs that
 *                              share the same translator hardware)
 * @ICBCFG_ADDR_TRANS_COUNT:    sentinel / number of valid entries
 */
enum icbcfg_addr_trans_type {
	ICBCFG_ADDR_TRANS_NONE		= 0,
	ICBCFG_ADDR_TRANS_NOC,		/* MemNOC address translator */
	ICBCFG_ADDR_TRANS_LLCC,		/* LLCC BEAC address translator */
	ICBCFG_ADDR_TRANS_MC,		/* MC ISU address translator */
	ICBCFG_ADDR_TRANS_LLCC_6CH,	/* 6-channel LLCC BEAC */
	ICBCFG_ADDR_TRANS_COUNT,
};

/**
 * struct icbcfg_mem_region - a contiguous physical DDR region.
 * @start: first byte of the region (inclusive)
 * @end:   last byte of the region (inclusive)
 */
struct icbcfg_mem_region {
	uint64_t start;
	uint64_t end;
};

/**
 * struct icbcfg_seg_offsets - per-segment register offsets within a
 *                             translation block.
 * @region_cfg2: byte offset of the REGION_CFG2 register
 * @region_cfg3: byte offset of the REGION_CFG3 register
 */
struct icbcfg_seg_offsets {
	uint32_t region_cfg2;
	uint32_t region_cfg3;
};

/* Per-variant (SKU) device configuration. */
struct icbcfg_device_config {
	/* Chip identification */
	uint32_t  family;
	bool      match;
	uint32_t  version;

	/* Optional SKU discriminator */
	uint8_t  *reg_addr;
	uint32_t  reg_mask;
	uint32_t  reg_val;

	/* DDR topology */
	uint32_t                    map_ddr_region_count;
	struct icbcfg_mem_region   *map_ddr_regions;
	uint32_t                    num_segments;
	uint32_t                    num_channels;
	uint32_t                    addr_width;
	enum icbcfg_addr_trans_type trans_type;
	uint8_t                   **trans_bases;
	/* bases for segments >= ADDL_REGION_START */
	uint8_t                   **addl_trans_bases;
	struct icbcfg_seg_offsets  *seg_offsets;
};

/* Top-level descriptor exported by each platform's icbcfg_query_data.c. */
struct icbcfg_info {
	uint32_t                      num_configs;
	struct icbcfg_device_config **configs;
};

/**
 * enum icbcfg_error_type - return codes for ICB query functions.
 */
enum icbcfg_error_type {
	ICBCFG_SUCCESS               =  0,
	ICBCFG_ERROR                 = -1,
	ICBCFG_ERROR_INVALID_PARAM   = -2,
	ICBCFG_ERROR_INIT_FAILURE    = -3,
	ICBCFG_ERROR_INVALID_ADDRESS = -4,
};

struct icb_region {
	uint64_t	base_addr;
	uint64_t	size;
	bool		interleaved;
};

/* Per-channel memory-map descriptor. */
struct icb_channel {
	struct icb_region	regions[10]; /* MAX_REGIONS */
};

/* Full system memory map returned by icb_get_memmap(). */
struct icb_mem_map {
	struct icb_channel	channels[8]; /* MAX_CHANNELS */
};

/* Declared in the platform icbcfg_query_data.c */
extern struct icbcfg_info icbcfg_info;

/*
 * ICB configuration query API, implemented by the common icbcfg_query
 * driver and consumed by other in-tree drivers.
 */
void update_addr_translation(void);
enum icbcfg_error_type icb_get_memmap(struct icb_mem_map *info);
enum icbcfg_error_type
icb_get_allowed_mem_region(struct icbcfg_mem_region *info);
enum icbcfg_error_type
icb_get_allowed_mem_region_ex(struct icbcfg_mem_region **info,
			     uint32_t *num_regions);
enum icbcfg_error_type icb_get_mc_addr(uint64_t soc_addr, uint64_t *mc_addr);
enum icbcfg_error_type icb_get_soc_addr(uint64_t mc_addr, uint64_t *soc_addr);
enum icbcfg_error_type icb_get_num_ddr_channels(uint32_t *num_channels);

#endif /* QTI_ICBCFG_QUERY_H */
