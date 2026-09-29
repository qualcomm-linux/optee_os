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

#include "clock_group_qcom.h"
#include "dtb_chip_id.h"
#include "nspss.h"

#define NSPSS_QDSP6SS_RST_EVB		(0x10)

#define BOOT_FSM_TIMEOUT	10000

/*
 * DTB boot-parameter registers, PUB-block-relative. Register 4 (0x70,
 * platform-info) is intentionally omitted, matching ipq96xx/cdsp.c.
 */
#define DTB_CONFIG_0_REG		0x60
#define DTB_CONFIG_1_REG		0x64
#define DTB_CONFIG_2_REG		0x68
#define DTB_CONFIG_3_REG		0x6c
#define DTB_CONFIG_5_REG		0x74

/* Per-instance resource tables; blank placeholders, filled in later per-instance */
static const struct fw_rsc_devmem nspss0_mem_res[] = { };
static const struct fw_rsc_devmem nspss1_mem_res[] = { };
static const struct fw_rsc_devmem nspss2_mem_res[] = { };
static const struct fw_rsc_devmem nspss3_mem_res[] = { };

DEFINE_RESOURCE_TABLE(NSPSS0, ARRAY_SIZE(nspss0_mem_res));
DEFINE_RESOURCE_TABLE(NSPSS1, ARRAY_SIZE(nspss1_mem_res));
DEFINE_RESOURCE_TABLE(NSPSS2, ARRAY_SIZE(nspss2_mem_res));
DEFINE_RESOURCE_TABLE(NSPSS3, ARRAY_SIZE(nspss3_mem_res));

/*
 * Program the firmware boot address only. Boot params (DTB) are programmed
 * separately by the DTB companion's own fw_start below, mirroring the TZ
 * split between nspss_bring_up() (RST_EVB only) and nspss_dtb_bring_up()
 * (QDSP6SS_BOOT_PARAMSi).
 */
static TEE_Result nspss_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);
	vaddr_t pub = base + CDSP_QDSP6SS_PUB_OFFSET;

	io_write32(pub + NSPSS_QDSP6SS_RST_EVB, data->fw_base >> 4);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result nspss_fw_shutdown(struct qcom_pas_data *data)
{
	return qcom_clock_pas_reset(data->clk_group);
}

/*
 * DTB companion boot routine, one per CDSP instance. The DTB subsystem is
 * window-less (no data->base/size — see pas_platform_mem_setup()'s
 * "window-less images" handling), so each instance maps the NSPSS block's
 * PUB registers itself off its own raw physical base, matching the way TZ's
 * nspss_dtb_bring_up() addresses its per-instance HWIO_OUTI() macros.
 */
static TEE_Result nspss_dtb_fw_start(struct qcom_pas_data *data,
				     paddr_t nsp_base, size_t nsp_size)
{
	struct io_pa_va iopv = { .pa = nsp_base };
	vaddr_t pub = io_pa_or_va(&iopv, nsp_size) + CDSP_QDSP6SS_PUB_OFFSET;

	if (!data->fw_base || !data->fw_size)
		return TEE_ERROR_BAD_PARAMETERS;

	io_write32(pub + DTB_CONFIG_0_REG, (uint32_t)data->fw_base);
	io_write32(pub + DTB_CONFIG_1_REG, (uint32_t)(data->fw_base >> 32));
	io_write32(pub + DTB_CONFIG_2_REG, DTB_CHIP_FAMILY_ID);
	io_write32(pub + DTB_CONFIG_3_REG, DTB_VERSION);
	io_write32(pub + DTB_CONFIG_5_REG, (uint32_t)data->fw_size);
	dsb();

	return TEE_SUCCESS;
}

static TEE_Result nspss0_dtb_fw_start(struct qcom_pas_data *data)
{
	return nspss_dtb_fw_start(data, CDSP_0_BASE, CDSP_0_SIZE);
}

static TEE_Result nspss1_dtb_fw_start(struct qcom_pas_data *data)
{
	return nspss_dtb_fw_start(data, CDSP_1_BASE, CDSP_1_SIZE);
}

static TEE_Result nspss2_dtb_fw_start(struct qcom_pas_data *data)
{
	return nspss_dtb_fw_start(data, CDSP_2_BASE, CDSP_2_SIZE);
}

static TEE_Result nspss3_dtb_fw_start(struct qcom_pas_data *data)
{
	return nspss_dtb_fw_start(data, CDSP_3_BASE, CDSP_3_SIZE);
}

static TEE_Result nspss_dtb_fw_shutdown(struct qcom_pas_data *data __unused)
{
	return TEE_SUCCESS;
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

const struct qcom_pas_ops nspss0_dtb_ops = {
	.fw_start = nspss0_dtb_fw_start,
	.fw_shutdown = nspss_dtb_fw_shutdown,
};

const struct qcom_pas_ops nspss1_dtb_ops = {
	.fw_start = nspss1_dtb_fw_start,
	.fw_shutdown = nspss_dtb_fw_shutdown,
};

const struct qcom_pas_ops nspss2_dtb_ops = {
	.fw_start = nspss2_dtb_fw_start,
	.fw_shutdown = nspss_dtb_fw_shutdown,
};

const struct qcom_pas_ops nspss3_dtb_ops = {
	.fw_start = nspss3_dtb_fw_start,
	.fw_shutdown = nspss_dtb_fw_shutdown,
};
