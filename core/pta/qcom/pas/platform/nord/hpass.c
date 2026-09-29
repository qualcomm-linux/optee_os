// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <platform_config.h>
#include <resource_table.h>
#include <stdint.h>
#include <string.h>

#include "dtb_chip_id.h"
#include "hpass.h"

/*
 * QDSP6 boot registers for each HPS instance. The PUB block
 * holds the reset/boot FSM registers; the TCSR block holds the EVB select.
 */
#define HPASS_QDSP6SS_RST_EVB			(0x10)

#define HPASS_TCSR_EVB_SEL_OFFSET		(0x3000)
#define HPASS_TCSR_EVB_SEL_STRIDE		(0x1000)

#define BOOT_FSM_TIMEOUT			(10000)

#define HPASS0_IDX				(0)
#define HPASS1_IDX				(1)
#define HPASS2_IDX				(2)

/*
 * DTB boot-parameter registers, HPASS-instance-base-relative (HPASS's PUB
 * block starts at the instance base directly, unlike CDSP). Register 4
 * (0x70, platform-info) is intentionally omitted, matching ipq96xx/cdsp.c.
 */
#define DTB_CONFIG_0_REG			0x60
#define DTB_CONFIG_1_REG			0x64
#define DTB_CONFIG_2_REG			0x68
#define DTB_CONFIG_3_REG			0x6c
#define DTB_CONFIG_5_REG			0x74

/* Per-instance resource tables; blank placeholders, filled in later per-instance */
static const struct fw_rsc_devmem hpass0_mem_res[] = { };
static const struct fw_rsc_devmem hpass1_mem_res[] = { };
static const struct fw_rsc_devmem hpass2_mem_res[] = { };

DEFINE_RESOURCE_TABLE(HPASS0, ARRAY_SIZE(hpass0_mem_res));
DEFINE_RESOURCE_TABLE(HPASS1, ARRAY_SIZE(hpass1_mem_res));
DEFINE_RESOURCE_TABLE(HPASS2, ARRAY_SIZE(hpass2_mem_res));

/*
 * Shared boot routine. idx reproduces each instance's former
 * HPASS_EFUSE_Q6SS_EVB_SEL macro exactly, now passed in as a call-site
 * constant instead of being duplicated per file. The EVB-select register
 * is indexed off HPASS_TCSR_BASE directly rather than being derived by
 * subtracting a per-instance offset from the PUB-block base, since that
 * subtraction underflows for HPASS1/HPASS2.
 *
 * Boot params (DTB) are programmed separately by the DTB companion's own
 * fw_start below, mirroring TZ's split between hpass_program_boot_addr()
 * (RST_EVB + EVB_SEL) and hpass_dtb_bring_up() (BOOT_PARAMS).
 */
static TEE_Result hpass_fw_start(struct qcom_pas_data *data, uint32_t idx)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);
	struct io_pa_va tcsr_iopv = { .pa = HPASS_TCSR_BASE };
	vaddr_t tcsr = io_pa_or_va(&tcsr_iopv, HPASS_TCSR_SIZE);
	uint32_t evb_sel = HPASS_TCSR_EVB_SEL_OFFSET +
			   HPASS_TCSR_EVB_SEL_STRIDE * idx;

	if (!base || !tcsr)
		return TEE_ERROR_GENERIC;

	/*
	 * Program the firmware entry address and select the programmed EVB;
	 * the Q6 PLL and core RCG are already configured by hpass_setup().
	 */
	io_write32(base + HPASS_QDSP6SS_RST_EVB, data->fw_base >> 4);
	io_write32(tcsr + evb_sel, 0);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result hpass0_fw_start(struct qcom_pas_data *data)
{
	return hpass_fw_start(data, HPASS0_IDX);
}

static TEE_Result hpass1_fw_start(struct qcom_pas_data *data)
{
	return hpass_fw_start(data, HPASS1_IDX);
}

static TEE_Result hpass2_fw_start(struct qcom_pas_data *data)
{
	return hpass_fw_start(data, HPASS2_IDX);
}

static TEE_Result hpass_fw_shutdown(struct qcom_pas_data *data)
{
	return qcom_clock_pas_reset(data->clk_group);
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

/*
 * DTB companion boot routine, one per HPASS instance. The DTB subsystem is
 * window-less (no data->base/size — see pas_platform_mem_setup()'s
 * "window-less images" handling), so each instance maps the HPASS PUB block
 * itself off its own raw physical base, matching TZ's hpass_dtb_bring_up().
 */
static TEE_Result hpass_dtb_fw_start(struct qcom_pas_data *data,
				     paddr_t hps_base, size_t hps_size)
{
	struct io_pa_va iopv = { .pa = hps_base };
	vaddr_t base = io_pa_or_va(&iopv, hps_size);

	if (!data->fw_base || !data->fw_size)
		return TEE_ERROR_BAD_PARAMETERS;

	io_write32(base + DTB_CONFIG_0_REG, (uint32_t)data->fw_base);
	io_write32(base + DTB_CONFIG_1_REG, (uint32_t)(data->fw_base >> 32));
	io_write32(base + DTB_CONFIG_2_REG, DTB_CHIP_FAMILY_ID);
	io_write32(base + DTB_CONFIG_3_REG, DTB_VERSION);
	io_write32(base + DTB_CONFIG_5_REG, (uint32_t)data->fw_size);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result hpass0_dtb_fw_start(struct qcom_pas_data *data)
{
	return hpass_dtb_fw_start(data, HPASS_0_BASE, HPASS_0_SIZE);
}

static TEE_Result hpass1_dtb_fw_start(struct qcom_pas_data *data)
{
	return hpass_dtb_fw_start(data, HPASS_1_BASE, HPASS_1_SIZE);
}

static TEE_Result hpass2_dtb_fw_start(struct qcom_pas_data *data)
{
	return hpass_dtb_fw_start(data, HPASS_2_BASE, HPASS_2_SIZE);
}

static TEE_Result hpass_dtb_fw_shutdown(struct qcom_pas_data *data __unused)
{
	return TEE_SUCCESS;
}

const struct qcom_pas_ops hpass0_dtb_ops = {
	.fw_start = hpass0_dtb_fw_start,
	.fw_shutdown = hpass_dtb_fw_shutdown,
};

const struct qcom_pas_ops hpass1_dtb_ops = {
	.fw_start = hpass1_dtb_fw_start,
	.fw_shutdown = hpass_dtb_fw_shutdown,
};

const struct qcom_pas_ops hpass2_dtb_ops = {
	.fw_start = hpass2_dtb_fw_start,
	.fw_shutdown = hpass_dtb_fw_shutdown,
};
