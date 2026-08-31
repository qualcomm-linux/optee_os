/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of lib/libutils/ext/include/trace.h.
 *
 * Trace output isn't useful for automated assertions and the real
 * trace_ext_puts()/trace_printf() backends only exist inside a running
 * OP-TEE core. Route everything to plain printf so a developer running the
 * suite by hand can still see driver-side error messages; tests do not
 * assert on this output.
 */
#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>

#define MSG(...)	printf(__VA_ARGS__), printf("\n")
#define EMSG(...)	printf("EMSG: " __VA_ARGS__), printf("\n")
#define IMSG(...)	printf("IMSG: " __VA_ARGS__), printf("\n")
#define DMSG(...)	printf("DMSG: " __VA_ARGS__), printf("\n")
#define FMSG(...)	do {} while (0)

#endif /* TRACE_H */
