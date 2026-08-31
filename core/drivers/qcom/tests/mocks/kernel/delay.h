/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/kernel/delay.h.
 *
 * Backed by a controllable virtual microsecond clock (mock_clock.h) instead
 * of a real hardware timer, so tests can deterministically drive timeout
 * paths (e.g. check_aop_init()'s 100ms AOP boot-cookie poll) without
 * actually sleeping.
 */
#ifndef __KERNEL_DELAY_H
#define __KERNEL_DELAY_H

#include <stdint.h>

#include "mock_clock.h"

static inline uint64_t timeout_init_us(uint32_t us)
{
	return mock_clock_get_us() + us;
}

static inline int timeout_elapsed(uint64_t expire)
{
	return mock_clock_get_us() >= expire;
}

static inline void udelay(uint32_t us)
{
	mock_clock_advance_us(us);
}

static inline void mdelay(uint32_t ms)
{
	mock_clock_advance_us(ms * 1000);
}

#endif /* __KERNEL_DELAY_H */
