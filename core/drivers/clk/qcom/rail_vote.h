/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _RAIL_VOTE_H_
#define _RAIL_VOTE_H_

#include <stdint.h>
#include <tee_api_types.h>

/* RPMh corner levels; not tied to any one rail. */
#define RAIL_VOLTAGE_LEVEL_MIN_SVS	0x30
#define RAIL_VOLTAGE_LEVEL_LOW_SVS	0x40
#define RAIL_VOLTAGE_LEVEL_SVS		0x80

#ifdef CFG_QCOM_RPMH_CLIENT
TEE_Result rail_vote_init(void);
TEE_Result rail_vote(uint16_t old_corner, uint16_t new_corner);
#else
static inline TEE_Result rail_vote_init(void)
{
	return TEE_SUCCESS;
}

static inline TEE_Result rail_vote(uint16_t old_corner __unused,
				   uint16_t new_corner)
{
	return new_corner ? TEE_ERROR_NOT_SUPPORTED : TEE_SUCCESS;
}
#endif

#endif /* _RAIL_VOTE_H_ */
