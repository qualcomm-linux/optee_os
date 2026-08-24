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

#include "nspss0.h"

#define NSPSS_QDSP6SS_RST_EVB		(0x10)

static const struct fw_rsc_devmem nspss_mem_res[] = { };

DEFINE_RESOURCE_TABLE(NSPSS, ARRAY_SIZE(nspss_mem_res));

#define BOOT_FSM_TIMEOUT	10000


static TEE_Result nspss0_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);

	/* Program firmware */
	io_write32(base + NSPSS_QDSP6SS_RST_EVB, data->fw_base >> 4);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result nspss0_fw_shutdown(struct qcom_pas_data *data)
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
		.num = NSPSS_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(NSPSS)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   nspss_mem_res,
			   NSPSS_RESOURCE_TABLE_HEADER_SIZE,
			   NSPSS_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops nspss0_ops = {
	.fw_start = nspss0_fw_start,
	.fw_shutdown = nspss0_fw_shutdown,
	.get_resource_table = nspss0_get_resource_table,
};
