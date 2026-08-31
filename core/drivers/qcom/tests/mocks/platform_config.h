/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/arch/arm/plat-qcom/platform_config.h.
 *
 * The real per-SoC target_config.h files (kodiak/lemans/nord) pull in the
 * full platform build. Tests use these fixed, test-only addresses instead
 * so driver init paths (rpmh_client_init(), cmd_db_init()) have concrete
 * values to register/allocate/map against.
 *
 * UL() normally comes from lib/libutils/isoc/include/stdint.h, which is
 * off this build's include path (see Makefile) because it hard-codes
 * ARM32/ARM64/RV64 layouts elsewhere in the same directory. Define it
 * directly here instead.
 */
#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <util.h>

#ifndef UL
#define UL(v)	(v ## UL)
#endif

#define AOP_MSG_RAM_BASE	UL(0x0C300000)
#define AOP_MSG_RAM_SIZE	UL(0x00100000)

#define RPMH_BASE_ADDR		UL(0x18200000)
#define RPMH_RSC_SIZE		UL(0x00040000)

#define AOP_CMD_DB_BASE		UL(0x80860000)
#define AOP_CMD_DB_SIZE		UL(0x00020000)

#endif /* PLATFORM_CONFIG_H */
