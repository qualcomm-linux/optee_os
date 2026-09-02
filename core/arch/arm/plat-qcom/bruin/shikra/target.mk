# Shikra OP-TEE platform.

# Threads are expensive in OP-TEE, so they don't have to match core count.
$(call force,CFG_TEE_CORE_NB_CORE,18)

$(call force,CFG_QCOM_RPM_CLIENT,y)
CFG_QCOM_RPM_TRANSPORT_STUB ?= y
