CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_PAS_PTA ?= y

ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_RESERVED_VASPACE_SIZE ?= (60 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif

CFG_QCOM_CLK_CFG ?= y
CFG_QCOM_RPM_CLIENT ?= y
ifeq ($(CFG_QCOM_RPM_CLIENT),y)
CFG_QCOM_RPM_TRANSPORT_GLINK ?= y
endif

# ICB RPM depends on both CLK_CFG and RPM_CLIENT
ifeq ($(CFG_QCOM_CLK_CFG),y)
ifeq ($(CFG_QCOM_RPM_CLIENT),y)
CFG_QCOM_ICB_RPM ?= y
endif
endif

ifeq ($(CFG_QCOM_ICB_RPM),y)
# u_data.c defines 21 routes.
CFG_QCOM_ICB_MAX_CLIENTS = 21
endif
