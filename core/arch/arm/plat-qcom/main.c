// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025, Linaro Limited
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <console.h>
#ifdef CFG_GIC
#include <drivers/gic.h>
#endif
#ifdef CFG_QCOM_GENI_UART
#include <drivers/qcom_geni_uart.h>
#endif
#ifdef CFG_PL011
#include <drivers/pl011.h>
#endif
#include <kernel/boot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>

#include "diag_log.h"

/*
 * Register the physical memory area for peripherals etc. Here we are
 * registering the UART console.
 */
#ifdef CFG_QCOM_GENI_UART
register_phys_mem_pgdir(MEM_AREA_IO_NSEC, GENI_UART_REG_BASE,
			GENI_UART_REG_SIZE);
#endif
#ifdef CFG_PL011
register_phys_mem_pgdir(MEM_AREA_IO_NSEC, CONSOLE_UART_BASE, PL011_UART_SIZE);
#endif

#ifdef CFG_GIC
register_phys_mem_pgdir(MEM_AREA_IO_SEC, GICD_BASE, GIC_DIST_REG_SIZE);
#ifdef _CFG_ARM_V3_OR_V4
register_phys_mem_pgdir(MEM_AREA_IO_SEC, GICR_BASE,
			GIC_REDIST_REG_SIZE * CFG_TEE_CORE_NB_CORE);
#else
register_phys_mem_pgdir(MEM_AREA_IO_SEC, GICC_BASE, GIC_CPU_REG_SIZE);
#endif
#endif /* CFG_GIC */

register_ddr(DRAM0_BASE, DRAM0_SIZE);
#ifdef DRAM1_BASE
register_ddr(DRAM1_BASE, DRAM1_SIZE);
#endif

#ifdef CFG_QCOM_GENI_UART
static struct qcom_geni_uart_data console_data;
#endif
#ifdef CFG_PL011
static struct pl011_data console_data;
#endif

void plat_trace_ext_puts(const char *str)
{
	qcom_diag_log_puts(str);
}

void plat_trace_init(void)
{
	qcom_diag_log_init();
}

void plat_console_init(void)
{
#ifdef CFG_QCOM_GENI_UART
	qcom_geni_uart_init(&console_data, GENI_UART_REG_BASE);
	register_serial_console(&console_data.chip);
#endif
#ifdef CFG_PL011
	pl011_init(&console_data, CONSOLE_UART_BASE, 0, 0);
	register_serial_console(&console_data.chip);
#endif
}

static TEE_Result platform_banner(void)
{
	IMSG("Platform Qualcomm: Flavor %s", TO_STR(PLATFORM_FLAVOR));

	return TEE_SUCCESS;
}

boot_final(platform_banner);

#ifdef CFG_GIC
void boot_primary_init_intc(void)
{
#ifdef _CFG_ARM_V3_OR_V4
	gic_init_v3(0, GICD_BASE, GICR_BASE);
#else
	gic_init(GICC_BASE, GICD_BASE);
#endif
}

void boot_secondary_init_intc(void)
{
	gic_init_per_cpu();
}
#endif /* CFG_GIC */
