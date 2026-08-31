/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of lib/libutils/isoc/include/assert.h.
 *
 * The real header's assert()/runtime_assert() route through OP-TEE's own
 * _assert_break()/_assert_log(), which aren't available/needed on host.
 * Just forward to the host libc's <assert.h>.
 *
 * The real header also transitively pulls in <trace.h> (for EMSG()/etc.);
 * several driver .c files include <assert.h> without including <trace.h>
 * themselves, relying on that transitive include. Keep it here too.
 */
#ifndef __MOCK_ASSERT_H
#define __MOCK_ASSERT_H

#include <trace.h>

#include_next <assert.h>

#endif /* __MOCK_ASSERT_H */
