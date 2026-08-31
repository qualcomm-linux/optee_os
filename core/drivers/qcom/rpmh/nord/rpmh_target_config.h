/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __RPMH_TARGET_CONFIG_H__
#define __RPMH_TARGET_CONFIG_H__

#include <platform_config.h>

/*
 * TCS (TCS Command Set) indices for secure world:
 * - AMC (Active Mode Controller): Triggers immediately
 * - SLEEP: Triggers on entering suspend
 * - WAKE: Triggers on the next wake-up
 *
 * TODO: pending AoP confirmation for Nord IQ10 (see to_ask.md #2.3 and
 * policy_decisions.md #2). Values below are copied from hoya and are not
 * verified for Nord. This file is not currently compiled - CFG_QCOM_RPMH_CLIENT
 * is not set for wildcat/nord.
 */
enum rpmh_tcs_config {
	RPMH_TCS_AMC   = 0,  /* Active TCS start */
	RPMH_TCS_SLEEP = 2,  /* Active TCS end, Sleep TCS start */
	RPMH_TCS_WAKE  = 3,  /* Sleep TCS end, Wake TCS start */
	RPMH_TCS_MAX   = 4   /* Wake TCS end, Max TCS count */
};

#define RPMH_MAX_CMDS_PER_TCS          16

#endif /* __RPMH_TARGET_CONFIG_H__ */
