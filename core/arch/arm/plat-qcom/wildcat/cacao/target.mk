# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

# Qualcomm Cacao platform configuration.

$(call force,CFG_TEE_CORE_NB_CORE,8)

# DARE-TZ secure memory regions. DARE is an in-line memory encryption
# IP on Wildcat; it is set up by the TME root-of-trust before OP-TEE
# runs, so no specific OP-TEE driver is needed.
CFG_TZDRAM_START ?= 0x80fcd000
CFG_TEE_RAM_VA_SIZE ?= 0x147000
CFG_TA_RAM_VA_SIZE ?= 0x300000

ifneq ($(CFG_INSECURE),y)
CFG_QCOM_QFPROM_FUSEPROV ?= y
endif

ifeq ($(CFG_QCOM_QFPROM_FUSEPROV),y)
$(call force,CFG_QCOM_CMD_DB,y)
$(call force,CFG_QCOM_RPMH_CLIENT,y)
$(call force,CFG_QCOM_QFPROM,y)
endif

CFG_QCOM_PAS_PTA ?= y

ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_PAS_MD_SLOTS ?= 8
CFG_RESERVED_VASPACE_SIZE ?= (64 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif
