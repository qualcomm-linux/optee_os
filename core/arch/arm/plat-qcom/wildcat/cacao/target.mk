# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

# Qualcomm Cacao platform configuration.

$(call force,CFG_TEE_CORE_NB_CORE,8)

CFG_TZDRAM_START ?= 0x80fcd000
CFG_TEE_RAM_VA_SIZE ?= 0x147000
CFG_TA_RAM_VA_SIZE ?= 0x300000

CFG_DRIVERS_CLK ?= n
CFG_DRIVERS_QCOM_CLK ?= n

CFG_QCOM_QFPROM_FUSEPROV ?= n
CFG_QCOM_QFPROM ?= n
CFG_QCOM_CMD_DB ?= n
CFG_QCOM_RPMH_CLIENT ?= n

CFG_QCOM_PAS_PTA ?= n

ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_RESERVED_VASPACE_SIZE ?= (60 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif
