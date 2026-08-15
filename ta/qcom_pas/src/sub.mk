global-incdirs-y += ../include
srcs-y += qcom_pas.c
srcs-$(CFG_QCOM_PAS_AUTH) += pas_auth.c pas_fuse.c pas_mbn_parser.c pas_meta.c
