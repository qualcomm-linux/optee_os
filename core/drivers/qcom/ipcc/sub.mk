# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#

# A chipset without an IPCC gets the stub instead, so that a caller of the API
# links either way and every call fails with TEE_ERROR_NOT_SUPPORTED.

ifeq ($(CFG_QCOM_IPCC),y)

srcs-y += ipcc_core.c

# One file per backend, each defining its own ipcc_backend_ops
srcs-y += ipcc_router.c
srcs-y += ipcc_direct.c

# Chipset description: protocols, clients and the backend each one runs
srcs-y += $(PLATFORM_FLAVOR)/ipcc_config.c

global-incdirs-y += .
global-incdirs-y += $(PLATFORM_FLAVOR)

else

srcs-y += ipcc_stub.c

endif
