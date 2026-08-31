/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Controllable virtual microsecond clock for Tier 1 host unit tests. Backs
 * kernel/delay.h's timeout_init_us()/timeout_elapsed()/udelay()/mdelay()
 * and kernel/tee_time.h's tee_time_get_sys_time(), so tests can
 * deterministically drive timeout logic (e.g. the AOP boot-cookie poll,
 * TCS stuck-timeout detection) without real wall-clock waits.
 */
#ifndef __MOCK_CLOCK_H
#define __MOCK_CLOCK_H

#include <stdint.h>

/* Reset the virtual clock to 0; call at the start of every test. */
void mock_clock_reset(void);

uint64_t mock_clock_get_us(void);

/* Advance the virtual clock by @us microseconds. udelay()/mdelay() call
 * this so a driver's busy-wait loop makes forward progress against
 * timeout_elapsed() without an actual sleep.
 */
void mock_clock_advance_us(uint64_t us);

#endif /* __MOCK_CLOCK_H */
