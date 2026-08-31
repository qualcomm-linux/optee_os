/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#include "mock_clock.h"

static uint64_t clock_us;

void mock_clock_reset(void)
{
	clock_us = 0;
}

uint64_t mock_clock_get_us(void)
{
	return clock_us;
}

void mock_clock_advance_us(uint64_t us)
{
	clock_us += us;
}
