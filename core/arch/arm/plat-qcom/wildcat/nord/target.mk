# Nord (SA8797P / Oryon) OP-TEE platform.

# Threads are expensive in OP-TEE, so they don't have
# to be same as number of cores.
$(call force,CFG_TEE_CORE_NB_CORE,18)

CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

# QUPv3 serial-engine (bus) clock set-rate/DFS walker, consumed on-demand by a
# future TEE-side SPI/I2C driver. Set-rate votes CX/MX via RPMh, so pull
# cmd_db/RPMh client in whenever the walker is built.
CFG_QCOM_CLK_BSP ?= y
ifeq ($(CFG_QCOM_CLK_BSP),y)
$(call force,CFG_QCOM_CMD_DB,y)
$(call force,CFG_QCOM_RPMH_CLIENT,y)
endif
