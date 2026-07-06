/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __QFPROM_TARGET_H__
#define __QFPROM_TARGET_H__

#include <clock_group_qcom.h>
#include <platform_config.h>
#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>
#include <util.h>

#define QFPROM_RAW_BASE                         0x00780000
#define QFPROM_CORR_BASE                        0x00784000
#define QFPROM_SIZE                             0x4000

/*
 * SECURE_BOOTn secure-control register array holding the per-code-segment
 * AUTH_EN, PK_HASH_IN_FUSE and USE_SERIAL_NUM bits. Authentication is
 * required when AUTH_EN is blown; the root-of-trust anchor lives in the
 * PK_HASH_0 fuse rather than a ROM constant when PK_HASH_IN_FUSE is blown.
 * The APPS code segment is index 1 within the array.
 */
#define SECURE_BOOT_APPS_ADDR			(SECURITY_CONTROL_BASE + 0x606c)
#define SECURE_BOOT_AUTH_EN_BMSK		0x20
#define SECURE_BOOT_USE_SERIAL_NUM_BMSK		0x40
#define SECURE_BOOT_PK_HASH_IN_FUSE_BMSK	0x10

/*
 * Size of the OEM root-of-trust digest stored in the PK_HASH_0 fuse region.
 * SHA-384 (48 bytes) on this platform, matching the secure-boot ROM.
 */
#define QFPROM_ROOT_OF_TRUST_BYTE_SIZE		48

/*
 * PIL subsystem anti-rollback fuse layout (lemans).
 *
 * EN bit: OEM_CONFIG_ROW2_MSB[2] (corrected addr 0x7841BC, mask 0x4).
 * PIL_SUBSYSTEM0 (LSB, bits 31:0):  ANTI_ROLLBACK_9 corrected 0x784278.
 * PIL_SUBSYSTEM1 (MSB, bits 59:32): ANTI_ROLLBACK_9+4 corrected 0x78427C,
 *                                   mask 0x0fffffff (28 bits used).
 * Device version = popcount(LSB) + popcount(MSB) (lemans counts both the
 *                   LSB and MSB fuse banks).
 */
#define PIL_ARB_EN_CORR_ADDR			(QFPROM_CORR_BASE + 0x01BC)
#define PIL_ARB_EN_BMSK				0x00000004
#define PIL_ARB_LSB_CORR_ADDR			(QFPROM_CORR_BASE + 0x0278)
#define PIL_ARB_LSB_BMSK			0xffffffff
#define PIL_ARB_MSB_CORR_ADDR			(QFPROM_CORR_BASE + 0x027C)
#define PIL_ARB_MSB_BMSK			0x0fffffff
/* lemans counts both the LSB and MSB fuse banks */
#define PIL_ARB_MSB_ENABLED			1
/* Raw row for blowing the ARB fuse (ANTI_ROLLBACK_9). */
#define PIL_ARB_RAW_ADDR			(QFPROM_RAW_BASE + 0x0278)
/* Max unary-encoded versions per bank (LSB = 32 bits, MSB = 28 bits). */
#define PIL_ARB_LSB_MAX_VERSION			32
#define PIL_ARB_MSB_MAX_VERSION			28

/*
 * Device-identity sense registers in the SECURITY_CONTROL block (hardware
 * shadow of the underlying OEM_CONFIG / PTE fuse rows). Same offsets on
 * lemans and kodiak. Used to bind signed image metadata to this device.
 */
#define OEM_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x6138)
#define OEM_ID_BMSK				0xffff0000
#define OEM_ID_SHFT				16
#define MODEL_ID_BMSK				0x0000ffff
#define MODEL_ID_SHFT				0
#define JTAG_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x6130)
#define JTAG_ID_AUTH_BMSK			0x0fffffff
#define SERIAL_NUM_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x6134)

/*
 * SOC hardware version lives in a TCSR register (not a fuse). The metadata
 * soc_vers binding compares against the family|device field (bits 31:16).
 */
