/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __ICE_HWKM_H
#define __ICE_HWKM_H

#include <tee_api_types.h>

TEE_Result generate_hw_wrapped_key(uint8_t *out_blob, size_t *out_blob_len);

#endif /* __ICE_HWKM_H */
