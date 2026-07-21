global-incdirs-y += .
global-incdirs-y += platform/$(PLATFORM_FLAVOR)

srcs-y += clk-qcom.c
srcs-$(CFG_QCOM_PAS_PTA) += platform/$(PLATFORM_FLAVOR)/clk-qcom-pas.c
srcs-$(CFG_QCOM_CLK_BSP) += platform/$(PLATFORM_FLAVOR)/clk-qcom-lemans.c

incdirs-y += .
incdirs-$(CFG_QCOM_PAS_PTA) += platform/$(PLATFORM_FLAVOR)
incdirs-$(CFG_QCOM_CLK_BSP) += platform/$(PLATFORM_FLAVOR)