#define TCSR_SOC_HW_VERSION_ADDR		0x01FC8000
#define SOC_HW_VERSION_FAM_DEV_BMSK		0xffff0000
#define SOC_HW_VERSION_FAM_DEV_SHFT		16

/*
 * OEM_CONFIG2 fuse register (SECURITY_CONTROL block), holding the
 * EKU_ENFORCEMENT_EN bit (same offset/bit on lemans and kodiak) and, on
 * lemans only, the per-segment hash algorithm select bits below.
 */
#define OEM_CONFIG2_ADDR			(SECURITY_CONTROL_BASE + 0x6054)
#define EKU_ENFORCEMENT_EN_SHFT			30

/*
 * Per-segment hash algorithm select (lemans only): bit
 * SEGMENT_HASH_FUNCTION_SELECT0_SHFT + root_cert_sel of OEM_CONFIG2 is
 * indexed by the metadata's root_cert_sel field (0-3): bit set -> SHA-256,
 * else -> SHA-384. kodiak has no such fuse field and always hashes segments
 * with SHA-384.
 */
#define SEGMENT_HASH_SELECT_SUPPORTED		1
#define SEGMENT_HASH_FUNCTION_SELECT0_SHFT	16

/*
 * Multiple-root-certificate (MRC) fuse fields (SECURITY_CONTROL sense
 * registers). OEM_CONFIG0 ROOT_CERT_TOTAL_NUM holds (number of provisioned
 * root certificates - 1); the activation/revocation lists are 4-bit fields,
 * one bit per root index.
 */
#define OEM_CONFIG0_ADDR			(SECURITY_CONTROL_BASE + 0x604c)
#define ROOT_CERT_TOTAL_NUM_BMSK		0x00060000
#define ROOT_CERT_TOTAL_NUM_SHFT		17
#define MRC_ACTIVATION_LIST_ADDR		(SECURITY_CONTROL_BASE + 0x6268)
#define MRC_REVOCATION_LIST_ADDR		(SECURITY_CONTROL_BASE + 0x6270)
#define MRC_ROOT_CERT_LIST_BMSK			0x0000000f

#define QFPROM_BLOW_TIMER_OFFSET                0x2030
#define QFPROM_ACCEL_OFFSET                     0x2038

#define QFPROM_BLOW_STATUS_OFFSET               0x203c
#define QFPROM_BLOW_STATUS_RMSK                 0x3
#define QFPROM_BLOW_STATUS_BUSY_VAL             0x1
#define QFPROM_BLOW_STATUS_ERROR_VAL            0x2
#define QFPROM_BLOW_STATUS_READY_VAL            0x0

#define QFPROM_FEC_ESR_OFFSET                   0x2058
#define QFPROM_FEC_EAR_OFFSET                   0x205c
#define QFPROM_FEC_ESR_ERR_SEEN_BMSK            BIT(0)
#define QFPROM_FEC_EAR_ERR_ADDR_BMSK            0xFFFF

#define QFPROM_BLOW_TIMER_CLK_FREQ_MHZ_X10      48
#define QFPROM_FUSE_BLOW_TIME_IN_US             5

#define QFPROM_GATELAST_VAL                     0x1
#define QFPROM_TRIPPT_SEL_VAL                   0x4
#define QFPROM_ACCEL_VAL                        0x10

#define QFPROM_GATELAST_SHFT                    11
#define QFPROM_TRIPPT_SEL_SHFT                  8
#define QFPROM_ACCEL_SHFT                       0

#define QFPROM_ACCEL_VALUE \
	((QFPROM_GATELAST_VAL << QFPROM_GATELAST_SHFT) | \
	 (QFPROM_TRIPPT_SEL_VAL << QFPROM_TRIPPT_SEL_SHFT) | \
	 (QFPROM_ACCEL_VAL << QFPROM_ACCEL_SHFT))

#define QFPROM_ACCEL_RESET_VALUE \
	(0x1 << QFPROM_GATELAST_SHFT)

#define QFPROM_RAW_TO_CORR(raw_addr) \
	((raw_addr) + (QFPROM_CORR_BASE - QFPROM_RAW_BASE))

