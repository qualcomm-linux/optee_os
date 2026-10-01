// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <trace.h>
#include <util.h>

#include <drivers/qcom/chipinfo/chipinfo.h>

#include "icbcfg_hwio.h"
#include "icbcfg_query.h"

#define MAX_REGIONS		10U
#define ADDL_REGION_START	6U
#define MAX_CHANNELS		8U

vaddr_t icbcfg_hwio_get_base(void)
{
	static vaddr_t io_base;
	static bool    io_base_valid;

	if (!io_base_valid) {
		io_base = (vaddr_t)phys_to_virt(ICBCFG_HWIO_BASE,
					       MEM_AREA_IO_SEC,
					       ICBCFG_HWIO_SIZE) -
			  ICBCFG_HWIO_BASE;
		io_base_valid = true;
	}

	return io_base;
}

register_phys_mem_pgdir(MEM_AREA_IO_SEC, ICBCFG_HWIO_BASE, ICBCFG_HWIO_SIZE);

struct addr_trans_seg {
	uint64_t	base;
	uint64_t	offset;	/* sign-extended */
	bool		error;
};

static bool				 query_dev_config_valid;
static struct icbcfg_device_config	*icb_dev_config;
static struct addr_trans_seg		 addr_trans_segs[MAX_REGIONS + 1U];

