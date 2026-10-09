CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_DIAG_LOG ?= $(CFG_TEE_CORE_DEBUG)

ifneq ($(CFG_INSECURE),y)
CFG_QCOM_QFPROM_FUSEPROV ?= y
endif

# Fuse writes remain disabled unless explicitly enabled for a provisioning build.
CFG_QFPROM_PROGRAMMING ?= n

ifeq ($(CFG_QCOM_QFPROM_FUSEPROV),y)
$(call force,CFG_QCOM_QFPROM,y)
endif

ifeq ($(CFG_QFPROM_PROGRAMMING),y)
$(call force,CFG_QCOM_QFPROM_FUSEPROV,y)
$(call force,CFG_QCOM_QFPROM,y)
endif