#define LCM_ADDR                                0x00780120
#define PRI_KEY_DERIVATION_KEY_ADDR             0x00780128
#define MRC_2_0_ADDR                            0x00780148
#define PTE_ADDR                                0x00780158
#define READ_PERMISSION_ADDR                    0x00780190
#define WRITE_PERMISSION_ADDR                   0x00780198
#define FEC_ENABLES_ADDR                        0x007801A0
#define OEM_CONFIG_ADDR                         0x007801A8
#define FEATURE_CONFIG_M_ADDR                   0x007801D0
#define FEATURE_CONFIG_NM_ADDR                  0x00780210
#define ANTI_ROLLBACK_1_ADDR                    0x00780238
#define ANTI_ROLLBACK_2_ADDR                    0x00780240
#define ANTI_ROLLBACK_3_ADDR                    0x00780248
#define ANTI_ROLLBACK_4_ADDR                    0x00780250
#define ANTI_ROLLBACK_5_ADDR                    0x00780258
#define ANTI_ROLLBACK_6_ADDR                    0x00780260
#define ANTI_ROLLBACK_7_ADDR                    0x00780268
#define ANTI_ROLLBACK_8_ADDR                    0x00780270
#define ANTI_ROLLBACK_9_ADDR                    0x00780278
#define ANTI_ROLLBACK_10_ADDR                   0x00780280
#define ANTI_ROLLBACK_11_ADDR                   0x00780288
#define ANTI_ROLLBACK_12_ADDR                   0x00780290
#define ANTI_ROLLBACK_13_ADDR                   0x00780298
#define PK_HASH_0_ADDR                          0x007802A0
#define CALIBRATION_ADDR                        0x007802E0
#define MEMORY_CONFIGURATION_ADDR               0x00780490
#define QC_SPARE_20_ADDR                        0x00780D50
#define QC_SPARE_21_ADDR                        0x00780D58
#define OEM_IMAGE_ENCRYPTION_KEY_ADDR           0x00780D60
#define OEM_SECURE_BOOT_ADDR                    0x00780D78
#define SEC_KEY_DERIVATION_KEY_ADDR             0x00780D88
#define IMAGE_ENCRYPTION_KEY_1_ADDR             0x00781150
#define USER_KEY_DERIVATION_KEY_ADDR            0x00781168
#define OEM_SPARE_28_ADDR                       0x00781190
#define OEM_SPARE_29_ADDR                       0x007811A0
#define OEM_SPARE_30_ADDR                       0x007811B0
#define OEM_SPARE_31_ADDR                       0x007811C0

#define QFPROM_READ_PERM_LSB_OFFSET \
	(READ_PERMISSION_ADDR - QFPROM_RAW_BASE)
#define QFPROM_READ_PERM_MSB_OFFSET \
	(QFPROM_READ_PERM_LSB_OFFSET + 4)

#define QFPROM_WRITE_PERM_LSB_OFFSET \
	(WRITE_PERMISSION_ADDR - QFPROM_RAW_BASE)
#define QFPROM_WRITE_PERM_MSB_OFFSET \
	(QFPROM_WRITE_PERM_LSB_OFFSET + 4)

#define QFPROM_HW_MUTEX_ID			8
#define QFPROM_HW_MUTEX_PID			1
#define QFPROM_HW_MUTEX_TIMEOUT_US		10000
#define QFPROM_MUTEX_REG_ADDR \
	(TCSR_MUTEX_BASE + (0x1000 * QFPROM_HW_MUTEX_ID))

