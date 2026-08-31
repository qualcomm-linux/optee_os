# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# Tier 1 host unit tests for RPMh/CmdDb, built against kodiak's driver
# config (core/drivers/qcom/rpmh/kodiak/). Thin wrapper around the shared
# test harness one level up - see ../Makefile for the actual build rules.
#
# Run with: make -C core/drivers/qcom/tests/kodiak run

.PHONY: all run clean

all run clean:
	$(MAKE) -C .. $@ FLAVOR=kodiak
