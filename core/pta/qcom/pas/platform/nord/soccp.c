// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk_qcom.h>
#include <io.h>
#include <kernel/delay.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <resource_table.h>
#include <stdint.h>

#include "soccp.h"

#define SOCCP_CSR_CNOC_REG_BASE		0xd40000
#define SOCCP_CSR_CNOC_REG_SIZE		0x61000
#define SOCCP_RVSSMP_RESET_VECTOR	0x3000c
#define SOCCP_SID3			0x60014

register_phys_mem(MEM_AREA_IO_NSEC, SOCCP_CSR_CNOC_REG_BASE,
		  SOCCP_CSR_CNOC_REG_SIZE);

static TEE_Result soccp_fw_start(struct qcom_pas_data *data)
{
	struct io_pa_va soccp_csr_io = { .pa = SOCCP_CSR_CNOC_REG_BASE };
	vaddr_t soccp_csr = io_pa_or_va(&soccp_csr_io, SOCCP_CSR_CNOC_REG_SIZE);

	if (!soccp_csr)
		return TEE_ERROR_GENERIC;

	io_write32(soccp_csr + SOCCP_RVSSMP_RESET_VECTOR, data->fw_base);
	io_write32(soccp_csr + SOCCP_SID3, 4);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result soccp_fw_shutdown(struct qcom_pas_data *data)
{
	return qcom_clock_pas_reset(data->clk_group);
}

static const struct fw_rsc_devmem soccp_mem_res[] = {
};

DEFINE_RESOURCE_TABLE(SOCCP, ARRAY_SIZE(soccp_mem_res));

static TEE_Result soccp_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = SOCCP_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(SOCCP)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   soccp_mem_res,
			   SOCCP_RESOURCE_TABLE_HEADER_SIZE,
			   SOCCP_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops soccp_ops = {
	.fw_start = soccp_fw_start,
	.fw_shutdown = soccp_fw_shutdown,
	.get_resource_table = soccp_get_resource_table,
};
