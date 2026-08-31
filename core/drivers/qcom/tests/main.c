/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Entry point for the Tier 1 RPMh/CmdDb host unit test suite. Aggregates
 * every per-module test array and runs them all through harness_run().
 */
#include <stdio.h>

#include "harness.h"

extern const struct harness_test rpmh_hal_tests[];
extern const int rpmh_hal_tests_count;

extern const struct harness_test rpmh_tcs_tests[];
extern const int rpmh_tcs_tests_count;

extern const struct harness_test rpmh_resource_commands_tests[];
extern const int rpmh_resource_commands_tests_count;

extern const struct harness_test rpmh_client_tests[];
extern const int rpmh_client_tests_count;

extern const struct harness_test cmd_db_tests[];
extern const int cmd_db_tests_count;

int main(void)
{
	int failures = 0;

	printf("=== rpmh_hal ===\n");
	failures += harness_run(rpmh_hal_tests, rpmh_hal_tests_count);

	printf("\n=== rpmh_tcs ===\n");
	failures += harness_run(rpmh_tcs_tests, rpmh_tcs_tests_count);

	printf("\n=== rpmh_resource_commands ===\n");
	failures += harness_run(rpmh_resource_commands_tests,
				rpmh_resource_commands_tests_count);

	printf("\n=== rpmh_client ===\n");
	failures += harness_run(rpmh_client_tests, rpmh_client_tests_count);

	printf("\n=== cmd_db ===\n");
	failures += harness_run(cmd_db_tests, cmd_db_tests_count);

	if (failures) {
		printf("\n%d test(s) FAILED\n", failures);
		return 1;
	}

	printf("\nALL TESTS PASSED\n");
	return 0;
}
