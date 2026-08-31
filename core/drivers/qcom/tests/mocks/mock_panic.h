/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Backing implementation for the mocked kernel/panic.h. The real panic()
 * is __noreturn and halts the CPU, which would kill the whole test
 * process. Instead, a test that expects a panic wraps the call in
 * MOCK_PANIC_EXPECT() (harness.h), which setjmp()s a recovery point; when
 * mock_panic() runs it records the message and longjmp()s back there. If
 * panic() fires with no recovery point set up, that's a real bug (a driver
 * path panicked when the test didn't expect it) and the process aborts
 * with a clear message instead of silently continuing past code that
 * assumed panic() never returns.
 */
#ifndef __MOCK_PANIC_H
#define __MOCK_PANIC_H

#include <setjmp.h>

extern jmp_buf mock_panic_jmpbuf;
extern int mock_panic_armed;
extern int mock_panic_hit;
extern char mock_panic_msg[256];

void mock_panic(const char *file, int line, const char *msg);

#define MOCK_PANIC_ARM()	(mock_panic_armed = 1, mock_panic_hit = 0)
#define MOCK_PANIC_DISARM()	(mock_panic_armed = 0)

/*
 * Run @stmt, expecting it to reach a panic() before falling off the end.
 * Sets mock_panic_hit=1 and mock_panic_msg if it did. Safe to use even
 * when @stmt calls a function whose return value is relied upon by the
 * compiler to be reached only on the non-panic path - execution never
 * returns from mock_panic() on the panic path, matching real panic()'s
 * __noreturn contract from the caller's point of view.
 */
#define MOCK_PANIC_EXPECT(stmt)					\
	do {							\
		MOCK_PANIC_ARM();				\
		if (setjmp(mock_panic_jmpbuf) == 0) {		\
			stmt;					\
		}						\
		MOCK_PANIC_DISARM();				\
	} while (0)

#endif /* __MOCK_PANIC_H */
