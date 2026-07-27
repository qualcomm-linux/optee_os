# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

$(call force,CFG_GIC,y)
$(call force,CFG_CORE_ARM64_PA_BITS,40)
$(call force,CFG_QCOM_GENI_UART,y)

# The GENI UART is shared with the Linux kernel and an excessively long
# wait period may lead to RCU stall warnings depending on system load.
# Make this value configurable per platform.
CFG_QCOM_GENI_UART_RDY_WAIT_USEC ?= 1000

CFG_QCOM_SEC_WDOG ?= y