static void get_addr_translation(struct icbcfg_device_config *dev_config)
{
	uint32_t	i;
	uint32_t	error;
	uint32_t	lo, hi;
	uint64_t	max_addr;
	uint64_t	sign_bit;

	if (dev_config->addr_width < 2U) {
		EMSG("ICB: invalid addr_width %u", dev_config->addr_width);
		return;
	}

	max_addr = 1ULL << dev_config->addr_width;
	sign_bit = 1ULL << (dev_config->addr_width - 1U);

	memset(addr_trans_segs, 0, sizeof(addr_trans_segs));

	switch (dev_config->trans_type) {
	case ICBCFG_ADDR_TRANS_NOC: {
		uint8_t *base = dev_config->trans_bases[0];

		error = HWIO_INXF(base, MEMNOC_ADDR_TRANSLATOR_CFG, ERROR);
		for (i = 0U; i < dev_config->num_segments; i++) {
			struct addr_trans_seg *seg = &addr_trans_segs[i];

			if (i == 0U) {
				seg->base = 0ULL;
			} else {
				lo = HWIO_INXI(base,
					MEMNOC_ADDR_TRANSLATOR_BASEn_LOW, i);
				hi = HWIO_INXI(base,
					MEMNOC_ADDR_TRANSLATOR_BASEn_HIGH, i);
				seg->base = ((uint64_t)lo) |
					(((uint64_t)hi) << 32);
			}

			lo = HWIO_INXI(base,
				MEMNOC_ADDR_TRANSLATOR_OFFSETn_LOW, i);
			hi = HWIO_INXI(base,
				MEMNOC_ADDR_TRANSLATOR_OFFSETn_HIGH, i);
			seg->offset = ((uint64_t)lo) | (((uint64_t)hi) << 32);
			seg->error = (error & (1U << i)) != 0U;
			seg->offset = (seg->offset ^ sign_bit) - sign_bit;
		}
		break;
	}

	case ICBCFG_ADDR_TRANS_LLCC: {
		uint8_t *base = dev_config->trans_bases[0];
		uint8_t *addl_base = dev_config->addl_trans_bases ?
			dev_config->addl_trans_bases[0] : NULL;

		/* Segments 0..(ADDL_REGION_START-1): base; 6+: addl_base */
		error = HWIO_INXF(base, LLCC_BEAC_ADDR_TRANSLATOR_CFG, ERROR);
		for (i = 0U; i < dev_config->num_segments; i++) {
			struct addr_trans_seg *seg = &addr_trans_segs[i];

			if (i == 0U) {
				seg->base = 0ULL;
			} else if (i < ADDL_REGION_START) {
				lo = HWIO_INXI(base,
					LLCC_BEAC_ADDR_REGIONn_CFG2, i);
				hi = HWIO_INXI(base,
					LLCC_BEAC_ADDR_REGIONn_CFG3, i);
				seg->base = ((uint64_t)lo) |
					(((uint64_t)hi) << 32);
			} else if (addl_base) {
				lo = HWIO_INXI(addl_base,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG2, i);
				hi = HWIO_INXI(addl_base,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG3, i);
				seg->base = ((uint64_t)lo) |
					(((uint64_t)hi) << 32);
			}

			if (i < ADDL_REGION_START) {
				lo = HWIO_INXI(base,
					LLCC_BEAC_ADDR_REGIONn_CFG0, i);
				hi = HWIO_INXI(base,
					LLCC_BEAC_ADDR_REGIONn_CFG1, i);
				seg->offset = ((uint64_t)lo) |
					(((uint64_t)hi) << 32);
			} else if (addl_base) {
				lo = HWIO_INXI(addl_base,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG0, i);
				hi = HWIO_INXI(addl_base,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG1, i);
				seg->offset = ((uint64_t)lo) |
					(((uint64_t)hi) << 32);
			}
			seg->error = (error & (1U << i)) != 0U;
			seg->offset = (seg->offset ^ sign_bit) - sign_bit;
		}
		break;
	}

	case ICBCFG_ADDR_TRANS_MC: {
		uint8_t *base = dev_config->trans_bases[0];

		/* Segments 0..5: REGIONn_CFG{0..3}; 6+: REGION_n_CFG{0,1} */
		error = HWIO_INXF(base, MC_ISU_ADDR_TRANSLATOR_CFG, ERROR);
		for (i = 0U; i < dev_config->num_segments; i++) {
			struct addr_trans_seg *seg = &addr_trans_segs[i];

			if (i < ADDL_REGION_START) {
				if (i == 0U) {
					seg->base = 0ULL;
				} else {
					lo = HWIO_INXI(base,
						MC_ISU_ADDR_REGIONn_CFG2, i);
					hi = HWIO_INXI(base,
						MC_ISU_ADDR_REGIONn_CFG3, i);
					seg->base = ((uint64_t)lo) |
						(((uint64_t)hi) << 32);
				}
				lo = HWIO_INXI(base,
					MC_ISU_ADDR_REGIONn_CFG0, i);
				hi = HWIO_INXI(base,
					MC_ISU_ADDR_REGIONn_CFG1, i);
				seg->offset = ((uint64_t)lo) |
					(((uint64_t)hi) << 32);
			} else {
				seg->base = 0ULL;
				lo = HWIO_INXI(base,
					MC_ISU_ADDR_REGION_n_CFG0, i);
				hi = HWIO_INXI(base,
					MC_ISU_ADDR_REGION_n_CFG1, i);
				seg->offset = ((uint64_t)lo) |
					(((uint64_t)hi) << 32);
			}
			seg->error = (error & (1U << i)) != 0U;
			seg->offset = (seg->offset ^ sign_bit) - sign_bit;
		}
		break;
	}

	case ICBCFG_ADDR_TRANS_LLCC_6CH: {
		uint8_t *base = dev_config->trans_bases[0];

		error = HWIO_INXF(base, LLCC_BEAC_ADDR_TRANSLATOR_CFG, ERROR);
		for (i = 0U; i < dev_config->num_segments; i++) {
			struct addr_trans_seg *seg = &addr_trans_segs[i];

			if (i == 0U) {
				seg->base = 0ULL;
			} else {
				uint8_t *b2 = base +
					dev_config->seg_offsets[i].region_cfg2;
				uint8_t *b3 = base +
					dev_config->seg_offsets[i].region_cfg3;

				lo = HWIO_INXI(b2,
					LLCC_BEAC_ADDR_REGIONn_CFG2, i);
				hi = HWIO_INXI(b3,
					LLCC_BEAC_ADDR_REGIONn_CFG3, i);
				seg->base = ((uint64_t)lo) |
					(((uint64_t)hi) << 32);
			}
			seg->error = (error & (1U << i)) != 0U;
		}
		break;
	}

	default:
		return;
	}

	/* Sentinel cap entries mark the top of the address space as invalid. */
	for (; i < ARRAY_SIZE(addr_trans_segs); i++) {
		addr_trans_segs[i].base  = max_addr;
		addr_trans_segs[i].error = true;
	}
}

