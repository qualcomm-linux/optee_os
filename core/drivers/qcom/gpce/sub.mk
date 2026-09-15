# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# All rights reserved.
#
# GPCE BAM driver

global-incdirs-y += .

global-incdirs-y += ../../crypto/crypto_api/include

# Environment
srcs-y += qce_env_optee.c

# BAM core
srcs-y += bams.c
srcs-y += ce_hal.c
srcs-y += ce_bam.c

# UCLIB engines
srcs-y += cipher.c
srcs-y += hash.c
srcs-y += mac.c

# OP-TEE integration
srcs-y += qce_cipher.c
srcs-y += qce_hash.c
srcs-y += qce_mac.c
