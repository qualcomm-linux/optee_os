/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Minimal Tier 1 host unit-test harness for the QCOM RPMh/CmdDb drivers.
 * No third-party test framework (e.g. cmocka) is available/used here;
 * this is deliberately small: register test functions in a table, run
 * each with a fresh mock environment, and report pass/fail.
 *
 * A failing TEST_ASSERT*() records the failure and longjmp()s back to the
 * runner so the remaining assertions in a broken test are skipped without
 * aborting the whole suite - the next test still runs with a clean mock
 * environment.
 */
#ifndef __HARNESS_H
#define __HARNESS_H

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

struct harness_test {
	const char *name;
	void (*fn)(void);
};

#define TEST(name) static void name(void)
#define TEST_CASE(name) { #name, name }

extern jmp_buf harness_test_jmpbuf;

void harness_fail(const char *file, int line, const char *fmt, ...)
	__attribute__((format(printf, 3, 4)));

#define TEST_ASSERT_MSG(cond, ...)					\
	do {								\
		if (!(cond))						\
			harness_fail(__FILE__, __LINE__, __VA_ARGS__);	\
	} while (0)

#define TEST_ASSERT(cond) \
	TEST_ASSERT_MSG((cond), "assertion failed: %s", #cond)

#define TEST_ASSERT_EQ_U32(actual, expected)				\
	TEST_ASSERT_MSG((uint32_t)(actual) == (uint32_t)(expected),	\
			"%s == %s failed: got 0x%08x, want 0x%08x",	\
			#actual, #expected,				\
			(unsigned int)(actual), (unsigned int)(expected))

#define TEST_ASSERT_EQ_INT(actual, expected)				\
	TEST_ASSERT_MSG((long)(actual) == (long)(expected),		\
			"%s == %s failed: got %ld, want %ld",		\
			#actual, #expected,				\
			(long)(actual), (long)(expected))

/*
 * For invariants inside the mock support code itself (e.g. an access to
 * an unmapped fake register) - a violation here means the test or mock is
 * wrong, not the driver under test, so it aborts immediately rather than
 * being recorded as a test failure.
 */
#define MOCK_ASSERT_MSG(cond, msg)					\
	do {								\
		if (!(cond)) {						\
			fprintf(stderr, "MOCK ERROR %s:%d: %s\n",	\
				__FILE__, __LINE__, msg);		\
			abort();					\
		}							\
	} while (0)

/* Runs each test in @tests, resetting the mock environment before each.
 * Returns the number of failed tests (0 == success), suitable as a
 * process exit code.
 */
int harness_run(const struct harness_test *tests, int count);

#endif /* __HARNESS_H */