static bool get_device_configuration(struct icbcfg_device_config **dev_config)
{
	if (!query_dev_config_valid) {
		enum qti_chipinfo_family family;
		uint32_t version;
		uint32_t i;

		family  = qti_chipinfo_get_chip_family();
		version = qti_chipinfo_get_chip_version();

		for (i = 0U; i < icbcfg_info.num_configs; i++) {
			struct icbcfg_device_config *cfg =
				icbcfg_info.configs[i];

			if (!cfg)
				continue;

			if ((uint32_t)family != cfg->family)
				continue;

			/*
			 * Exact match, or non-exact and
			 * version >= cfg->version.
			 */
			if (version != cfg->version &&
			   (cfg->match || version < cfg->version)) {
				continue;
			}

			/* No register check, or masked read equals reg_val. */
			if (cfg->reg_addr &&
			   cfg->reg_val != (HWIO_IN((uintptr_t)cfg->reg_addr) &
					   cfg->reg_mask)) {
				continue;
			}

			icb_dev_config = cfg;
			get_addr_translation(icb_dev_config);
			query_dev_config_valid = true;
			break;
		}
	}

	*dev_config = icb_dev_config;
	return query_dev_config_valid;
}

static void get_system_memory_map(struct icbcfg_device_config *dev_config,
				 struct icb_mem_map *sys_map)
{
	uint32_t		i, region;
	struct icb_region	*regions = sys_map->channels[0].regions;
	uint64_t		last_base = 0ULL;

	memset(sys_map, 0, sizeof(struct icb_mem_map));

	for (i = 0U, region = 0U; i < ARRAY_SIZE(addr_trans_segs); i++) {
		if (!addr_trans_segs[i].error) {
			if (regions[region].base_addr == 0ULL) {
				regions[region].base_addr   =
					addr_trans_segs[i].base;
				regions[region].interleaved =
					dev_config->num_channels > 1U;
				last_base = addr_trans_segs[i].base;
			} else {
				regions[region].size +=
					addr_trans_segs[i].base - last_base;
				last_base = addr_trans_segs[i].base;
			}
		} else {
			if (regions[region].base_addr != 0ULL) {
				regions[region].size +=
					addr_trans_segs[i].base - last_base;
				region++;
			}
		}
	}

	for (i = 1U; i < dev_config->num_channels; i++)
		sys_map->channels[i] = sys_map->channels[0];
}

static enum icbcfg_error_type
translate_address(struct icbcfg_device_config *dev_config,
		 uint64_t soc_addr, uint64_t *mc_addr)
{
	enum icbcfg_error_type	ret = ICBCFG_ERROR_INVALID_ADDRESS;
	uint32_t		i;

	if (dev_config->family == (uint32_t)QTI_CHIPINFO_FAMILY_SDM845 &&
	   dev_config->version <= CHIPINFO_VERSION(1, 0)) {
		for (i = 0U; i < dev_config->num_segments; i++) {
			if (addr_trans_segs[i].base   <= soc_addr &&
			   addr_trans_segs[i + 1U].base >  soc_addr) {
				if (!addr_trans_segs[i].error) {
					*mc_addr = soc_addr +
						   addr_trans_segs[i].offset;
					ret = ICBCFG_SUCCESS;
				}
				break;
			}
		}
	} else {
		*mc_addr = soc_addr;
		ret = ICBCFG_SUCCESS;
	}

	return ret;
}

