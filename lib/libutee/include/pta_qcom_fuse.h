/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PTA_QCOM_FUSE_H
#define __PTA_QCOM_FUSE_H

/*
 * Interface to the pseudo TA which exposes Qualcomm fuse (QFPROM) state to
 * user-mode TAs that cannot access the QFPROM core driver directly.
 */

#define PTA_QCOM_FUSE_UUID { 0x6b46384c, 0x4a3e, 0x4b9d, \
		{ 0xa8, 0x2f, 0x1c, 0x3d, 0xe5, 0x9f, 0xa2, 0x11 } }

/*
 * Query whether secure boot (image authentication) is enabled.
 *
 * Reads the SECURE_BOOTn AUTH_EN bit for the APPS code segment.
 *
 * [out] params[0].value.a:  1 if secure boot is enabled, 0 otherwise
 */
#define PTA_QCOM_FUSE_GET_SECBOOT_STATE		1

/* Size of the OEM root-of-trust digest (SHA-384) returned below. */
#define PTA_QCOM_FUSE_ROOT_OF_TRUST_SIZE	48

/*
 * Read the OEM root-of-trust digest from the PK_HASH_0 fuse region.
 *
 * [out] params[0].memref:  buffer receiving the digest; must be at least
 *                          PTA_QCOM_FUSE_ROOT_OF_TRUST_SIZE bytes
 */
#define PTA_QCOM_FUSE_GET_ROOT_OF_TRUST		2

/*
 * Read the device-identity fields used for metadata binding.
 *
 * [out] params[0].value.a:  OEM_ID
 * [out] params[0].value.b:  MODEL_ID
 * [out] params[1].value.a:  JTAG_ID (authentication bits, masked 0x0FFFFFFF)
 * [out] params[1].value.b:  serial number
 */
#define PTA_QCOM_FUSE_GET_DEVICE_IDS		3

/*
 * Read the SOC hardware version family|device field (bits 31:16 of the TCSR
 * SOC_HW_VERSION register), matching the metadata soc_vers binding value.
 *
 * [out] params[0].value.a:  family|device number
 */
#define PTA_QCOM_FUSE_GET_SOC_HW_VERSION	4

/*
 * Read the per-segment hash digest size selected by a metadata root_cert_sel
 * index.
 *
 * [in]  params[0].value.a:  root_cert_sel (0-3)
 * [out] params[0].value.b:  digest size in bytes (32 = SHA-256, 48 = SHA-384)
 */
#define PTA_QCOM_FUSE_GET_SEGMENT_HASH_SIZE	5

/*
 * Query whether Extended Key Usage enforcement is fused on. When enabled,
 * image certificate chains must carry the code-signing EKU OID on the leaf.
 *
 * [out] params[0].value.a:  1 if EKU enforcement is enabled, 0 otherwise
 */
#define PTA_QCOM_FUSE_GET_EKU_ENFORCEMENT_EN	6

/*
 * Query the APPS SECURE_BOOTn USE_SERIAL_NUM fuse override. When set,
 * serial-number binding is forced regardless of the image metadata's own
 * serial-number-binding flag.
 *
 * [out] params[0].value.a:  1 if the override is blown, 0 otherwise
 */
#define PTA_QCOM_FUSE_GET_USE_SERIAL_NUM	7

/*
 * Query whether OEM image encryption (UIE) is provisioned, i.e. whether the
 * OEM_CONFIG0 IMAGE_ENCRYPTION_ENABLE fuse is blown.
 *
 * [out] params[0].value.a:  1 if image encryption is enabled, 0 otherwise
 */
#define PTA_QCOM_FUSE_GET_IMAGE_ENCRYPTION_EN	8

#endif /* __PTA_QCOM_FUSE_H */
