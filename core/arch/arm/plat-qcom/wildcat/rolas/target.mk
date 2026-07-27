# ROLAS (Hafnium SPMC-only) OP-TEE platform.

# ROLAS is an ARMv9.3-A core. This toolchain has no -march/-mcpu support
# for ARMv9.3-A yet (GCC's newest recognized Arm core here is Neoverse V2,
# ARMv9.0-A), so build for Neoverse V2 as an ARMv9.0-A baseline until the
# toolchain gains ARMv9.3-A support.
include core/arch/arm/cpu/neoverse-v2.mk

# ROLAS runs exclusively under Hafnium SPMC (S-EL2): interrupts are
# distributed by the SPMC rather than a directly-owned GIC, and the
# console is a PL011 instead of the other wildcat chipsets' own UART.
$(call force,CFG_CORE_FFA,y)
$(call force,CFG_CORE_SEL2_SPMC,y)
$(call force,CFG_GIC,n)
$(call force,CFG_ARM_GICV3,n)
$(call force,CFG_ARM_GICV4,n)
$(call force,CFG_PL011,y)

# ROLAS uses 48-bit PA instead of the other wildcat chipsets' 40-bit.
$(call force,CFG_CORE_ARM64_PA_BITS,48)

$(call force,CFG_TEE_CORE_NB_CORE,8)
CFG_NUM_THREADS ?= 8

# SPMC memory layout: load address 0xF8200000 (see manifest), entry point
# is load_address + manifest size (0x4000). ROLAS gets its TZDRAM layout
# from its FF-A partition manifest rather than the wildcat DARE-TZ default.
CFG_TZDRAM_START ?= 0xF8204000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000
CFG_TA_RAM_VA_SIZE ?= 0x007fc000
CFG_TZDRAM_SIZE ?= (CFG_TEE_RAM_VA_SIZE + CFG_TA_RAM_VA_SIZE)
