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

static const struct fw_rsc_devmem hpass_mem_res[] = {
	{ .name = "gcc_gpll0", .flags = IOMMU_READ,
		.da = 0x00100000, .pa = 0x00100000, .len = 0x00001000, },
	{ .name = "gcc_gpll1", .flags = IOMMU_READ,
		.da = 0x00101000, .pa = 0x00101000, .len = 0x00001000, },
	{ .name = "gcc_gpll2", .flags = IOMMU_READ,
		.da = 0x00102000, .pa = 0x00102000, .len = 0x00001000, },
	{ .name = "gcc_gpll3", .flags = IOMMU_READ,
		.da = 0x00103000, .pa = 0x00103000, .len = 0x00001000, },
	{ .name = "gcc_gpll4", .flags = IOMMU_READ,
		.da = 0x00104000, .pa = 0x00104000, .len = 0x00001000, },
	{ .name = "gcc_gpll5", .flags = IOMMU_READ,
		.da = 0x00105000, .pa = 0x00105000, .len = 0x00001000, },
	{ .name = "gcc_gpll6", .flags = IOMMU_READ,
		.da = 0x00106000, .pa = 0x00106000, .len = 0x00001000, },
	{ .name = "gcc_gpll7", .flags = IOMMU_READ,
		.da = 0x00107000, .pa = 0x00107000, .len = 0x00001000, },
	{ .name = "gcc_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0011f000, .pa = 0x0011f000, .len = 0x00001000, },
	{ .name = "gcc_1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00123000, .pa = 0x00123000, .len = 0x00001000, },
	{ .name = "gcc_2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00128000, .pa = 0x00128000, .len = 0x00001000, },
	{ .name = "gcc_3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0012f000, .pa = 0x0012f000, .len = 0x00001000, },
	{ .name = "gcc_4", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00147000, .pa = 0x00147000, .len = 0x00001000, },
	{ .name = "gcc_5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00162000, .pa = 0x00162000, .len = 0x00001000, },
	{ .name = "gcc_6", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x001b9000, .pa = 0x001b9000, .len = 0x00004000, },
	{ .name = "rng_cm_cm_soc_qr", .flags = IOMMU_READ,
		.da = 0x010c0000, .pa = 0x010c0000, .len = 0x00001000, },
	{ .name = "rng_ee4_ee4_soc", .flags = IOMMU_READ,
		.da = 0x010c4000, .pa = 0x010c4000, .len = 0x00001000, },
	{ .name = "tcsr_tcsr_regs_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01f40000, .pa = 0x01f40000, .len = 0x00073000, },
	{ .name = "tcsr_tcsr_regs_1", .flags = IOMMU_READ,
		.da = 0x01fc8000, .pa = 0x01fc8000, .len = 0x00001000, },
	{ .name = "tcsr_tcsr_regs_2", .flags = IOMMU_READ,
		.da = 0x01fd4000, .pa = 0x01fd4000, .len = 0x00001000, },
	{ .name = "hpass_tcm", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x04800000, .pa = 0x04800000, .len = 0x00080000, },
	{ .name = "hpass_lpass_stm", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x05400000, .pa = 0x05400000, .len = 0x00100000, },
	{ .name = "usb2phy_2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x088e0000, .pa = 0x088e0000, .len = 0x00001000, },
	{ .name = "eud_mode_manager", .flags = IOMMU_READ,
		.da = 0x088e9000, .pa = 0x088e9000, .len = 0x00001000, },
	{ .name = "usb2phy_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x088ea000, .pa = 0x088ea000, .len = 0x00001000, },
	{ .name = "usb2phy_1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x088eb000, .pa = 0x088eb000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll0", .flags = IOMMU_READ,
		.da = 0x08900000, .pa = 0x08900000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll1", .flags = IOMMU_READ,
		.da = 0x08901000, .pa = 0x08901000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll2", .flags = IOMMU_READ,
		.da = 0x08902000, .pa = 0x08902000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll3", .flags = IOMMU_READ,
		.da = 0x08903000, .pa = 0x08903000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll4", .flags = IOMMU_READ,
		.da = 0x08904000, .pa = 0x08904000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll5", .flags = IOMMU_READ,
		.da = 0x08905000, .pa = 0x08905000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll6", .flags = IOMMU_READ,
		.da = 0x08906000, .pa = 0x08906000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll7", .flags = IOMMU_READ,
		.da = 0x08907000, .pa = 0x08907000, .len = 0x00001000, },
	{ .name = "ne_gcc_gpll8", .flags = IOMMU_READ,
		.da = 0x08908000, .pa = 0x08908000, .len = 0x00001000, },
	{ .name = "ne_gcc_ne_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x08914000, .pa = 0x08914000, .len = 0x00001000, },
	{ .name = "ne_gcc_ne_1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x08920000, .pa = 0x08920000, .len = 0x00003000, },
	{ .name = "ne_gcc_ne_2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x08938000, .pa = 0x08938000, .len = 0x00001000, },
	{ .name = "se_gcc_gpll0", .flags = IOMMU_READ,
		.da = 0x08a00000, .pa = 0x08a00000, .len = 0x00001000, },
	{ .name = "se_gcc_gpll1", .flags = IOMMU_READ,
		.da = 0x08a01000, .pa = 0x08a01000, .len = 0x00001000, },
	{ .name = "se_gcc_gpll2", .flags = IOMMU_READ,
		.da = 0x08a02000, .pa = 0x08a02000, .len = 0x00001000, },
	{ .name = "se_gcc_gpll3", .flags = IOMMU_READ,
		.da = 0x08a03000, .pa = 0x08a03000, .len = 0x00001000, },
	{ .name = "se_gcc_gpll4", .flags = IOMMU_READ,
		.da = 0x08a04000, .pa = 0x08a04000, .len = 0x00001000, },
	{ .name = "se_gcc_gpll5", .flags = IOMMU_READ,
		.da = 0x08a05000, .pa = 0x08a05000, .len = 0x00001000, },
	{ .name = "se_gcc_gpll6", .flags = IOMMU_READ,
		.da = 0x08a06000, .pa = 0x08a06000, .len = 0x00001000, },
	{ .name = "se_gcc_se_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x08a18000, .pa = 0x08a18000, .len = 0x00003000, },
	{ .name = "se_gcc_se_1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x08a1d000, .pa = 0x08a1d000, .len = 0x00001000, },
	{ .name = "se_gcc_se_2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x08a26000, .pa = 0x08a26000, .len = 0x00002000, },
	{ .name = "nw_gcc_gpll0", .flags = IOMMU_READ,
		.da = 0x08b00000, .pa = 0x08b00000, .len = 0x00001000, },
	{ .name = "nw_gcc_gpll1", .flags = IOMMU_READ,
		.da = 0x08b01000, .pa = 0x08b01000, .len = 0x00001000, },
	{ .name = "nw_gcc_gpll2", .flags = IOMMU_READ,
		.da = 0x08b02000, .pa = 0x08b02000, .len = 0x00001000, },
	{ .name = "nw_gcc_gpll3", .flags = IOMMU_READ,
		.da = 0x08b03000, .pa = 0x08b03000, .len = 0x00001000, },
	{ .name = "nw_gcc_gpll4", .flags = IOMMU_READ,
		.da = 0x08b04000, .pa = 0x08b04000, .len = 0x00001000, },
	{ .name = "nw_gcc_gpll5", .flags = IOMMU_READ,
		.da = 0x08b05000, .pa = 0x08b05000, .len = 0x00001000, },
	{ .name = "nw_gcc_nw_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x08b1f000, .pa = 0x08b1f000, .len = 0x00003000, },
	{ .name = "nw_gcc_nw_1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x08b32000, .pa = 0x08b32000, .len = 0x00001000, },
	{ .name = "ipc_soc_ipc_co_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x09002000, .pa = 0x09002000, .len = 0x00001000, },
	{ .name = "ipc_soc_ipc_co_1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0900b000, .pa = 0x0900b000, .len = 0x00002000, },
	{ .name = "ipc_soc_ipc_co_2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x090c0000, .pa = 0x090c0000, .len = 0x00001000, },
	{ .name = "ipc_soc_ipc_co_3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x090d0000, .pa = 0x090d0000, .len = 0x00002000, },
	{ .name = "ipc_soc_ipc_co_4", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x09180000, .pa = 0x09180000, .len = 0x00001000, },
	{ .name = "ipc_soc_ipc_co_5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x09190000, .pa = 0x09190000, .len = 0x00002000, },
	{ .name = "ipc_soc_ipc_co_6", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x09240000, .pa = 0x09240000, .len = 0x00001000, },
	{ .name = "ipc_soc_ipc_co_7", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0924b000, .pa = 0x0924b000, .len = 0x00002000, },
	{ .name = "ipc_soc_ipc_co_8", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x09300000, .pa = 0x09300000, .len = 0x00001000, },
	{ .name = "ipc_soc_ipc_co_9", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0931d000, .pa = 0x0931d000, .len = 0x00002000, },
	{ .name = "usb3_1_usb31", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0a400000, .pa = 0x0a400000, .len = 0x0000e000, },
	{ .name = "usb3_0_usb31", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0a600000, .pa = 0x0a600000, .len = 0x0000e000, },
	{ .name = "rpmh_pdc_pdc_wra", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0b200000, .pa = 0x0b200000, .len = 0x00400000, },
	{ .name = "rpmh_bcm", .flags = IOMMU_READ,
		.da = 0x0bbf0000, .pa = 0x0bbf0000, .len = 0x00003000, },
	{ .name = "aop_ss_msg_ram_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c300000, .pa = 0x0c300000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c301000, .pa = 0x0c301000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c302000, .pa = 0x0c302000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c303000, .pa = 0x0c303000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_4", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c304000, .pa = 0x0c304000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c305000, .pa = 0x0c305000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_6", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c306000, .pa = 0x0c306000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_7", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c307000, .pa = 0x0c307000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_8", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c308000, .pa = 0x0c308000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_9", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c309000, .pa = 0x0c309000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_10", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c30a000, .pa = 0x0c30a000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_11", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c30b000, .pa = 0x0c30b000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_12", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c30c000, .pa = 0x0c30c000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_13", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c30d000, .pa = 0x0c30d000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_14", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c30e000, .pa = 0x0c30e000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_15", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c30f000, .pa = 0x0c30f000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_16", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c310000, .pa = 0x0c310000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_17", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c311000, .pa = 0x0c311000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_18", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c312000, .pa = 0x0c312000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_19", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c313000, .pa = 0x0c313000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_20", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c314000, .pa = 0x0c314000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_21", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c315000, .pa = 0x0c315000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_22", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c316000, .pa = 0x0c316000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_23", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c317000, .pa = 0x0c317000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_24", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c318000, .pa = 0x0c318000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_25", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c319000, .pa = 0x0c319000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_26", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c31a000, .pa = 0x0c31a000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_27", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c31b000, .pa = 0x0c31b000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_28", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c31c000, .pa = 0x0c31c000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_29", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c31d000, .pa = 0x0c31d000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_30", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c31e000, .pa = 0x0c31e000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ra_31", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c31f000, .pa = 0x0c31f000, .len = 0x00001000, },
	{ .name = "qdss_apb", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10000000, .pa = 0x10000000, .len = 0x00080000, },
	{ .name = "qdss_hpass_stm_s", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x119b2000, .pa = 0x119b2000, .len = 0x00001000, },
	{ .name = "system_imem", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x14680000, .pa = 0x14680000, .len = 0x00001000, },
	{ .name = "qdss_stm", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x16000000, .pa = 0x16000000, .len = 0x01000000, },
	{ .name = "qfprom_raw", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x360c0000, .pa = 0x360c0000, .len = 0x00008000, },
	{ .name = "fuse_controller", .flags = IOMMU_READ,
		.da = 0x360d4000, .pa = 0x360d4000, .len = 0x00004000, },
	{ .name = "ddr_interleave_0", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x821ff000, .pa = 0x821ff000, .len = 0x00001000, },
	{ .name = "ddr_interleave_1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x87148000, .pa = 0x87148000, .len = 0x00020000, },
	{ .name = "ddr_interleave_2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0xba200000, .pa = 0xba200000, .len = 0x00200000, },
	{ .name = "ddr_interleave_3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0xbb600000, .pa = 0xbb600000, .len = 0x00a00000, },
	{ .name = "ddr_interleave_4", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0xd4e00000, .pa = 0xd4e00000, .len = 0x02000000, },
	/* SMEM (smem@90900000, 4 MB) holds the GLINK FIFOs the DSP must
	 * read/write to talk to the apps processor. Without this mapping the
	 * DSP takes a translation fault on its first SMEM access -> mmufault/watchdog and never reaches running.
	 */
	{ .name = "smem", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x89B00000, .pa = 0x89B00000, .len = 0x400000, },
};

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
