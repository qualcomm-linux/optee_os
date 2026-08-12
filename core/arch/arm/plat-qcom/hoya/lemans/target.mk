CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_DIAG_LOG ?= $(CFG_TEE_CORE_DEBUG)

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
CFG_RESERVED_VASPACE_SIZE ?= (256 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe

CFG_QCOM_PAS_AUTH ?= y

CFG_PAS_MD_SLOTS ?= 8
endif
CFG_QCOM_HWKM ?= y

# QUPv3 serial-engine (bus) clock set-rate/DFS walker, consumed on-demand by a
# future TEE-side SPI/I2C driver. Set-rate votes CX/MX via RPMh, so pull
# cmd_db/RPMh client in whenever the walker is built.
CFG_QCOM_CLK_BSP ?= y
ifeq ($(CFG_QCOM_CLK_BSP),y)
$(call force,CFG_QCOM_CMD_DB,y)
$(call force,CFG_QCOM_RPMH_CLIENT,y)
endif