static enum icbcfg_error_type
translate_mc_address(struct icbcfg_device_config *dev_config,
		    uint64_t mc_addr, uint64_t *soc_addr)
{
	enum icbcfg_error_type	ret = ICBCFG_ERROR_INVALID_ADDRESS;
	uint32_t		i;

	for (i = 0U; i < dev_config->num_segments; i++) {
		uint64_t seg_base = addr_trans_segs[i].base +
				    addr_trans_segs[i].offset;
		uint64_t seg_end  = addr_trans_segs[i + 1U].base +
				    addr_trans_segs[i].offset;

		if (seg_base <= mc_addr && seg_end > mc_addr) {
			if (!addr_trans_segs[i].error) {
				*soc_addr = mc_addr -
					    addr_trans_segs[i].offset;
				ret = ICBCFG_SUCCESS;
				break;
			}
		}
	}

	return ret;
}

static bool check_overlap(const struct icb_region *region1,
			 const struct icb_region *region2)
{
	if ((region1->base_addr < region2->base_addr &&
	    region1->base_addr + region1->size <= region2->base_addr) ||
	   (region2->base_addr < region1->base_addr &&
	   region2->base_addr + region2->size <= region1->base_addr)) {
		return false;
	}

	return true;
}

static enum icbcfg_error_type
compare_ch_segments(struct icbcfg_device_config *dev_config)
{
	uint32_t	i, j;
	uint32_t	base_low = 0U, base_high = 0U, off_low, off_high;
	uint32_t	cbase_low, cbase_high;

	switch (dev_config->trans_type) {
	case ICBCFG_ADDR_TRANS_NOC: {
		uint8_t *base = dev_config->trans_bases[0];

		for (i = 0U; i < dev_config->num_segments; i++) {
			if (i != 0U) {
				base_low  = HWIO_INXI(base,
					MEMNOC_ADDR_TRANSLATOR_BASEn_LOW, i);
				base_high = HWIO_INXI(base,
					MEMNOC_ADDR_TRANSLATOR_BASEn_HIGH, i);
			}
			off_low  = HWIO_INXI(base,
				MEMNOC_ADDR_TRANSLATOR_OFFSETn_LOW, i);
			off_high = HWIO_INXI(base,
				MEMNOC_ADDR_TRANSLATOR_OFFSETn_HIGH, i);

			for (j = 1U; j < dev_config->num_channels; j++) {
				uint8_t *cbase = dev_config->trans_bases[j];

				if (i != 0U) {
					cbase_low  = HWIO_INXI(cbase,
					MEMNOC_ADDR_TRANSLATOR_BASEn_LOW, i);
					cbase_high = HWIO_INXI(cbase,
					MEMNOC_ADDR_TRANSLATOR_BASEn_HIGH, i);
					if (base_low != cbase_low ||
						base_high != cbase_high)
						return ICBCFG_ERROR;
				}

				cbase_low  = HWIO_INXI(cbase,
					MEMNOC_ADDR_TRANSLATOR_OFFSETn_LOW, i);
				cbase_high = HWIO_INXI(cbase,
					MEMNOC_ADDR_TRANSLATOR_OFFSETn_HIGH, i);
				if (off_low != cbase_low ||
					off_high != cbase_high)
					return ICBCFG_ERROR;
			}
		}
		break;
	}

	case ICBCFG_ADDR_TRANS_LLCC: {
		uint8_t *base = dev_config->trans_bases[0];

		for (i = 0U;
			i < dev_config->num_segments && i < ADDL_REGION_START;
			i++) {
			if (i != 0U) {
				base_low  = HWIO_INXI(base,
					LLCC_BEAC_ADDR_REGIONn_CFG2, i);
				base_high = HWIO_INXI(base,
					LLCC_BEAC_ADDR_REGIONn_CFG3, i);
			}
			off_low  = HWIO_INXI(base,
			LLCC_BEAC_ADDR_REGIONn_CFG0, i);
			off_high = HWIO_INXI(base,
			LLCC_BEAC_ADDR_REGIONn_CFG1, i);

			for (j = 1U; j < dev_config->num_channels; j++) {
				uint8_t *cbase = dev_config->trans_bases[j];

				if (i != 0U) {
					cbase_low  = HWIO_INXI(cbase,
						LLCC_BEAC_ADDR_REGIONn_CFG2, i);
					cbase_high = HWIO_INXI(cbase,
						LLCC_BEAC_ADDR_REGIONn_CFG3, i);
					if (base_low != cbase_low ||
						base_high != cbase_high)
						return ICBCFG_ERROR;
				}

				cbase_low  = HWIO_INXI(cbase,
					LLCC_BEAC_ADDR_REGIONn_CFG0, i);
				cbase_high = HWIO_INXI(cbase,
					LLCC_BEAC_ADDR_REGIONn_CFG1, i);
				if (off_low != cbase_low ||
					off_high != cbase_high)
					return ICBCFG_ERROR;
			}
		}

		if (dev_config->addl_trans_bases) {
			uint8_t *addl_base = dev_config->addl_trans_bases[0];

			for (i = ADDL_REGION_START;
				i < dev_config->num_segments;
				i++) {
				base_low  = HWIO_INXI(addl_base,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG2, i);
				base_high = HWIO_INXI(addl_base,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG3, i);
				off_low   = HWIO_INXI(addl_base,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG0, i);
				off_high  = HWIO_INXI(addl_base,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG1, i);

				for (j = 1U;
					j < dev_config->num_channels;
					j++) {
					uint8_t *cbase =
						dev_config->addl_trans_bases[j];

					cbase_low  = HWIO_INXI(cbase,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG2, i);
					cbase_high = HWIO_INXI(cbase,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG3, i);
					if (base_low != cbase_low ||
						base_high != cbase_high)
						return ICBCFG_ERROR;

					cbase_low  = HWIO_INXI(cbase,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG0, i);
					cbase_high = HWIO_INXI(cbase,
					LLCC_BEAC_ADDR_ADDL_REGIONn_CFG1, i);
					if (off_low != cbase_low ||
						off_high != cbase_high)
						return ICBCFG_ERROR;
				}
			}
		}
		break;
	}

	case ICBCFG_ADDR_TRANS_LLCC_6CH:
		for (i = 0U; i < dev_config->num_segments; i++) {
			uint8_t *base = dev_config->trans_bases[0];

			if (i != 0U) {
				uint8_t *b2 = base +
					dev_config->seg_offsets[i].region_cfg2;
				uint8_t *b3 = base +
					dev_config->seg_offsets[i].region_cfg3;

				base_low  = HWIO_INXI(b2,
					LLCC_BEAC_ADDR_REGIONn_CFG2, i);
				base_high = HWIO_INXI(b3,
					LLCC_BEAC_ADDR_REGIONn_CFG3, i);
			}

			for (j = 1U; j < dev_config->num_channels; j++) {
				uint8_t *cbase = dev_config->trans_bases[j];
				uint8_t *cb2 = cbase +
					dev_config->seg_offsets[i].region_cfg2;
				uint8_t *cb3 = cbase +
					dev_config->seg_offsets[i].region_cfg3;

				if (i == 0U)
					continue;

				cbase_low  = HWIO_INXI(cb2,
					LLCC_BEAC_ADDR_REGIONn_CFG2, i);
				cbase_high = HWIO_INXI(cb3,
					LLCC_BEAC_ADDR_REGIONn_CFG3, i);
				if (base_low != cbase_low ||
					base_high != cbase_high)
					return ICBCFG_ERROR;
			}
		}

		for (i = 0U; i < MAX_REGIONS; i++) {
			uint8_t *base = dev_config->trans_bases[0];

			if (i != 0U) {
				base_low  = HWIO_INXI(base,
					LLCC_BEAC_MA_ADDR_REGIONn_CFG2, i);
				base_high = HWIO_INXI(base,
					LLCC_BEAC_MA_ADDR_REGIONn_CFG3, i);
			}
			off_low  = HWIO_INXI(base,
				LLCC_BEAC_MA_ADDR_REGIONn_CFG0, i);
			off_high = HWIO_INXI(base,
				LLCC_BEAC_MA_ADDR_REGIONn_CFG1, i);

			for (j = 1U; j < dev_config->num_channels; j++) {
				uint8_t *cbase = dev_config->trans_bases[j];

				if (i != 0U) {
					cbase_low  = HWIO_INXI(cbase,
					LLCC_BEAC_MA_ADDR_REGIONn_CFG2, i);
					cbase_high = HWIO_INXI(cbase,
					LLCC_BEAC_MA_ADDR_REGIONn_CFG3, i);
					if (base_low != cbase_low ||
						base_high != cbase_high)
						return ICBCFG_ERROR;
				}

				cbase_low  = HWIO_INXI(cbase,
					LLCC_BEAC_MA_ADDR_REGIONn_CFG0, i);
				cbase_high = HWIO_INXI(cbase,
					LLCC_BEAC_MA_ADDR_REGIONn_CFG1, i);
				if (off_low != cbase_low ||
					off_high != cbase_high)
					return ICBCFG_ERROR;
			}
		}
		break;

	case ICBCFG_ADDR_TRANS_MC:
		for (i = 0U; i < dev_config->num_segments; i++) {
			uint8_t *base = dev_config->trans_bases[0];

			if (i < ADDL_REGION_START) {
				if (i != 0U) {
					base_low  = HWIO_INXI(base,
						MC_ISU_ADDR_REGIONn_CFG2, i);
					base_high = HWIO_INXI(base,
						MC_ISU_ADDR_REGIONn_CFG3, i);
				}
				off_low  = HWIO_INXI(base,
					MC_ISU_ADDR_REGIONn_CFG0, i);
				off_high = HWIO_INXI(base,
					MC_ISU_ADDR_REGIONn_CFG1, i);
			} else {
				off_low  = HWIO_INXI(base,
					MC_ISU_ADDR_REGION_n_CFG0, i);
				off_high = HWIO_INXI(base,
					MC_ISU_ADDR_REGION_n_CFG1, i);
			}

			for (j = 1U; j < dev_config->num_channels; j++) {
				uint8_t *cbase = dev_config->trans_bases[j];

				if (i < ADDL_REGION_START && i != 0U) {
					cbase_low  = HWIO_INXI(cbase,
						MC_ISU_ADDR_REGIONn_CFG2, i);
					cbase_high = HWIO_INXI(cbase,
						MC_ISU_ADDR_REGIONn_CFG3, i);
					if (base_low != cbase_low ||
						base_high != cbase_high)
						return ICBCFG_ERROR;
				}

				if (i < ADDL_REGION_START) {
					cbase_low  = HWIO_INXI(cbase,
						MC_ISU_ADDR_REGIONn_CFG0, i);
					cbase_high = HWIO_INXI(cbase,
						MC_ISU_ADDR_REGIONn_CFG1, i);
				} else {
					cbase_low  = HWIO_INXI(cbase,
						MC_ISU_ADDR_REGION_n_CFG0, i);
					cbase_high = HWIO_INXI(cbase,
						MC_ISU_ADDR_REGION_n_CFG1, i);
				}

				if (off_low != cbase_low ||
					off_high != cbase_high)
					return ICBCFG_ERROR;
			}
		}
		break;

	default:
		return ICBCFG_ERROR;
	}

	return ICBCFG_SUCCESS;
}

