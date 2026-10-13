// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <pta_qcom_pas.h>
#include <resource_table.h>
#include <stdint.h>

#include "lmcu.h"

#define LMCU_NONCACHE_REMAP_STRIDE    0x8
#define LMCU_NONCACHE_SID_STRIDE      0x4
#define MAX_NONCACHE_REMAPPER         16
#define MAX_CACHE_REMAPPER            1

static const uint32_t lmcu_noncache_remapper_values[] = {
	0x00000001, 0x04000001, 0x08000001, 0x0c000001,
	0x10000001, 0x14000001, 0x18000001, 0x1c000001,
	0x20000001, 0x24000001, 0x28000001, 0x2c000001,
	0x30000001, 0x34000001, 0x38000001, 0x3c000001,
};

struct lmcu_regs {
	uint32_t noncache_sid_remapper;
	uint32_t cache_sid_remapper;
	uint32_t noncache_sid;
	uint32_t cache_sid;
	uint32_t tile0_reset_vector;
	uint32_t spare_reg0;
};

static const struct lmcu_regs lmcu0_regs = {
	.noncache_sid_remapper = 0x208d000,
	.cache_sid_remapper = 0x208d100,
	.noncache_sid = 0x208d200,
	.cache_sid = 0x208d300,
	.tile0_reset_vector = 0x208c018,
	.spare_reg0 = 0x2080100,
};

static const struct lmcu_regs lmcu1_regs = {
	.noncache_sid_remapper = 0x248d000,
	.cache_sid_remapper = 0x248d100,
	.noncache_sid = 0x248d200,
	.cache_sid = 0x248d300,
	.tile0_reset_vector = 0x248c018,
	.spare_reg0 = 0x2480100,
};

static const struct fw_rsc_devmem lmcu0_mem_res[] = {
	{ .name = "lpass_lmcu0_noncache_remapper",
		.flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x208d000,
		.pa = LPASS_BASE + 0x208d000, .len = 0xc0, },
	{ .name = "lpass_lmcu0_cache_remapper",
		.flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x208d100,
		.pa = LPASS_BASE + 0x208d100, .len = 0x60, },
	{ .name = "lpass_lmcu0_noncache_sid",
		.flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x208d200,
		.pa = LPASS_BASE + 0x208d200, .len = 0x60, },
	{ .name = "lpass_lmcu0_cache_sid",
		.flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x208d300,
		.pa = LPASS_BASE + 0x208d300, .len = 0x30, },
	{ .name = "lpass_lmcu0_cc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x2229000,
		.pa = LPASS_BASE + 0x2229000, .len = 0x3000, },
	{ .name = "tcsr", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = TCSR_BASE, .pa = TCSR_BASE, .len = TCSR_SIZE, },
};

static const struct fw_rsc_devmem lmcu1_mem_res[] = {
	{ .name = "lpass_lmcu1_noncache_remapper",
		.flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x248d000,
		.pa = LPASS_BASE + 0x248d000, .len = 0xc0, },
	{ .name = "lpass_lmcu1_cache_remapper",
		.flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x248d100,
		.pa = LPASS_BASE + 0x248d100, .len = 0x60, },
	{ .name = "lpass_lmcu1_noncache_sid",
		.flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x248d200,
		.pa = LPASS_BASE + 0x248d200, .len = 0x60, },
	{ .name = "lpass_lmcu1_cache_sid",
		.flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x248d300,
		.pa = LPASS_BASE + 0x248d300, .len = 0x30, },
	{ .name = "lpass_lmcu1_cc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = LPASS_BASE + 0x2629000,
		.pa = LPASS_BASE + 0x2629000, .len = 0x3000, },
	{ .name = "tcsr", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = TCSR_BASE, .pa = TCSR_BASE, .len = TCSR_SIZE, },
};

DEFINE_RESOURCE_TABLE(LMCU0, ARRAY_SIZE(lmcu0_mem_res));
DEFINE_RESOURCE_TABLE(LMCU1, ARRAY_SIZE(lmcu1_mem_res));

static struct qcom_pas_data *lmcu_dtb_data(uint32_t pas_id)
{
	struct qcom_pas_subsys *subsystems = NULL;
	size_t count = 0;
	size_t i;

	subsystems = qcom_pas_platform_subsys(&count);
	for (i = 0; i < count; i++)
		if (subsystems[i].data.pas_id == pas_id)
			return &subsystems[i].data;

