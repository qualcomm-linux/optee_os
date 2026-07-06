srcs-y += pta_qcom_pas.c
srcs-y += pas_core.c
srcs-$(CFG_QCOM_PAS_HASH_VERIFY) += pas_auth_core.c
incdirs-y += .
incdirs-y += platform/
subdirs-y += platform/$(PLATFORM_FLAVOR)
