# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#

# IPCC (Inter-Processor Communication Controller) driver
#
# A chipset without an IPCC gets the stub instead, so that a caller of the API
# links either way and every call fails with TEE_ERROR_NOT_SUPPORTED.

ifeq ($(CFG_QCOM_IPCC),y)

# Core API: attach, detach, register, deregister, trigger
srcs-y += ipcc_core.c

# Backend operations: assigns function pointers to backend implementations
srcs-y += ipcc_backends.c

# Router mode: shared Rx interrupt per protocol, Tx through our client page
srcs-y += ipcc_router.c

# Direct mode: per-signal interrupt line and per-client trigger register
srcs-y += ipcc_direct.c

# Chipset description, evaluated at runtime
srcs-y += $(PLATFORM_FLAVOR)/ipcc_config.c

global-incdirs-y += .
global-incdirs-y += $(PLATFORM_FLAVOR)

else

srcs-y += ipcc_stub.c

endif

