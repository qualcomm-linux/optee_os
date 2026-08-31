/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mock_panic.h"

jmp_buf mock_panic_jmpbuf;
int mock_panic_armed;
int mock_panic_hit;
char mock_panic_msg[256];

void mock_panic(const char *file, int line, const char *msg)
{
	snprintf(mock_panic_msg, sizeof(mock_panic_msg), "%s:%d: %s",
		 file ? file : "?", line, msg ? msg : "");

	if (!mock_panic_armed) {
		fprintf(stderr,
			"UNEXPECTED PANIC (no MOCK_PANIC_EXPECT active): %s\n",
			mock_panic_msg);
		abort();
	}

	mock_panic_hit = 1;
	longjmp(mock_panic_jmpbuf, 1);
}
