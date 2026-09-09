/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __RPMH_TARGET_CONFIG_H__
#define __RPMH_TARGET_CONFIG_H__

#include <platform_config.h>

enum rpmh_tcs_config {
	RPMH_TCS_AMC   = 0,  /* Active TCS start */
	RPMH_TCS_SLEEP = 2,  /* Active TCS end, Sleep TCS start */
	RPMH_TCS_WAKE  = 3,  /* Sleep TCS end, Wake TCS start */
	RPMH_TCS_MAX   = 4   /* Wake TCS end, Max TCS count */
};

#define RPMH_MAX_CMDS_PER_TCS          16

#endif /* __RPMH_TARGET_CONFIG_H__ */
