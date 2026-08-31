/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/kernel/tee_time.h.
 *
 * Only tee_time_get_sys_time() is used by the RPMh/CmdDb drivers
 * (rpmh_tcs.c's get_timestamp()); it is backed by the same virtual clock
 * used by the mocked kernel/delay.h so tests can control elapsed time.
 */
#ifndef __KERNEL_TEE_TIME_H
#define __KERNEL_TEE_TIME_H

#include <tee_api_types.h>

#include "mock_clock.h"

static inline TEE_Result tee_time_get_sys_time(TEE_Time *time)
{
	uint64_t us = mock_clock_get_us();

	time->seconds = (uint32_t)(us / 1000000);
	time->millis = (uint32_t)((us / 1000) % 1000);

	return TEE_SUCCESS;
}

#endif /* __KERNEL_TEE_TIME_H */
