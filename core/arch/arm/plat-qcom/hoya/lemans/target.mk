CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_DIAG_LOG ?= $(CFG_TEE_CORE_DEBUG)

ifneq ($(CFG_INSECURE),y)
CFG_QCOM_QFPROM_FUSEPROV ?= y
endif

CFG_QCOM_PAS_PTA ?= y

ifeq ($(CFG_QCOM_PAS_PTA),y)
# PAS subsystems map their controller windows at runtime from the reserved VA
# pool (never released). The six DSP windows total ~146.5 MB; the 60 MB default
# fits only one, so reserve 256 MB with headroom.
CFG_RESERVED_VASPACE_SIZE ?= (256 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe

# Authenticate PIL firmware: at INIT_IMAGE validate the certificate chain,
# signature and fuse bindings, then at AUTH_AND_RESET re-hash the loaded
# segments against the authenticated per-segment hash table before releasing
# the peripheral from reset.
CFG_QCOM_PAS_AUTH ?= y

# Lemans exposes up to 7 PAS subsystems that load concurrently; size the
# per-session metadata table to hold one slot per subsystem, with headroom.
CFG_PAS_MD_SLOTS ?= 8
endif
CFG_QCOM_HWKM ?= y

# Signature authentication reads the OEM root-of-trust anchor, device identity
# and other fuses via the fuse PTA; enable it (and therefore the underlying
# qfprom driver) whenever PAS authentication is on.
ifeq ($(CFG_QCOM_PAS_AUTH),y)
$(call force,CFG_QCOM_FUSE_PTA,y)
endif

# QFPROM backs fuse provisioning (writes) and the fuse PTA (reads) alike;
# enable the driver whenever either consumer is on.
ifneq ($(filter y,$(CFG_QCOM_QFPROM_FUSEPROV) $(CFG_QCOM_FUSE_PTA)),)
$(call force,CFG_QCOM_QFPROM,y)
endif

# CMD_DB/RPMH_CLIENT back the qfprom driver's fuse-write path (voltage rail
# sequencing), needed whenever qfprom itself is enabled.
ifeq ($(CFG_QCOM_QFPROM),y)
$(call force,CFG_QCOM_CMD_DB,y)
$(call force,CFG_QCOM_RPMH_CLIENT,y)
endif
