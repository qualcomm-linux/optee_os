user-ta-uuid := cff7d191-7ca0-4784-af13-48223b9a4fbe

# INIT_IMAGE metadata (ELF hdr + phdr table + hash segment) is up to
# ~400 KB. Size the heap to hold a full TEE-private copy when
# CFG_QCOM_PAS_AUTH is enabled.
ifeq ($(CFG_QCOM_PAS_AUTH),y)
# X.509 cert-chain parsing and ECDSA/ECP verification need working
# memory beyond the metadata copy; give it headroom.
CFG_PAS_TA_HEAP_SIZE ?= (1024 * 1024)
else
CFG_PAS_TA_HEAP_SIZE ?= (4 * 1024)
endif

# Authenticate each PIL firmware image: at INIT_IMAGE validate the
# certificate chain, signature and fuse bindings (secure-boot devices),
# then at AUTH_AND_RESET re-hash the loaded segments against the
# authenticated per-segment hash table before releasing the peripheral
# from reset.
CFG_QCOM_PAS_AUTH ?= n
