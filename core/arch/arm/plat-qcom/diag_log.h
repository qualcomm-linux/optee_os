/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __DIAG_LOG_H
#define __DIAG_LOG_H

#include <compiler.h>
#include <config.h>
#include <platform_config.h>
#include <util.h>

#define IMEM_DIAG_OFFSET	UL(0x720)
#define DIAG_LOG_START_INFO	(IMEM_BASE + IMEM_DIAG_OFFSET)
#define TCSR_BOOT_MISC_DETECT	UL(0x1FD3000)
#define DIAG_SIZE		UL(0x3000)
#define DIAG_BASE		(IMEM_BASE + IMEM_SIZE - DIAG_SIZE)

void qcom_diag_log_init(void);
void qcom_diag_log_puts(const char *str);

#endif /* __DIAG_LOG_H */
