global-incdirs-y += .
global-incdirs-y += platform/$(PLATFORM_FLAVOR)

srcs-y += clk-qcom.c
srcs-$(CFG_QCOM_PAS_PTA) += platform/$(PLATFORM_FLAVOR)/clk-qcom-pas.c
srcs-$(CFG_QCOM_CLK_BSP) += platform/$(PLATFORM_FLAVOR)/clk-qcom-bsp.c

# Rail-vote backend for the QUP SE walker. RPMh targets build the RPMh
# backend; an RPM-based target selects its own instead. A CLK_BSP build with
# no backend fails to link rather than silently under-volting.
ifeq ($(CFG_QCOM_CLK_BSP),y)
srcs-$(CFG_QCOM_RPMH_CLIENT) += clk-qcom-vreg.c
endif

incdirs-y += .
incdirs-$(CFG_QCOM_PAS_PTA) += platform/$(PLATFORM_FLAVOR)
incdirs-$(CFG_QCOM_CLK_BSP) += platform/$(PLATFORM_FLAVOR)
