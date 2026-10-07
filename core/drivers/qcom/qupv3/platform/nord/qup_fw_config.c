// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Nord QUPv3 SE firmware loading configuration, derived from the TZ
 * qup_accesscontrol nord QUPAC_Private.c / QUPAC_Access.c / QUPAC_FW_Array.c.
 */

#include <drivers/qcom/qup_fw_load.h>
#include <mm/core_mmu.h>
#include <util.h>

/* FW segments are loaded to DDR by the boot chain (flashless) */
#define QUPV3_FW_DDR_BASE		UL(0x87574000)
#define QUPV3_FW_DDR_SIZE		UL(0x18000)

#define QUPV3_WRAP0_GSI_BASE		UL(0x00904000)
#define QUPV3_WRAP1_GSI_BASE		UL(0x00a04000)
#define QUPV3_WRAP2_GSI_BASE		UL(0x00804000)
#define QUPV3_GSI_SIZE			UL(0x58000)

#define QUPV3_WRAP0_BASE		UL(0x00980000)
#define QUPV3_WRAP1_BASE		UL(0x00a80000)
#define QUPV3_WRAP2_BASE		UL(0x00880000)
#define QUPV3_WRAP3_BASE		UL(0x00c80000)
#define QUPV3_WRAP0_COMMON_BASE		UL(0x009c0000)
#define QUPV3_WRAP1_COMMON_BASE		UL(0x00ac0000)
#define QUPV3_WRAP2_COMMON_BASE		UL(0x008c0000)
#define QUPV3_WRAP3_COMMON_BASE		UL(0x00cc0000)

/*
 * The wrapper mapping covers the span through the common register page, not
 * the populated-SE count. For example, WRAP3 has SE0 only, but its common
 * block is at WRAP3_BASE + 0x40000, so it still needs 0x41000.
 */
#define QUPV3_WRAP0_SIZE		UL(0x41000)
#define QUPV3_WRAP1_SIZE		UL(0x41000)
#define QUPV3_WRAP2_SIZE		UL(0x41000)
#define QUPV3_WRAP3_SIZE		UL(0x41000)

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUPV3_WRAP0_BASE, QUPV3_WRAP0_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUPV3_WRAP1_BASE, QUPV3_WRAP1_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUPV3_WRAP2_BASE, QUPV3_WRAP2_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUPV3_WRAP3_BASE, QUPV3_WRAP3_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUPV3_WRAP0_GSI_BASE, QUPV3_GSI_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUPV3_WRAP1_GSI_BASE, QUPV3_GSI_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUPV3_WRAP2_GSI_BASE, QUPV3_GSI_SIZE);

/* ELF + program headers of the QUP FW image (segments live in DDR) */
static const uint8_t qupv3_elf_hdr[] __aligned(4) = {
	0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x00, 0xa4, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x40, 0x57, 0x87, 0x34, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x34, 0x00, 0x20, 0x00, 0x0a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x74, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x00, 0x00, 0x00, 0x40, 0x57, 0x87,
	0x00, 0x40, 0x57, 0x87, 0x50, 0x4d, 0x00, 0x00,
	0x50, 0x4d, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08,
	0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x50, 0x5d, 0x00, 0x00, 0x00, 0x90, 0x57, 0x87,
	0x00, 0x90, 0x57, 0x87, 0x7b, 0x05, 0x00, 0x00,
	0x7b, 0x05, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08,
	0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0xd0, 0x62, 0x00, 0x00, 0x00, 0xa0, 0x57, 0x87,
	0x00, 0xa0, 0x57, 0x87, 0x55, 0x07, 0x00, 0x00,
	0x55, 0x07, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08,
	0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x28, 0x6a, 0x00, 0x00, 0x00, 0xb0, 0x57, 0x87,
	0x00, 0xb0, 0x57, 0x87, 0x65, 0x11, 0x00, 0x00,
	0x65, 0x11, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08,
	0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x90, 0x7b, 0x00, 0x00, 0x00, 0xd0, 0x57, 0x87,
	0x00, 0xd0, 0x57, 0x87, 0x25, 0x07, 0x00, 0x00,
	0x25, 0x07, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08,
	0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0xb8, 0x82, 0x00, 0x00, 0x00, 0xe0, 0x57, 0x87,
	0x00, 0xe0, 0x57, 0x87, 0x5d, 0x08, 0x00, 0x00,
	0x5d, 0x08, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08,
	0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x18, 0x8b, 0x00, 0x00, 0x00, 0xf0, 0x57, 0x87,
	0x00, 0xf0, 0x57, 0x87, 0xd4, 0x02, 0x00, 0x00,
	0xd4, 0x02, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08,
	0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0xf0, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x58, 0x87,
	0x00, 0x00, 0x58, 0x87, 0x8d, 0x08, 0x00, 0x00,
	0x8d, 0x08, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08,
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xd0, 0x34, 0x00, 0x00,
	0xd0, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	0x00, 0x10, 0x00, 0x00,
};

