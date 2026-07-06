/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __QFPROM_H__
#define __QFPROM_H__

#include <stdbool.h>
#include <stdint.h>
#include <tee_api_types.h>

enum qfprom_addr_space {
	QFPROM_ADDR_SPACE_RAW = 0,
	QFPROM_ADDR_SPACE_CORR = 1,
};

enum qfprom_error {
	QFPROM_NO_ERR = 0x0,
	QFPROM_ERR_UNKNOWN = 0x1,
	QFPROM_DATA_PTR_NULL_ERR = 0x2,
	QFPROM_ADDRESS_INVALID_ERR = 0x3,
	QFPROM_WRITE_ERR = 0x4,
	QFPROM_REGION_NOT_SUPPORTED_ERR = 0x5,
	QFPROM_REGION_NOT_READABLE_ERR = 0x6,
	QFPROM_REGION_NOT_WRITABLE_ERR = 0x7,
	QFPROM_FEC_ERR = 0x8,
	QFPROM_OPERATION_NOT_ALLOWED_ERR = 0x9,
	QFPROM_FAILED_TO_CHANGE_VOLTAGE_ERR = 0xA,
	QFPROM_ERROR_CLOCK_FAILED = 0x10,
	QFPROM_ERROR_TIMEOUT = 0x11,
};

/* Read QFPROM row data */
TEE_Result qfprom_read_row(uint32_t addr,
			   enum qfprom_addr_space type,
			   uint32_t *data);

/*
 * Report whether secure boot (image authentication) is enabled for the APPS
 * code segment, i.e. whether the SECURE_BOOT AUTH_EN fuse is blown.
 *
 * @enabled: set to true if secure boot is enabled, false otherwise
 *
 * Return TEE_SUCCESS on success or an error if the state cannot be read.
 */
TEE_Result qcom_secboot_is_enabled(bool *enabled);

/*
 * Report whether the APPS SECURE_BOOTn USE_SERIAL_NUM fuse override is
 * blown. When set, serial-number binding is forced regardless of the
 * image metadata's own serial-number-binding flag.
 *
 * @enabled: set to true if the override is blown, false otherwise
 *
 * Return TEE_SUCCESS on success or an error if the state cannot be read.
 */
TEE_Result qcom_secboot_get_use_serial_num(bool *enabled);

/*
 * Read the OEM root-of-trust digest from the PK_HASH_0 fuse region. This is
 * the hash the secure-boot ROM compares the firmware signing root certificate
 * against. It is QFPROM_ROOT_OF_TRUST_BYTE_SIZE (48 bytes, SHA-384) on the
 * supported platforms.
 *
 * @hash: output buffer for the digest
 * @len:  size of @hash; must equal QFPROM_ROOT_OF_TRUST_BYTE_SIZE
 *
 * Return TEE_SUCCESS on success or an error if the value cannot be read.
 */
TEE_Result qcom_secboot_get_root_of_trust(uint8_t *hash, size_t len);

/*
 * Read the PIL subsystem anti-rollback device version from the QFPROM.
 *
 * If the PIL ARB enable fuse is blown AND secure boot is enabled, returns
 *   popcount(PIL_SUBSYSTEM0_LSB) + popcount(PIL_SUBSYSTEM1_MSB)
 * otherwise returns 0 (enforcement disabled).
 *
 * @version: populated with the device version on success
 *
 * Return TEE_SUCCESS on success or an error if fuses cannot be read.
 */
TEE_Result qcom_secboot_get_pil_rollback_version(uint32_t *version);

/*
 * Advance the PIL subsystem anti-rollback fuse to @version. The version is
 * unary ("thermometer") encoded and the write only blows additional bits
 * (monotonic). No-op when @version is
 * not greater than the current device version, when ARB enforcement is not
 * active (PIL ARB enable fuse or secure boot unblown), or when QFPROM
 * programming is not built in.
 *
 * @version: image version to record as the new device floor
 *
 * Return TEE_SUCCESS on success (including the no-op cases) or an error if the
 * fuse write fails.
 */
TEE_Result qcom_secboot_blow_pil_rollback_version(uint32_t version);

