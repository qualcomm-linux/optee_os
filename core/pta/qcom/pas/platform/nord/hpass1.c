// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <platform_config.h>
#include <resource_table.h>
#include <stdint.h>
#include <string.h>

#include "hpass1.h"

/*
 * QDSP6 boot registers for each HPS instance. The PUB block
 * holds the reset/boot FSM registers; the TCSR block holds the EVB select.
 */
#define HPASS_BASE_OFFSET				(0x2000000)
#define HPASS_TCSR_REGS_REG_OFFSET		(0x1580000)

#define HPASS_QDSP6SS_RST_EVB			(0x10)

#define HPASS_TCSR_EVB_SEL_OFFSET		(0x3000)
#define HPASS_TCSR_EVB_SEL_STRIDE		(0x1000)
#define HPASS_INSTANCE_IDX				(0x1)

#define HPASS_EFUSE_Q6SS_EVB_SEL		(HPASS_TCSR_REGS_REG_OFFSET - HPASS_BASE_OFFSET + \
						 HPASS_TCSR_EVB_SEL_OFFSET + \
						 (HPASS_TCSR_EVB_SEL_STRIDE * HPASS_INSTANCE_IDX))

#define BOOT_FSM_TIMEOUT				(10000)

static const struct fw_rsc_devmem hpass_mem_res[] = { };

DEFINE_RESOURCE_TABLE(HPASS, ARRAY_SIZE(hpass_mem_res));

static TEE_Result hpass_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);

	if (!base)
		return TEE_ERROR_GENERIC;

	/*
	 * Program the firmware entry address and select the programmed EVB;
	 */
	io_write32(base + HPASS_QDSP6SS_RST_EVB, data->fw_base >> 4);
	io_write32(base + HPASS_EFUSE_Q6SS_EVB_SEL, 0);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result hpass_fw_shutdown(struct qcom_pas_data *data __unused)
{
	return TEE_ERROR_NOT_IMPLEMENTED;
}

static TEE_Result hpass_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = HPASS_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(HPASS)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   hpass_mem_res,
			   HPASS_RESOURCE_TABLE_HEADER_SIZE,
			   HPASS_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops hpass1_ops = {
	.fw_start = hpass_fw_start,
	.fw_shutdown = hpass_fw_shutdown,
	.get_resource_table = hpass_get_resource_table,
};