static enum icbcfg_error_type
check_dup_region(struct icbcfg_device_config *dev_config)
{
	uint32_t		i, j;
	struct icb_region	region1, region2;

	if (dev_config->num_segments == 0U ||
	    dev_config->num_segments > MAX_REGIONS) {
		EMSG("ICB: invalid num_segments %u", dev_config->num_segments);
		return ICBCFG_ERROR;
	}

	if (compare_ch_segments(dev_config) != ICBCFG_SUCCESS)
		return ICBCFG_ERROR;

	for (i = 0U; i < dev_config->num_segments - 1U; i++) {
		if (addr_trans_segs[i].error)
			continue;

		if (addr_trans_segs[i].base >= addr_trans_segs[i + 1U].base)
			return ICBCFG_ERROR;

		region1.base_addr   = addr_trans_segs[i].base +
				      addr_trans_segs[i].offset;
		region1.size        = addr_trans_segs[i + 1U].base -
				      addr_trans_segs[i].base;
		region1.interleaved = dev_config->num_channels > 1U;

		for (j = i + 1U; j < dev_config->num_segments; j++) {
			if (addr_trans_segs[j].error)
				continue;

			if (addr_trans_segs[j].base >=
			   addr_trans_segs[j + 1U].base) {
				return ICBCFG_ERROR;
			}

			region2.base_addr   = addr_trans_segs[j].base +
					      addr_trans_segs[j].offset;
			region2.size        = addr_trans_segs[j + 1U].base -
					      addr_trans_segs[j].base;
			region2.interleaved = dev_config->num_channels > 1U;

			if (check_overlap(&region1, &region2))
				return ICBCFG_ERROR;
		}
	}

	return ICBCFG_SUCCESS;
}