enum qfprom_region_name {
	QFPROM_CRI_CM_PRIVATE_REGION = 0,
	QFPROM_LCM_REGION,
	QFPROM_PRI_KEY_DERIVATION_KEY_REGION,
	QFPROM_MRC_2_0_REGION,
	QFPROM_PTE_REGION,
	QFPROM_READ_PERMISSION_REGION,
	QFPROM_WRITE_PERMISSION_REGION,
	QFPROM_FEC_ENABLES_REGION,
	QFPROM_OEM_CONFIG_REGION,
	QFPROM_FEATURE_CONFIG_M_REGION,
	QFPROM_FEATURE_CONFIG_NM_REGION,
	QFPROM_ANTI_ROLLBACK_1_REGION,
	QFPROM_ANTI_ROLLBACK_2_REGION,
	QFPROM_ANTI_ROLLBACK_3_REGION,
	QFPROM_ANTI_ROLLBACK_4_REGION,
	QFPROM_ANTI_ROLLBACK_5_REGION,
	QFPROM_ANTI_ROLLBACK_6_REGION,
	QFPROM_ANTI_ROLLBACK_7_REGION,
	QFPROM_ANTI_ROLLBACK_8_REGION,
	QFPROM_ANTI_ROLLBACK_9_REGION,
	QFPROM_ANTI_ROLLBACK_10_REGION,
	QFPROM_ANTI_ROLLBACK_11_REGION,
	QFPROM_ANTI_ROLLBACK_12_REGION,
	QFPROM_ANTI_ROLLBACK_13_REGION,
	QFPROM_PK_HASH_0_REGION,
	QFPROM_CALIBRATION_REGION,
	QFPROM_MEMORY_CONFIGURATION_REGION,
	QFPROM_QC_SPARE_20_REGION,
	QFPROM_QC_SPARE_21_REGION,
	QFPROM_OEM_IMAGE_ENCRYPTION_KEY_REGION,
	QFPROM_OEM_SECURE_BOOT_REGION,
	QFPROM_SEC_KEY_DERIVATION_KEY_REGION,
	QFPROM_BOOT_ROM_PATCH_REGION,
	QFPROM_IMAGE_ENCRYPTION_KEY_1_REGION,
	QFPROM_USER_KEY_DERIVATION_KEY_REGION,
	QFPROM_OEM_SPARE_28_REGION,
	QFPROM_OEM_SPARE_29_REGION,
	QFPROM_OEM_SPARE_30_REGION,
	QFPROM_OEM_SPARE_31_REGION,
	QFPROM_LAST_REGION_DUMMY,
};

enum qfprom_perm_bit_pos {
	LCM = 1,
	PRI_KEY_DERIVATION_KEY = 2,
	MRC_2_0 = 3,
	PTE = 4,
	READ_PERMISSION = 5,
	WRITE_PERMISSION = 6,
	FEC_ENABLES = 7,
	OEM_CONFIG = 8,
	FEATURE_CONFIG_M = 9,
	FEATURE_CONFIG_NM = 10,
	ANTI_ROLLBACK_1 = 11,
	ANTI_ROLLBACK_2 = 12,
	ANTI_ROLLBACK_3 = 13,
	ANTI_ROLLBACK_4 = 14,
	ANTI_ROLLBACK_5 = 15,
	ANTI_ROLLBACK_6 = 16,
	ANTI_ROLLBACK_7 = 17,
	ANTI_ROLLBACK_8 = 18,
	ANTI_ROLLBACK_9 = 19,
	ANTI_ROLLBACK_10 = 20,
	ANTI_ROLLBACK_11 = 21,
	ANTI_ROLLBACK_12 = 22,
	ANTI_ROLLBACK_13 = 23,
	PK_HASH_0 = 24,
	CALIBRATION = 25,
	MEMORY_CONFIGURATION = 26,
	QC_SPARE_20 = 27,
	QC_SPARE_21 = 28,
	OEM_IMAGE_ENCRYPTION_KEY = 29,
	OEM_SECURE_BOOT = 30,
	SEC_KEY_DERIVATION_KEY = 31,
	IMAGE_ENCRYPTION_KEY_1 = 1,  /* MSB bit 1 */
	USER_KEY_DERIVATION_KEY = 2, /* MSB bit 2 */
	OEM_SPARE_28 = 3,            /* MSB bit 3 */
	OEM_SPARE_29 = 4,            /* MSB bit 4 */
	OEM_SPARE_30 = 5,            /* MSB bit 5 */
	OEM_SPARE_31 = 6,            /* MSB bit 6 */
};

