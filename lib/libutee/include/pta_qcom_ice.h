/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PTA_QCOM_ICE_H
#define __PTA_QCOM_ICE_H

/*
 * Qualcomm ICE PTA - Filesystem Encryption using the Inline Crypto Engine
 * Provides hardware key configuration for ICE-accelerated storage encryption.
 * Storage-controller agnostic: caters to ICE blocks on different storage
 * controllers (eMMC/SDCC, UFS, ...).
 * UUID: {29e87b9e-012a-4878-a1e1-a1b90a215b16}
 */
#define PTA_QCOM_ICE_UUID \
	{ 0x29e87b9e, 0x012a, 0x4878, \
		{ 0xa1, 0xe1, 0xa1, 0xb9, 0x0a, 0x21, 0x5b, 0x16 } }

/*
 * Generate a hardware-wrapped ICE storage key blob.
 *
 * Caller supplies an output buffer and its size.
 *
 * [out] params[0].memref.buffer     Output wrapped key blob buffer
 * [in/out] params[0].memref.size    Input: buffer capacity
 *                                   Output: actual blob size
 *                                   Required size: 68 bytes (HWKM_MAX_BLOB_SIZE)
 *                                   If too small, returns TEE_ERROR_SHORT_BUFFER
 *                                   and updates size with required length.
 */
#define PTA_CMD_ICE_GENERATE_KEY        2

/*
 * Import key material and return a hardware-wrapped ICE storage key blob.
 *
 * [in]  params[0].memref.buffer      Input key material to import
 * [in]  params[0].memref.size        Input key length (1..HWKM_MAX_KEY_SIZE)
 * [out] params[1].memref.buffer      Output wrapped key blob buffer
 * [in/out] params[1].memref.size     Input: buffer capacity
 *                                    Output: actual blob size
 *                                    Required size: 68 bytes (HWKM_MAX_BLOB_SIZE)
 *                                    If too small, returns TEE_ERROR_SHORT_BUFFER
 *                                    and updates size with required length.
 */
#define PTA_CMD_ICE_IMPORT_KEY          3

#endif /* __PTA_EMMC_ICE_H */

