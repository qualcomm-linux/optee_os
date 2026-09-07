# Nord (SA8797P / Oryon) OP-TEE platform.

# Threads are expensive in OP-TEE, so they don't have
# to be same as number of cores.
$(call force,CFG_TEE_CORE_NB_CORE,18)

# CFG_QCOM_PAS_PTA enables PIL segment-hash verification (no fuse dependency).
# It is disabled here because core/pta/qcom/pas/sub.mk unconditionally requires
# platform/$(PLATFORM_FLAVOR)/sub.mk to exist, regardless of any CFG_QCOM_PAS_*
# flag. That directory holds the per-subsystem PIL driver implementation
# (subsys.c implementing qcom_pas_platform_subsys(), plus one .c/.h pair per
# remoteproc peripheral — CDSP, LPASS, IRIS, etc. — with real register/clock/
# reset sequencing for that subsystem on this chip). That platform-specific
# driver code does not exist for Nord yet.
#
# To enable PAS for Nord:
#   1. Create core/pta/qcom/pas/platform/nord/ with subsys.c and
#      per-subsystem driver files (CDSP, LPASS, IRIS, etc.)
#   2. Set CFG_QCOM_PAS_PTA ?= y below
#
# CFG_QCOM_PAS_AUTH (certificate chain, signature and fuse-bound binding
# checks for PIL images) is nested inside the CFG_QCOM_PAS_PTA block, mirroring
# lemans/target.mk, so it takes effect once the platform driver above exists
# and CFG_QCOM_PAS_PTA is turned on. Fuse reads use sense registers (hardware
# shadow of fuse rows) instead of the QFPROM driver, so no CFG_QCOM_QFPROM
# dependency. See the "chore: use..." commit for the sense-register fuse
# read implementation.
#
CFG_QCOM_PAS_PTA ?= n
ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_RESERVED_VASPACE_SIZE ?= (256 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
CFG_QCOM_PAS_AUTH ?= y
endif

