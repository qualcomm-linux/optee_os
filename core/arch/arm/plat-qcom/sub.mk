global-incdirs-y += .
global-incdirs-y += $(QCOM_ARCH_FAMILY)
global-incdirs-y += $(QCOM_ARCH_FAMILY)/$(PLATFORM_FLAVOR)
srcs-y += main.c
srcs-y += diag_log.c

$(eval $(call cfg-depends-all,CFG_QFPROM_PROGRAMMING,CFG_QCOM_QFPROM))
subdirs-$(CFG_QFPROM_PROGRAMMING) += provision
