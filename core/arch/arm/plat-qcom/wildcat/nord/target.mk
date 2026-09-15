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

$(call force,CFG_QCOM_GPCE,y)
$(call force,CFG_CRYPTO_DRIVER,y)
$(call force,CFG_CRYPTO_DRV_HASH,y)
$(call force,CFG_CRYPTO_DRV_CIPHER,y)
$(call force,CFG_CRYPTO_DRV_MAC,y)
$(call force,CFG_QCOM_GPCE_HASH_TEST,y)
$(call force,CFG_QCOM_GPCE_CIPHER_TEST,y)
$(call force,CFG_QCOM_GPCE_MAC_TEST,y)
