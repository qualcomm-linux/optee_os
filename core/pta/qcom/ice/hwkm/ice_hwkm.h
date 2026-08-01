/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __ICE_HWKM_H
#define __ICE_HWKM_H

#include <tee_api_types.h>

TEE_Result clear_ice_slave_slot_hwkm(uint32_t slot);

TEE_Result set_config_ice_key_using_hwkm(uint32_t slot,
					 const uint8_t *wrapped_blob,
					 size_t wrapped_blob_len);

TEE_Result export_hw_wrapped_key(const uint8_t *in_blob, size_t in_blob_len,
                                 uint8_t *out_blob, size_t *out_blob_len);

TEE_Result import_and_wrap_with_hw_key(const uint8_t *in_key, size_t in_key_len,
				       uint8_t *out_blob, size_t *out_blob_len);

TEE_Result generate_hw_wrapped_key(uint8_t *out_blob, size_t *out_blob_len);

#endif /* __ICE_HWKM_H */