/*
 * Device-identity values read from the SECURITY_CONTROL sense registers,
 * used to bind signed image metadata to this device.
 *
 * @oem_id:     OEM identifier (OEM_ID sense register, bits 31:16)
 * @model_id:   product/model identifier (OEM_ID sense register, bits 15:0)
 * @jtag_id:    JTAG ID masked to the authentication bits (0x0FFFFFFF)
 * @serial_num: device serial number
 */
struct qcom_secboot_device_ids {
	uint32_t oem_id;
	uint32_t model_id;
	uint32_t jtag_id;
	uint32_t serial_num;
};

/*
 * Read the device-identity fields (OEM_ID, MODEL_ID, JTAG_ID, serial number)
 * from the SECURITY_CONTROL sense registers.
 *
 * @ids: populated on success
 *
 * Return TEE_SUCCESS on success or an error if the registers cannot be read.
 */
TEE_Result qcom_secboot_get_device_ids(struct qcom_secboot_device_ids *ids);

/*
 * Read the SOC hardware version family|device field (bits 31:16 of the TCSR
 * SOC_HW_VERSION register), matching the value the metadata soc_vers binding
 * compares against.
 *
 * @fam_dev: populated with the family|device number on success
 *
 * Return TEE_SUCCESS on success or an error if the register cannot be read.
 */
TEE_Result qcom_secboot_get_soc_hw_version(uint32_t *fam_dev);

/*
 * Read the digest size the per-segment hash table uses for a given
 * root_cert_sel index: the OEM metadata's root_cert_sel (word 28, range
 * 0-3) selects one of four
 * OEM_CONFIG2 fuse bits on platforms that implement the field (lemans); the
 * bit is 1 for SHA-256, 0 for SHA-384. Platforms without the field (kodiak)
 * always report SHA-384.
 *
 * @root_cert_sel: metadata root_cert_sel index (0-3)
 * @hash_size:     populated with the digest size in bytes (32 or 48)
 *
 * Return TEE_SUCCESS on success, TEE_ERROR_BAD_PARAMETERS if root_cert_sel is
 * out of range.
 */
TEE_Result qcom_secboot_get_segment_hash_size(uint32_t root_cert_sel,
					      uint32_t *hash_size);

/*
 * Read the OEM_CONFIG2 EKU_ENFORCEMENT_EN fuse bit. When enabled, image
 * certificate chains must carry the code-signing Extended Key Usage OID
 * on the leaf. Same register/bit on lemans and kodiak.
 *
 * @enabled: populated with the fuse bit's state on success
 *
 * Return TEE_SUCCESS on success or an error if the register cannot be read.
 */
TEE_Result qcom_secboot_get_eku_enforcement_en(bool *enabled);

/*
 * Read the multiple-root-certificate (MRC) provisioning state.
 *
 * When more than one root certificate is provisioned, root selection is
 * active: a signed image nominates which root it chains to, and the returned
 * activation/revocation lists (4 bits each, one per root index) gate which
 * roots may be used. When a single root is provisioned, selection is
 * disabled and @num_roots is reported as 1.
 *
 * @root_sel_enabled: set true when multiple roots are provisioned
 * @num_roots:        number of provisioned roots (1 when selection disabled)
 * @activation_list:  per-index active bitmap (bit i set => root i active)
 * @revocation_list:  per-index revoked bitmap (bit i set => root i revoked)
 *
 * Return TEE_SUCCESS on success or an error if the fuses cannot be read.
 */
TEE_Result qcom_secboot_get_mrc_info(bool *root_sel_enabled,
				     uint32_t *num_roots,
				     uint32_t *activation_list,
				     uint32_t *revocation_list);

/* Write QFPROM row data */
TEE_Result qfprom_write_row(uint32_t addr, uint32_t *data);

/* Check if row has FEC protection */
TEE_Result qfprom_row_has_fec_bits(uint32_t addr,
				   enum qfprom_addr_space type,
				   uint8_t *has_fec);

/* Calculate FEC bits for 56-bit data */
uint32_t qfprom_fec_63_56_bit(uint32_t lsb_data, uint32_t msb_data);

/* Hardware init/deinit for batch fuse operations */
TEE_Result qfprom_hw_init(void);
void qfprom_hw_deinit(void);

#endif /* __QFPROM_H__ */
