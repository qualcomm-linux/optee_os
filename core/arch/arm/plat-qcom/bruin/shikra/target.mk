# Enable the Qualcomm CSRNG hardware RNG driver.
# Shikra uses the legacy PRNG block via HWKM MMIO.
# Register offsets and base address are defined in target_config.h.
$(call force,CFG_QCOM_CSRNG,y)