# Nord (SA8797P / Oryon) OP-TEE platform.

# Threads are expensive in OP-TEE, so they don't have
# to be same as number of cores.
$(call force,CFG_TEE_CORE_NB_CORE,18)

# DARE-TZ secure memory regions. DARE is another in-line
# memory encryption IP similar to pIMEM but on wildcat arch
# it is setup by TME root-of-trust, no specific driver needed
# in OP-TEE for that.
CFG_TZDRAM_START ?= 0xBC280000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000
CFG_TA_RAM_VA_SIZE ?= 0x07B80000

# CFG_QCOM_PAS_PTA disabled: platform driver not yet implemented for Nord.
# Enable after creating core/pta/qcom/pas/platform/nord/ with subsystem code.
# CFG_QCOM_PAS_AUTH (fuse-bound binding checks) depends on CFG_QCOM_PAS_PTA.

CFG_QCOM_PAS_PTA ?= n
ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_RESERVED_VASPACE_SIZE ?= (256 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
CFG_QCOM_PAS_AUTH ?= y
endif

CFG_QCOM_PAS_PTA ?= n
ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_RESERVED_VASPACE_SIZE ?= (256 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
CFG_QCOM_PAS_AUTH ?= y
endif

