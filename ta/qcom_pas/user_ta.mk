user-ta-uuid := cff7d191-7ca0-4784-af13-48223b9a4fbe

# INIT_IMAGE metadata (ELF hdr + phdr table + hash segment) is up to
# ~400 KB. Size the heap to hold a full TEE-private copy when
# CFG_QCOM_PAS_AUTH is enabled.
ifeq ($(CFG_QCOM_PAS_AUTH),y)
CFG_PAS_TA_HEAP_SIZE ?= (512 * 1024)
else
CFG_PAS_TA_HEAP_SIZE ?= (4 * 1024)
endif

# Verify each PIL firmware image before releasing the peripheral from
# reset: re-hash the loaded segments and compare them against the
# per-segment hash table carried in the image metadata.
CFG_QCOM_PAS_AUTH ?= n