#define LCM_PERM_MASK				BIT(LCM)
#define PRI_KEY_DERIVATION_KEY_PERM_MASK	BIT(PRI_KEY_DERIVATION_KEY)
#define MRC_2_0_PERM_MASK			BIT(MRC_2_0)
#define PTE_PERM_MASK				BIT(PTE)
#define READ_PERMISSION_PERM_MASK		BIT(READ_PERMISSION)
#define WRITE_PERMISSION_PERM_MASK		BIT(WRITE_PERMISSION)
#define FEC_ENABLES_PERM_MASK			BIT(FEC_ENABLES)
#define OEM_CONFIG_PERM_MASK			BIT(OEM_CONFIG)
#define FEATURE_CONFIG_M_PERM_MASK		BIT(FEATURE_CONFIG_M)
#define FEATURE_CONFIG_NM_PERM_MASK		BIT(FEATURE_CONFIG_NM)
#define ANTI_ROLLBACK_1_PERM_MASK		BIT(ANTI_ROLLBACK_1)
#define ANTI_ROLLBACK_2_PERM_MASK		BIT(ANTI_ROLLBACK_2)
#define ANTI_ROLLBACK_3_PERM_MASK		BIT(ANTI_ROLLBACK_3)
#define ANTI_ROLLBACK_4_PERM_MASK		BIT(ANTI_ROLLBACK_4)
#define ANTI_ROLLBACK_5_PERM_MASK		BIT(ANTI_ROLLBACK_5)
#define ANTI_ROLLBACK_6_PERM_MASK		BIT(ANTI_ROLLBACK_6)
#define ANTI_ROLLBACK_7_PERM_MASK		BIT(ANTI_ROLLBACK_7)
#define ANTI_ROLLBACK_8_PERM_MASK		BIT(ANTI_ROLLBACK_8)
#define ANTI_ROLLBACK_9_PERM_MASK		BIT(ANTI_ROLLBACK_9)
#define ANTI_ROLLBACK_10_PERM_MASK		BIT(ANTI_ROLLBACK_10)
#define ANTI_ROLLBACK_11_PERM_MASK		BIT(ANTI_ROLLBACK_11)
#define ANTI_ROLLBACK_12_PERM_MASK		BIT(ANTI_ROLLBACK_12)
#define ANTI_ROLLBACK_13_PERM_MASK		BIT(ANTI_ROLLBACK_13)
#define PK_HASH_0_PERM_MASK			BIT(PK_HASH_0)
#define CALIBRATION_PERM_MASK			BIT(CALIBRATION)
#define MEMORY_CONFIGURATION_PERM_MASK		BIT(MEMORY_CONFIGURATION)
#define QC_SPARE_20_PERM_MASK			BIT(QC_SPARE_20)
#define QC_SPARE_21_PERM_MASK			BIT(QC_SPARE_21)
#define OEM_IMAGE_ENCRYPTION_KEY_PERM_MASK	BIT(OEM_IMAGE_ENCRYPTION_KEY)
#define OEM_SECURE_BOOT_PERM_MASK		BIT(OEM_SECURE_BOOT)
#define SEC_KEY_DERIVATION_KEY_PERM_MASK	BIT(SEC_KEY_DERIVATION_KEY)
#define IMAGE_ENCRYPTION_KEY_1_PERM_MASK	BIT(IMAGE_ENCRYPTION_KEY_1)
#define USER_KEY_DERIVATION_KEY_PERM_MASK	BIT(USER_KEY_DERIVATION_KEY)
#define OEM_SPARE_28_PERM_MASK			BIT(OEM_SPARE_28)
#define OEM_SPARE_29_PERM_MASK			BIT(OEM_SPARE_29)
#define OEM_SPARE_30_PERM_MASK			BIT(OEM_SPARE_30)
#define OEM_SPARE_31_PERM_MASK			BIT(OEM_SPARE_31)

#endif /* __QFPROM_TARGET_H__ */
