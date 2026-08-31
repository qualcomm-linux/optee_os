# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# Tier 1 host unit tests for RPMh/CmdDb, built against nord's driver
# config (core/drivers/qcom/rpmh/nord/). Thin wrapper around the shared
# test harness one level up - see ../Makefile for the actual build rules.
#
# NOTE: nord's rpmh_hwio.h/rpmh_target_config.h/rpmh_drv_config.c values
# are unverified placeholders pending AoP confirmation (see
# core/drivers/qcom/rpmh/nord/rpmh_target_config.h and
# core/arch/arm/plat-qcom/wildcat/nord/target_config.h), and
# CFG_QCOM_RPMH_CLIENT/CFG_QCOM_CMD_DB are not currently enabled for
# wildcat/nord in the real build. This suite still exercises the driver
# logic (TCS allocation, MSGID encoding, vote reconciliation, etc.) against
# nord's current values so regressions are caught the moment they land,
# but a pass here does not confirm nord's real register addresses.
#
# Run with: make -C core/drivers/qcom/tests/nord run

.PHONY: all run clean

all run clean:
	$(MAKE) -C .. $@ FLAVOR=nord
