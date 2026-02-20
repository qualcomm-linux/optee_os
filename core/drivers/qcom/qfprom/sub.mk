# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#

srcs-y += qfprom_core.c
srcs-y += qfprom_hal.c
srcs-y += qfprom_target.c

$(eval $(call cfg-depends-all,CFG_QCOM_QFPROM_FUSEPROV,CFG_QCOM_QFPROM))
subdirs-$(CFG_QCOM_QFPROM_FUSEPROV) += fuseprov

srcs-y += $(PLATFORM_FLAVOR)/qfprom_fuse_region.c

global-incdirs-y += .
global-incdirs-y += $(PLATFORM_FLAVOR)
