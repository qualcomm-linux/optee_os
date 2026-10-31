# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2026, Qualcomm Technologies, Inc.

# Qualcomm GPCE SHA-1 driver using direct HWIO transfers.
global-incdirs-y += .
global-incdirs-y += ../../crypto_api/include

srcs-$(CFG_QCOM_GPCE) += qce_hash.c
srcs-$(CFG_QCOM_GPCE) += sha1_hwio.c
srcs-$(CFG_QCOM_GPCE) += sha_hw_cmn.c
srcs-$(CFG_QCOM_GPCE) += sha_hwio_cmn.c
srcs-$(CFG_QCOM_GPCE) += ce_hal.c
srcs-$(CFG_QCOM_GPCE) += ce_hwio.c
srcs-$(CFG_QCOM_GPCE) += util_blkbuf.c
