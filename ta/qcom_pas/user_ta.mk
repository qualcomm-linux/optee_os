user-ta-uuid := cff7d191-7ca0-4784-af13-48223b9a4fbe

# The metadata blob passed at INIT_IMAGE is ELF hdr + phdr table + hash
# segment — up to ~400 KB for the larger QCS9100 DSP images. Set the heap
# large enough to hold a full copy plus working room.
CFG_PAS_TA_HEAP_SIZE ?= (512 * 1024)

# When enabled, the TA re-verifies each loaded firmware segment against the
# image's per-segment hash table before the peripheral is brought out of
# reset. Disable on configurations where the TA is not responsible for
# verifying the image.
CFG_QCOM_PAS_HASH_VERIFY ?= n
