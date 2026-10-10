// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <resource_table.h>
#include <stdint.h>

#include "lpass.h"

#define LPASS_QDSP6SS_RST_EVB             0xc00010
#define LPASS_LPASS_EFUSE_Q6SS_EVB_SEL    0x12eb000

static const struct fw_rsc_devmem lpass_mem_res[] = {
	{ .name = "gcc_lpass", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = GCC_BASE + 0x47000, .pa = GCC_BASE + 0x47000,
		.len = 0x1000, },
	{ .name = "tcsr", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = TCSR_BASE, .pa = TCSR_BASE, .len = TCSR_SIZE, },
	{ .name = "aoss_cc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = AOSS_BASE, .pa = AOSS_BASE, .len = AOSS_SIZE, },
};

DEFINE_RESOURCE_TABLE(LPASS, ARRAY_SIZE(lpass_mem_res));

static TEE_Result lpass_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);

	if (!base)
		return TEE_ERROR_GENERIC;

	io_write32(base + LPASS_QDSP6SS_RST_EVB, data->fw_base >> 8);
	io_write32(base + LPASS_LPASS_EFUSE_Q6SS_EVB_SEL, 0);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result lpass_fw_shutdown(struct qcom_pas_data *data)
{
	return qcom_clock_pas_reset(data->clk_group);
}

static TEE_Result lpass_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = LPASS_NUM_MEM_RESOURCES,
		.offset[LPASS_NUM_MEM_RESOURCES - 1] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header, lpass_mem_res,
			   LPASS_RESOURCE_TABLE_HEADER_SIZE,
			   LPASS_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops lpass_ops = {
	.fw_start = lpass_fw_start,
	.fw_shutdown = lpass_fw_shutdown,
	.get_resource_table = lpass_get_resource_table,
};
