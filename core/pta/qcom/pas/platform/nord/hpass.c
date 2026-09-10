// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <platform_config.h>
#include <resource_table.h>
#include <stdint.h>
#include <string.h>

#include "hpass.h"

/*
 * QDSP6 boot registers for each HPS instance. The PUB block
 * holds the reset/boot FSM registers; the TCSR block holds the EVB select.
 */
#define HPASS_TCSR_REGS_REG_OFFSET		(0x1580000)

#define HPASS_QDSP6SS_RST_EVB			(0x10)

#define HPASS_TCSR_EVB_SEL_OFFSET		(0x3000)
#define HPASS_TCSR_EVB_SEL_STRIDE		(0x1000)

#define BOOT_FSM_TIMEOUT			(10000)

#define HPASS0_BASE_OFFSET			(0xC00000)
#define HPASS1_BASE_OFFSET			(0x2000000)
#define HPASS2_BASE_OFFSET			(0x2100000)

#define HPASS0_IDX				(0)
#define HPASS1_IDX				(1)
#define HPASS2_IDX				(2)

/* Per-instance resource tables; blank placeholders, filled in later per-instance */
static const struct fw_rsc_devmem hpass0_mem_res[] = { };
static const struct fw_rsc_devmem hpass1_mem_res[] = { };
static const struct fw_rsc_devmem hpass2_mem_res[] = { };

DEFINE_RESOURCE_TABLE(HPASS0, ARRAY_SIZE(hpass0_mem_res));
DEFINE_RESOURCE_TABLE(HPASS1, ARRAY_SIZE(hpass1_mem_res));
DEFINE_RESOURCE_TABLE(HPASS2, ARRAY_SIZE(hpass2_mem_res));

/*
 * Shared boot routine. base_offset/idx reproduce each instance's former
 * HPASS_EFUSE_Q6SS_EVB_SEL macro exactly, now passed in as call-site
 * constants instead of being duplicated per file.
 */
static TEE_Result hpass_fw_start(struct qcom_pas_data *data,
				 uint32_t base_offset, uint32_t idx)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);
	uint32_t evb_sel = HPASS_TCSR_REGS_REG_OFFSET - base_offset +
			   HPASS_TCSR_EVB_SEL_OFFSET +
			   HPASS_TCSR_EVB_SEL_STRIDE * idx;

	if (!base)
		return TEE_ERROR_GENERIC;

	/*
	 * Program the firmware entry address and select the programmed EVB;
	 * the Q6 PLL and core RCG are already configured by hpass_setup().
	 */
	io_write32(base + HPASS_QDSP6SS_RST_EVB, data->fw_base >> 4);
	io_write32(base + evb_sel, 0);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result hpass0_fw_start(struct qcom_pas_data *data)
{
	return hpass_fw_start(data, HPASS0_BASE_OFFSET, HPASS0_IDX);
}

static TEE_Result hpass1_fw_start(struct qcom_pas_data *data)
{
	return hpass_fw_start(data, HPASS1_BASE_OFFSET, HPASS1_IDX);
}

static TEE_Result hpass2_fw_start(struct qcom_pas_data *data)
{
	return hpass_fw_start(data, HPASS2_BASE_OFFSET, HPASS2_IDX);
}

static TEE_Result hpass_fw_shutdown(struct qcom_pas_data *data __unused)
{
	return TEE_ERROR_NOT_IMPLEMENTED;
}

static TEE_Result hpass0_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = HPASS0_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(HPASS0)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   hpass0_mem_res,
			   HPASS0_RESOURCE_TABLE_HEADER_SIZE,
			   HPASS0_RESOURCE_TABLE_SIZE);
}

static TEE_Result hpass1_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = HPASS1_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(HPASS1)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   hpass1_mem_res,
			   HPASS1_RESOURCE_TABLE_HEADER_SIZE,
			   HPASS1_RESOURCE_TABLE_SIZE);
}

static TEE_Result hpass2_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = HPASS2_NUM_MEM_RESOURCES,
		.offset[RESOURCE_TABLE_OFFSET_LAST(HPASS2)] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header,
			   hpass2_mem_res,
			   HPASS2_RESOURCE_TABLE_HEADER_SIZE,
			   HPASS2_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops hpass0_ops = {
	.fw_start = hpass0_fw_start,
	.fw_shutdown = hpass_fw_shutdown,
	.get_resource_table = hpass0_get_resource_table,
};

const struct qcom_pas_ops hpass1_ops = {
	.fw_start = hpass1_fw_start,
	.fw_shutdown = hpass_fw_shutdown,
	.get_resource_table = hpass1_get_resource_table,
};

const struct qcom_pas_ops hpass2_ops = {
	.fw_start = hpass2_fw_start,
	.fw_shutdown = hpass_fw_shutdown,
	.get_resource_table = hpass2_get_resource_table,
};
