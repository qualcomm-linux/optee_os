# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#

# A chipset without an IPCC does not descend into this subdir at all; see
# core/drivers/qcom/sub.mk.

srcs-y += ipcc_core.c

# One file per backend, each defining its own ipcc_backend_ops
srcs-y += ipcc_router.c
srcs-y += ipcc_direct.c

# Chipset description: protocols, clients and the backend each one runs
srcs-y += $(PLATFORM_FLAVOR)/ipcc_config.c

global-incdirs-y += .
global-incdirs-y += $(PLATFORM_FLAVOR)
