CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_QFPROM_FUSEPROV ?= $(if $(filter y,$(CFG_INSECURE)),n,y)

_qcom_fuseprov_deps = $(if $(filter y,$(CFG_QCOM_QFPROM_FUSEPROV)),y,n)
CFG_QCOM_CMD_DB ?= $(_qcom_fuseprov_deps)
CFG_QCOM_RPMH_CLIENT ?= $(_qcom_fuseprov_deps)
CFG_QCOM_QFPROM ?= $(_qcom_fuseprov_deps)

CFG_QCOM_PAS_PTA ?= y

ifeq ($(CFG_QCOM_PAS_PTA),y)
# PAS subsystems map their controller windows at runtime from the reserved VA
# pool (never released). The six DSP windows total ~146.5 MB; the 60 MB default
# fits only one, so reserve 256 MB with headroom.
CFG_RESERVED_VASPACE_SIZE ?= (256 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif
