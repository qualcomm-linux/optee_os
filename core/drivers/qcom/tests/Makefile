# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# Tier 1 host unit tests for the QCOM RPMh/CmdDb OP-TEE drivers.
#
# Builds the *real, unmodified* driver .c files with plain host gcc against
# a mix of: (a) real, host-portable OP-TEE headers (util.h, compiler.h,
# types_ext.h, tee_api_types.h, sys/queue.h, string_ext.h) and (b) mocked
# OP-TEE-runtime headers under mocks/ (io.h, kernel/*.h, mm/*.h,
# platform_config.h, initcall.h, trace.h, arm.h) that stand in for the real
# hardware/runtime without touching a single line of driver source.
#
# The RPMh driver's DRV configuration (rpmh_drv_config.c), register layout
# (rpmh_hwio.h) and TCS layout (rpmh_target_config.h) are per-target, living
# under $(DRV_RPMH)/$(FLAVOR)/ - the same split core/drivers/qcom/rpmh/sub.mk
# uses for the real build via $(PLATFORM_FLAVOR). FLAVOR selects which
# target's driver config this test binary is built and run against.
#
# Run a single target:      make -C core/drivers/qcom/tests run FLAVOR=kodiak
# Run all targets:          make -C core/drivers/qcom/tests run-all
# Or via the per-target wrapper dirs: make -C core/drivers/qcom/tests/kodiak run

FLAVOR		?= kodiak
SUPPORTED_FLAVORS := kodiak lemans nord

ifeq ($(filter $(FLAVOR),$(SUPPORTED_FLAVORS)),)
$(error Unsupported FLAVOR '$(FLAVOR)' - expected one of: $(SUPPORTED_FLAVORS))
endif

CC		:= gcc
CFLAGS		:= -std=gnu11 -g -O0 -Wall -Wextra \
		   -Wno-unused-parameter \
		   -DTRACE_LEVEL=4

ROOT		:= ../../../..
DRV_RPMH	:= $(ROOT)/core/drivers/qcom/rpmh
DRV_CMDDB	:= $(ROOT)/core/drivers/qcom/cmd_db
FLAVOR_DIR	:= $(DRV_RPMH)/$(FLAVOR)

# vaddr_t/paddr_t (types_ext.h) are normally pulled in transitively via
# lib/libutils/isoc/include/malloc.h. That header (and setjmp.h/assert.h in
# the same directory) hard-codes ARM32/ARM64/RV64 layouts that don't exist
# on a host build, so isoc/include is deliberately kept off the include
# path below and types_ext.h is force-included into every translation
# unit instead - the same pattern core/core.mk uses to force-include
# conf.h into the real build.
CFLAGS		+= -include $(ROOT)/lib/libutils/ext/include/types_ext.h

# Order matters: mocks first so they shadow same-named real headers
# (io.h, kernel/mutex.h, kernel/delay.h, kernel/panic.h, kernel/tee_time.h,
# mm/core_mmu.h, mm/core_memprot.h, initcall.h, trace.h, arm.h,
# platform_config.h, setjmp.h, assert.h), then real reusable OP-TEE headers,
# then the driver source directories themselves (for their own
# local/relative includes) - FLAVOR_DIR before DRV_RPMH so a per-target
# rpmh_hwio.h/rpmh_target_config.h/rpmh_drv_config.h shadows any same-named
# common header.
INCLUDES	:= \
	-I. \
	-Imocks \
	-I$(ROOT)/lib/libutils/ext/include \
	-I$(ROOT)/lib/libutee/include \
	-I$(ROOT)/core/include \
	-I$(FLAVOR_DIR) \
	-I$(DRV_RPMH) \
	-I$(DRV_CMDDB)

MOCK_SRCS := \
	mocks/mock_regs.c \
	mocks/mock_clock.c \
	mocks/mock_panic.c \
	mocks/mock_memprot.c

FIXTURE_SRCS := \
	cmd_db_fixture.c

DRV_SRCS := \
	$(DRV_RPMH)/rpmh_hal.c \
	$(DRV_RPMH)/rpmh_tcs.c \
	$(DRV_RPMH)/rpmh_resource_commands.c \
	$(DRV_RPMH)/rpmh_client.c \
	$(FLAVOR_DIR)/rpmh_drv_config.c \
	$(DRV_CMDDB)/cmd_db.c \
	$(ROOT)/lib/libutils/ext/strlcpy.c

HARNESS_SRCS := harness.c

TEST_SRCS := \
	test_rpmh_hal.c \
	test_rpmh_tcs.c \
	test_rpmh_resource_commands.c \
	test_rpmh_client.c \
	test_cmd_db.c

BUILD_DIR := build/$(FLAVOR)

.PHONY: all run run-all clean

all: $(BUILD_DIR)/run_tests

run: $(BUILD_DIR)/run_tests
	@echo "=== FLAVOR=$(FLAVOR) ==="
	$(BUILD_DIR)/run_tests

run-all:
	@for f in $(SUPPORTED_FLAVORS); do \
		$(MAKE) --no-print-directory run FLAVOR=$$f || exit 1; \
	done

$(BUILD_DIR)/run_tests: $(TEST_SRCS) $(HARNESS_SRCS) $(MOCK_SRCS) $(FIXTURE_SRCS) $(DRV_SRCS) main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ \
		main.c $(TEST_SRCS) $(HARNESS_SRCS) $(MOCK_SRCS) $(FIXTURE_SRCS) $(DRV_SRCS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf build
