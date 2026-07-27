# Qualcomm platform support

PLATFORM_FLAVOR ?= kodiak

# The default value CFG_INSECURE ?= y is assigned in mk/config.mk.
# But mk/config.mk is included after $(platform-dir)/conf.mk from
# core/core.mk.
# Since we are making decision based on CFG_INSECURE in this file
# so we need to set it early here also.
CFG_INSECURE ?= y

$(call force,CFG_SECURE_TIME_SOURCE_CNTPCT,y)
$(call force,CFG_ARM64_core,y)
$(call force,CFG_WITH_ARM_TRUSTED_FW,y)
$(call force,CFG_CORE_LARGE_PHYS_ADDR,y)
$(call force,CFG_CORE_RESERVED_SHM,n)
$(call force,CFG_CRYPTO_WITH_CE,y)
$(call force,CFG_HW_UNIQUE_KEY_LENGTH,32)

ta-targets = ta_arm64
supported-ta-targets ?= ta_arm64

# Architecture family mapping
HOYA_ARCH_CHIPSETS := kodiak lemans
BOBCAT_ARCH_CHIPSETS := ipq96xx ipq52xx
WILDCAT_ARCH_CHIPSETS := nord rolas

ifneq (,$(filter $(PLATFORM_FLAVOR),$(HOYA_ARCH_CHIPSETS)))
QCOM_ARCH_FAMILY := hoya
else ifneq (,$(filter $(PLATFORM_FLAVOR),$(BOBCAT_ARCH_CHIPSETS)))
QCOM_ARCH_FAMILY := bobcat
else ifneq (,$(filter $(PLATFORM_FLAVOR),$(WILDCAT_ARCH_CHIPSETS)))
QCOM_ARCH_FAMILY := wildcat
else
$(error Unsupported PLATFORM_FLAVOR: $(PLATFORM_FLAVOR))
endif

# Include arch/target specific configurations if present
-include core/arch/arm/plat-qcom/$(QCOM_ARCH_FAMILY)/qcom-arch.mk
-include core/arch/arm/plat-qcom/$(QCOM_ARCH_FAMILY)/$(PLATFORM_FLAVOR)/target.mk
