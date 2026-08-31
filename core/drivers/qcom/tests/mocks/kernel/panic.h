/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/kernel/panic.h.
 *
 * The real panic() is __noreturn and halts the CPU; that would abort the
 * whole test binary. Here panic() records the message and longjmp()s back
 * to the point captured by MOCK_PANIC_EXPECT() in harness.h, so a test can
 * assert "this call panics" without losing the process. Calling panic()
 * outside of MOCK_PANIC_EXPECT() is a harness bug and aborts loudly.
 */
#ifndef __KERNEL_PANIC_H
#define __KERNEL_PANIC_H

#include "mock_panic.h"

#define panic(...) mock_panic(__FILE__, __LINE__, "" __VA_ARGS__)

static inline void cpu_idle(void) {}

#endif /* __KERNEL_PANIC_H */
