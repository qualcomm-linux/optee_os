/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of lib/libutils/isoc/include/setjmp.h.
 *
 * The real header hand-rolls jmp_buf per architecture (ARM32/ARM64/RV64)
 * for OP-TEE's own longjmp()/setjmp() implementation, none of which apply
 * to a host build. Just forward to the host libc's <setjmp.h>.
 */
#ifndef __MOCK_SETJMP_H
#define __MOCK_SETJMP_H

#include_next <setjmp.h>

#endif /* __MOCK_SETJMP_H */