enum nord_qupv3_se {
	QUPV3_0_SE0, QUPV3_0_SE1, QUPV3_0_SE2, QUPV3_0_SE3,
	QUPV3_0_SE4, QUPV3_0_SE5, QUPV3_0_SE6,
	QUPV3_1_SE0, QUPV3_1_SE1, QUPV3_1_SE2, QUPV3_1_SE3,
	QUPV3_1_SE4, QUPV3_1_SE5, QUPV3_1_SE6,
	QUPV3_2_SE0, QUPV3_2_SE1, QUPV3_2_SE2, QUPV3_2_SE3,
	QUPV3_2_SE4, QUPV3_2_SE5, QUPV3_2_SE6,
	QUPV3_3_SE0,
};

static const struct qupv3_se_hw nord_qupv3_se[] = {
	[QUPV3_0_SE0] = {
		.base = UL(0x00980000),
		.clk_name = "se_gcc_qupv3_wrap0_s0_clk",
	},
	[QUPV3_0_SE1] = {
		.base = UL(0x00984000),
		.clk_name = "se_gcc_qupv3_wrap0_s1_clk",
	},
	[QUPV3_0_SE2] = {
		.base = UL(0x00988000),
		.clk_name = "se_gcc_qupv3_wrap0_s2_clk",
	},
	[QUPV3_0_SE3] = {
		.base = UL(0x0098c000),
		.clk_name = "se_gcc_qupv3_wrap0_s3_clk",
	},
	[QUPV3_0_SE4] = {
		.base = UL(0x00990000),
		.clk_name = "se_gcc_qupv3_wrap0_s4_clk",
	},
	[QUPV3_0_SE5] = {
		.base = UL(0x00994000),
		.clk_name = "se_gcc_qupv3_wrap0_s5_clk",
	},
	[QUPV3_0_SE6] = {
		.base = UL(0x00998000),
		.clk_name = "se_gcc_qupv3_wrap0_s6_clk",
	},
	[QUPV3_1_SE0] = {
		.base = UL(0x00a80000),
		.clk_name = "se_gcc_qupv3_wrap1_s0_clk",
	},
	[QUPV3_1_SE1] = {
		.base = UL(0x00a84000),
		.clk_name = "se_gcc_qupv3_wrap1_s1_clk",
	},
	[QUPV3_1_SE2] = {
		.base = UL(0x00a88000),
		.clk_name = "se_gcc_qupv3_wrap1_s2_clk",
	},
	[QUPV3_1_SE3] = {
		.base = UL(0x00a8c000),
		.clk_name = "se_gcc_qupv3_wrap1_s3_clk",
	},
	[QUPV3_1_SE4] = {
		.base = UL(0x00a90000),
		.clk_name = "se_gcc_qupv3_wrap1_s4_clk",
	},
	[QUPV3_1_SE5] = {
		.base = UL(0x00a94000),
		.clk_name = "se_gcc_qupv3_wrap1_s5_clk",
	},
	[QUPV3_1_SE6] = {
		.base = UL(0x00a98000),
		.clk_name = "se_gcc_qupv3_wrap1_s6_clk",
	},
	[QUPV3_2_SE0] = {
		.base = UL(0x00880000),
		.clk_name = "ne_gcc_qupv3_wrap2_s0_clk",
	},
	[QUPV3_2_SE1] = {
		.base = UL(0x00884000),
		.clk_name = "ne_gcc_qupv3_wrap2_s1_clk",
	},
	[QUPV3_2_SE2] = {
		.base = UL(0x00888000),
		.clk_name = "ne_gcc_qupv3_wrap2_s2_clk",
	},
	[QUPV3_2_SE3] = {
		.base = UL(0x0088c000),
		.clk_name = "ne_gcc_qupv3_wrap2_s3_clk",
	},
	[QUPV3_2_SE4] = {
		.base = UL(0x00890000),
		.clk_name = "ne_gcc_qupv3_wrap2_s4_clk",
	},
	[QUPV3_2_SE5] = {
		.base = UL(0x00894000),
		.clk_name = "ne_gcc_qupv3_wrap2_s5_clk",
	},
	[QUPV3_2_SE6] = {
		.base = UL(0x00898000),
		.clk_name = "ne_gcc_qupv3_wrap2_s6_clk",
	},
	[QUPV3_3_SE0] = {
		.base = UL(0x00c80000),
		.clk_name = "gcc_qupv3_wrap3_s0_clk",
	},
};

