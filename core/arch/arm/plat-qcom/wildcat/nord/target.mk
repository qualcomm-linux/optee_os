# Nord (SA8797P / Oryon) OP-TEE platform.

include core/arch/arm/cpu/cortex-armv8-0.mk

$(call force,CFG_GIC,y)
$(call force,CFG_ARM_GICV4,y)
$(call force,CFG_QCOM_GENI_UART,y)
$(call force,CFG_CORE_ARM64_PA_BITS,40)

# The GENI UART is shared with the Linux kernel and an excessively long
# wait period may lead to RCU stall warnings depending on system load.
# Make this value configurable per platform.
CFG_QCOM_GENI_UART_RDY_WAIT_USEC ?= 1000

# DARE-TZ secure memory regions. DARE is another in-line memory encryption
# IP similar to pIMEM but on wildcat arch it is setup by TME root-of-trust,
# no specific driver needed in OP-TEE for that.
CFG_TZDRAM_START ?= 0xBC280000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000
CFG_TA_RAM_VA_SIZE ?= 0x07B80000
CFG_TZDRAM_SIZE ?= (CFG_TEE_RAM_VA_SIZE + CFG_TA_RAM_VA_SIZE)

# Threads are expensive in OP-TEE, so they don't have
# to be same as number of cores.
$(call force,CFG_TEE_CORE_NB_CORE,18)
