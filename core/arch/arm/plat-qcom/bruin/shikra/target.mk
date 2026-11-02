CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

ifneq ($(CFG_INSECURE),y)
CFG_QCOM_QFPROM ?= y
endif

ifeq ($(CFG_QCOM_QFPROM_FUSEPROV),y)
$(call force,CFG_QCOM_QFPROM,y)
endif

# Enable the Qualcomm GPCE SHA-1 direct-HWIO driver.
CFG_QCOM_GPCE ?= y
CFG_QCOM_GPCE_HASH ?= y
