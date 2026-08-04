# Wildcat architecture configuration

include core/arch/arm/cpu/cortex-armv8-0.mk

$(call force,CFG_ARM_GICV4,y)

# DARE-TZ secure memory regions. DARE is another in-line
# memory encryption IP similar to pIMEM but on wildcat arch
# it is setup by TME root-of-trust, no specific driver needed
# in OP-TEE for that.
CFG_TZDRAM_START ?= 0xBC280000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000
CFG_TA_RAM_VA_SIZE ?= 0x07B80000
CFG_TZDRAM_SIZE ?= (CFG_TEE_RAM_VA_SIZE + CFG_TA_RAM_VA_SIZE)
CFG_NUM_THREADS ?= 8

# OP-TEE re-verifies each loaded PIL firmware segment against the image's
# hash table before releasing the peripheral from reset.
CFG_QCOM_PAS_HASH_VERIFY ?= y

# Authenticate each image's certificate chain and signature, and bind it to
# its peripheral and device, before trusting its hash table.
CFG_QCOM_PAS_SECURE_BOOT ?= y