void update_addr_translation(void)
{
	if (query_dev_config_valid)
		get_addr_translation(icb_dev_config);
}

enum icbcfg_error_type icb_get_memmap(struct icb_mem_map *info)
{
	struct icbcfg_device_config	*dev_config;

	if (!info)
		return ICBCFG_ERROR_INVALID_PARAM;

	if (!get_device_configuration(&dev_config))
		return ICBCFG_ERROR;

	get_system_memory_map(dev_config, info);
	return ICBCFG_SUCCESS;
}

enum icbcfg_error_type
icb_get_allowed_mem_region(struct icbcfg_mem_region *info)
{
	struct icbcfg_device_config	*dev_config;

	if (!info)
		return ICBCFG_ERROR_INVALID_PARAM;

	if (!get_device_configuration(&dev_config))
		return ICBCFG_ERROR;

	if (check_dup_region(dev_config) != ICBCFG_SUCCESS)
		return ICBCFG_ERROR;

	*info = dev_config->map_ddr_regions[0];
	return ICBCFG_SUCCESS;
}

enum icbcfg_error_type
icb_get_allowed_mem_region_ex(struct icbcfg_mem_region **info,
			     uint32_t *num_regions)
{
	struct icbcfg_device_config	*dev_config;

	if (!info || !num_regions)
		return ICBCFG_ERROR_INVALID_PARAM;

	if (!get_device_configuration(&dev_config))
		return ICBCFG_ERROR;

	if (check_dup_region(dev_config) != ICBCFG_SUCCESS)
		return ICBCFG_ERROR;

	*info        = dev_config->map_ddr_regions;
	*num_regions = dev_config->map_ddr_region_count;
	return ICBCFG_SUCCESS;
}

enum icbcfg_error_type icb_get_mc_addr(uint64_t soc_addr, uint64_t *mc_addr)
{
	struct icbcfg_device_config	*dev_config;

	if (!mc_addr)
		return ICBCFG_ERROR_INVALID_PARAM;

	if (!get_device_configuration(&dev_config))
		return ICBCFG_ERROR;

	return translate_address(dev_config, soc_addr, mc_addr);
}

enum icbcfg_error_type icb_get_soc_addr(uint64_t mc_addr, uint64_t *soc_addr)
{
	struct icbcfg_device_config	*dev_config;

	if (!soc_addr)
		return ICBCFG_ERROR_INVALID_PARAM;

	if (!get_device_configuration(&dev_config))
		return ICBCFG_ERROR;

	return translate_mc_address(dev_config, mc_addr, soc_addr);
}

enum icbcfg_error_type icb_get_num_ddr_channels(uint32_t *num_channels)
{
	struct icbcfg_device_config	*dev_config;

	if (!num_channels)
		return ICBCFG_ERROR_INVALID_PARAM;

	if (!get_device_configuration(&dev_config))
		return ICBCFG_ERROR;

	*num_channels = dev_config->num_channels;
	return ICBCFG_SUCCESS;
}
