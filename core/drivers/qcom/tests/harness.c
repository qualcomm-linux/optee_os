/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#include <stdarg.h>
#include <stdio.h>

#include "harness.h"
#include "mocks/mock_clock.h"
#include "mocks/mock_panic.h"
#include "mocks/mock_regs.h"

jmp_buf harness_test_jmpbuf;

static int current_failed;

void harness_fail(const char *file, int line, const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "  FAIL %s:%d: ", file, line);

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");

	current_failed = 1;

	longjmp(harness_test_jmpbuf, 1);
}

int harness_run(const struct harness_test *tests, int count)
{
	int failures = 0;
	int i = 0;

	for (i = 0; i < count; i++) {
		mock_regs_reset();
		mock_clock_reset();
		MOCK_PANIC_DISARM();

		current_failed = 0;

		printf("RUN  %s\n", tests[i].name);

		if (setjmp(harness_test_jmpbuf) == 0)
			tests[i].fn();

		if (current_failed) {
			printf("FAIL %s\n", tests[i].name);
			failures++;
		} else {
			printf("PASS %s\n", tests[i].name);
		}
	}

	printf("\n%d/%d tests passed\n", count - failures, count);

	return failures;
}
