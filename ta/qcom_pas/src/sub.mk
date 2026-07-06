global-incdirs-y += ../include
srcs-y += qcom_pas.c
srcs-$(CFG_QCOM_PAS_HASH_VERIFY) += pas_hashseg.c
srcs-$(CFG_QCOM_PAS_SECURE_BOOT) += pas_auth.c
srcs-$(CFG_QCOM_PAS_SECURE_BOOT) += pas_auth_sig.c
srcs-$(CFG_QCOM_PAS_SECURE_BOOT) += pas_policy.c
