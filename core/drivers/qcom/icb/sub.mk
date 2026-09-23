# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#

global-incdirs-y += include

srcs-$(CFG_QCOM_ICB) += icbuarb.c
srcs-$(CFG_QCOM_ICB) += icbcfg_query.c