static const char * const nord_wrap0_clks[] = {
	"se_gcc_qupv3_wrap0_core_2x_clk", "se_gcc_qupv3_wrap0_core_clk",
	"se_gcc_qupv3_wrap0_m_ahb_clk", "se_gcc_qupv3_wrap0_s_ahb_clk",
};

static const char * const nord_wrap1_clks[] = {
	"se_gcc_qupv3_wrap1_core_2x_clk", "se_gcc_qupv3_wrap1_core_clk",
	"se_gcc_qupv3_wrap1_m_ahb_clk", "se_gcc_qupv3_wrap1_s_ahb_clk",
};

static const char * const nord_wrap2_clks[] = {
	"ne_gcc_qupv3_wrap2_core_2x_clk", "ne_gcc_qupv3_wrap2_core_clk",
	"ne_gcc_qupv3_wrap2_m_ahb_clk", "ne_gcc_qupv3_wrap2_s_ahb_clk",
};

static const char * const nord_wrap3_clks[] = {
	"gcc_qupv3_wrap3_core_2x_clk", "gcc_qupv3_wrap3_core_clk",
	"gcc_qupv3_wrap3_m_clk", "gcc_qupv3_wrap3_s_ahb_clk",
};

#define COMMON(w, gsi, gsi_sz) \
	{ .base = QUPV3_WRAP##w##_COMMON_BASE, \
	   .clk_names = nord_wrap##w##_clks, \
	   .num_clks = ARRAY_SIZE(nord_wrap##w##_clks), \
	   .gsi_base = (gsi), .gsi_size = (gsi_sz) }

/*
 * As in TZ gpi_firmware_load(), GSI FW is loaded on wrappers 0-2 only
 * (MAX_NUM_QUP_BLOCKS - 1), wrapper 3 is left to its owner.
 */
static const struct qupv3_common_hw nord_qupv3_common[] = {
	COMMON(0, QUPV3_WRAP0_GSI_BASE, QUPV3_GSI_SIZE),
	COMMON(1, QUPV3_WRAP1_GSI_BASE, QUPV3_GSI_SIZE),
	COMMON(2, QUPV3_WRAP2_GSI_BASE, QUPV3_GSI_SIZE),
	COMMON(3, 0, 0),
};

#define PERM(se, proto, md, owner, fifo, ld, excl) \
	{ .periph_id = (se), .protocol = QUPV3_PROTOCOL_##proto, \
	   .mode = QUPV3_MODE_##md, .ns_owner = QUPV3_AC_##owner, \
	   .allow_fifo = (fifo), .load = (ld), .mod_excl = (excl) }

/* Mirrors qupv3_perms_default_auto[] of TZ nord QUPAC_Access.c */
static const struct qupv3_se_perms nord_qupv3_perms[] = {
	PERM(QUPV3_0_SE1, I2C, FIFO, HLOS, true, true, false),
	PERM(QUPV3_0_SE4, UART_2W, FIFO, HLOS, true, true, false),
	PERM(QUPV3_1_SE0, I2C, FIFO, HLOS, true, true, false),
	PERM(QUPV3_1_SE2, I2C, FIFO, HLOS, true, true, false),
	PERM(QUPV3_1_SE5, I2C, FIFO, HLOS, true, true, false),
	PERM(QUPV3_2_SE0, SPI, FIFO, HLOS, true, true, false),
	/* Debug UART, FW already loaded by the boot chain */
	PERM(QUPV3_2_SE1, UART_2W, FIFO, HLOS, true, false, false),
	PERM(QUPV3_2_SE2, SPI, FIFO, HLOS, true, true, false),
	PERM(QUPV3_2_SE3, UART_4W, FIFO, HLOS, true, true, false),
	PERM(QUPV3_2_SE4, I2C, FIFO, HLOS, true, true, false),
	PERM(QUPV3_2_SE5, I2C, FIFO, HLOS, true, true, false),
};

static const struct qupv3_fw_platform nord_qupv3_fw = {
	.elf_hdr = qupv3_elf_hdr,
	.elf_hdr_size = sizeof(qupv3_elf_hdr),
	.fw_ddr_base = QUPV3_FW_DDR_BASE,
	.fw_ddr_size = QUPV3_FW_DDR_SIZE,
	.se = nord_qupv3_se,
	.num_se = ARRAY_SIZE(nord_qupv3_se),
	.common = nord_qupv3_common,
	.num_common = ARRAY_SIZE(nord_qupv3_common),
	.perms = nord_qupv3_perms,
	.num_perms = ARRAY_SIZE(nord_qupv3_perms),
};

const struct qupv3_fw_platform *qupv3_fw_get_platform(void)
{
	return &nord_qupv3_fw;
}