	return NULL;
}

static TEE_Result lmcu_fw_start(struct qcom_pas_data *data,
				const struct lmcu_regs *regs,
				uint32_t dtb_pas_id)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);
	struct qcom_pas_data *dtb = lmcu_dtb_data(dtb_pas_id);
	unsigned int i;

	if (!base || !dtb || !dtb->fw_base)
		return TEE_ERROR_NO_DATA;

	for (i = 0; i < ARRAY_SIZE(lmcu_noncache_remapper_values); i++)
		io_write32(base + regs->noncache_sid_remapper +
				   i * LMCU_NONCACHE_REMAP_STRIDE,
				   lmcu_noncache_remapper_values[i]);

	for (i = 0; i < MAX_NONCACHE_REMAPPER; i++)
		io_write32(base + regs->noncache_sid +
				   i * LMCU_NONCACHE_SID_STRIDE,
				   0);
	io_write32(base + regs->noncache_sid +
			   MAX_NONCACHE_REMAPPER * LMCU_NONCACHE_SID_STRIDE, 1);

	for (i = 0; i < MAX_CACHE_REMAPPER; i++)
		io_write32(base + regs->cache_sid_remapper +
				   i * LMCU_NONCACHE_REMAP_STRIDE, 0x08000001);

	io_clrbits32(base + regs->cache_sid, 0xf);
	io_clrbits32(base + regs->cache_sid +
		   MAX_CACHE_REMAPPER * LMCU_NONCACHE_SID_STRIDE, 0xf);

	io_write32(base + regs->tile0_reset_vector, data->fw_base);
	io_write32(base + regs->spare_reg0, dtb->fw_base);

	return TEE_SUCCESS;
}

static TEE_Result lmcu0_fw_start(struct qcom_pas_data *data)
{
	return lmcu_fw_start(data, &lmcu0_regs, DTB_ID_LMCU0);
}

static TEE_Result lmcu1_fw_start(struct qcom_pas_data *data)
{
	return lmcu_fw_start(data, &lmcu1_regs, DTB_ID_LMCU1);
}

static TEE_Result lmcu_fw_shutdown(struct qcom_pas_data *data)
{
	return qcom_clock_pas_reset(data->clk_group);
}

static TEE_Result lmcu_get_resource_table(struct resource_table *rt,
					  size_t *rt_size,
					  struct resource_table *table,
					  const struct fw_rsc_devmem *mem_res,
					  size_t header_size, size_t size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};

	return get_mem_rsc(rt, rt_size, table, &header, mem_res, header_size,
			   size);
}

static TEE_Result lmcu0_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	static struct resource_table table = {
		.ver = 1,
		.num = LMCU0_NUM_MEM_RESOURCES,
		.offset[LMCU0_NUM_MEM_RESOURCES - 1] = 0,
	};

	return lmcu_get_resource_table(rt, rt_size, &table, lmcu0_mem_res,
					   LMCU0_RESOURCE_TABLE_HEADER_SIZE,
					   LMCU0_RESOURCE_TABLE_SIZE);
}

static TEE_Result lmcu1_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	static struct resource_table table = {
		.ver = 1,
		.num = LMCU1_NUM_MEM_RESOURCES,
		.offset[LMCU1_NUM_MEM_RESOURCES - 1] = 0,
	};

	return lmcu_get_resource_table(rt, rt_size, &table, lmcu1_mem_res,
					   LMCU1_RESOURCE_TABLE_HEADER_SIZE,
					   LMCU1_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops lmcu0_ops = {
	.fw_start = lmcu0_fw_start,
	.fw_shutdown = lmcu_fw_shutdown,
	.get_resource_table = lmcu0_get_resource_table,
};

const struct qcom_pas_ops lmcu1_ops = {
	.fw_start = lmcu1_fw_start,
	.fw_shutdown = lmcu_fw_shutdown,
	.get_resource_table = lmcu1_get_resource_table,
};

static TEE_Result lmcu_dtb_fw_start(struct qcom_pas_data *data __unused)
{
	return TEE_SUCCESS;
}

const struct qcom_pas_ops lmcu0_dtb_ops = {
	.fw_start = lmcu_dtb_fw_start,
};

const struct qcom_pas_ops lmcu1_dtb_ops = {
	.fw_start = lmcu_dtb_fw_start,
};
