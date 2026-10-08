# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

# Qualcomm Cacao platform configuration.

$(call force,CFG_TEE_CORE_NB_CORE,3)
CFG_QCOM_UART_CONSOLE := n

# DARE-TZ secure memory regions. DARE is an in-line memory encryption
# IP on Wildcat; it is set up by the TME root-of-trust before OP-TEE
# runs, so no specific OP-TEE driver is needed.
CFG_TZDRAM_START ?= 0x80fcd000
CFG_TEE_RAM_VA_SIZE ?= 0x147000
CFG_TA_RAM_VA_SIZE ?= 0x300000

CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

# QUPv3 serial-engine (bus) clock set-rate/DFS walker, consumed on-demand by a
# future TEE-side SPI/I2C driver. Set-rate votes CX/MX via RPMh, so pull
# cmd_db/RPMh client in whenever the walker is built.
CFG_QCOM_CLK_CFG ?= y
ifeq ($(CFG_QCOM_CLK_CFG),y)
$(call force,CFG_QCOM_CMD_DB,y)
$(call force,CFG_QCOM_RPMH_CLIENT,y)
endif
