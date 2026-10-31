/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 *
 * Crypto Engine register access definitions.
 */

#ifndef QCOM_GPCE_CE_HWIO_H
#define QCOM_GPCE_CE_HWIO_H

#include <stdbool.h>
#include <kernel/mutex.h>
#include <mm/core_memprot.h>
#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>
#include <tee_api_defines.h>

#include "hash.h"

TEE_Result CE_HWIO_set_ce_ctx(hash_ctx_t *ctx, size_t auth_sz,
			      size_t seg_sz, bool first, bool last);
TEE_Result CE_HWIO_get_ce_ctx(hash_ctx_t *ctx);
TEE_Result CE_HWIO_xfer_data(hash_ctx_t *ctx, const uint8_t *ibuf,
			     size_t isz);

#endif /* QCOM_GPCE_CE_HWIO_H */
