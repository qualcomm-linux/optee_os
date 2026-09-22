// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <kernel/cache_helpers.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <resource_table.h>
#include <stdint.h>
#include <string.h>
#include <trace.h>

#include "nspss.h"

#define NSPSS_QDSP6SS_RST_EVB		(0x10)

#define BOOT_FSM_TIMEOUT	10000

/* Per-instance resource tables; blank placeholders, filled in later per-instance */
static const struct fw_rsc_devmem nspss0_mem_res[] = { };
static const struct fw_rsc_devmem nspss1_mem_res[] = { };
static const struct fw_rsc_devmem nspss2_mem_res[] = { };
static const struct fw_rsc_devmem nspss3_mem_res[] = { };

DEFINE_RESOURCE_TABLE(NSPSS0, ARRAY_SIZE(nspss0_mem_res));
DEFINE_RESOURCE_TABLE(NSPSS1, ARRAY_SIZE(nspss1_mem_res));
DEFINE_RESOURCE_TABLE(NSPSS2, ARRAY_SIZE(nspss2_mem_res));
DEFINE_RESOURCE_TABLE(NSPSS3, ARRAY_SIZE(nspss3_mem_res));

static TEE_Result nspss_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);

	/* Program firmware */
	io_write32(base + NSPSS_QDSP6SS_RST_EVB, data->fw_base >> 4);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result nspss_fw_shutdown(struct qcom_pas_data *data)
{
	return qcom_clock_pas_reset(data->clk_group);
}

static TEE_Result nspss0_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = NSPSS0_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(NSPSS0)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   nspss0_mem_res,
			   NSPSS0_RESOURCE_TABLE_HEADER_SIZE,
			   NSPSS0_RESOURCE_TABLE_SIZE);
}

static TEE_Result nspss1_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = NSPSS1_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(NSPSS1)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   nspss1_mem_res,
			   NSPSS1_RESOURCE_TABLE_HEADER_SIZE,
			   NSPSS1_RESOURCE_TABLE_SIZE);
}

static TEE_Result nspss2_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = NSPSS2_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(NSPSS2)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   nspss2_mem_res,
			   NSPSS2_RESOURCE_TABLE_HEADER_SIZE,
			   NSPSS2_RESOURCE_TABLE_SIZE);
}

static TEE_Result nspss3_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = NSPSS3_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(NSPSS3)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   nspss3_mem_res,
			   NSPSS3_RESOURCE_TABLE_HEADER_SIZE,
			   NSPSS3_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops nspss0_ops = {
	.fw_start = nspss_fw_start,
	.fw_shutdown = nspss_fw_shutdown,
	.get_resource_table = nspss0_get_resource_table,
};

const struct qcom_pas_ops nspss1_ops = {
	.fw_start = nspss_fw_start,
	.fw_shutdown = nspss_fw_shutdown,
	.get_resource_table = nspss1_get_resource_table,
};

const struct qcom_pas_ops nspss2_ops = {
	.fw_start = nspss_fw_start,
	.fw_shutdown = nspss_fw_shutdown,
	.get_resource_table = nspss2_get_resource_table,
};

const struct qcom_pas_ops nspss3_ops = {
	.fw_start = nspss_fw_start,
	.fw_shutdown = nspss_fw_shutdown,
	.get_resource_table = nspss3_get_resource_table,
};
