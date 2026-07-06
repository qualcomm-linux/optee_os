global-incdirs-y += ../include
srcs-y += qcom_pas.c
srcs-$(CFG_QCOM_PAS_HASH_VERIFY) += pas_hashseg.c
