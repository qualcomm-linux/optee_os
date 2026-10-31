# SPDX-License-Identifier: BSD-2-Clause
# Enable the Qualcomm GPCE SHA-1 direct-HWIO driver on Shikra.
CFG_CRYPTO_DRIVER ?= y
CFG_QCOM_GPCE ?= y
CFG_CRYPTO_DRV_HASH ?= y
CFG_QCOM_GPCE_HASH ?= y
