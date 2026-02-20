// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <stddef.h>

#include "rpmh_drv_config.h"
#include "rpmh_target_config.h"

static const struct tcs_config tcs_config_tz = {
	.amcs = RPMH_TCS_ACTIVE,
	.sleep_start = RPMH_TCS_ACTIVE,
	.wake_start = RPMH_TCS_ACTIVE + RPMH_TCS_SLEEP,
};

static const struct drv_config_data optee_config_data = {
	.drvs_count = 1,
	.init_clks_count = 0,
	.init_clks = NULL,
	.sleep_clks_count = 0,
	.sleep_clks = NULL,

	.drvs = (struct drv_config[]) {
		{
			.drv_id = RSC_DRV_TZ,
			.hw_drv = RSC_DRV_TZ,
			.wake_set_latency = 0x7080,
			.tcs_offset = 0,
			.tcs = RPMH_TCS_ACTIVE + RPMH_TCS_SLEEP + RPMH_TCS_WAKE,
			.cmds = RPMH_MAX_CMDS_PER_TCS,
			.modes_count = 1,
			.modes = (const struct tcs_config *[]) {
				&tcs_config_tz,
			}
		}
	}
};

const struct drv_config_data *const g_drv_config_data = &optee_config_data;
