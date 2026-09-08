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

/* Nord QFPROM base addresses (TME_FUSECONTROLLER_BASE-relative) */
#define QFPROM_RAW_BASE                          0x360c0000
#define QFPROM_CORR_BASE                         0x360c8000
#define QFPROM_SIZE                              0x8000

/* SECURE_BOOT register bits (single register, not per-segment) */
#define SECURE_BOOT_APPS_ADDR			(SECURITY_CONTROL_BASE + 0x0704)
#define SECURE_BOOT_AUTH_EN_BMSK		0x20
#define SECURE_BOOT_USE_SERIAL_NUM_BMSK		0x40
#define SECURE_BOOT_PK_HASH_IN_FUSE_BMSK	0x10

/* OEM root-of-trust digest size (SHA-384) */
#define QFPROM_ROOT_OF_TRUST_BYTE_SIZE		48

/* PIL anti-rollback fuse layout: NOT YET AVAILABLE for nord */
/* Device-identity sense registers (separate OEM_ID/MODEL_ID, not packed) */
#define OEM_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0800)
#define OEM_ID_BMSK				0x0000ffff
#define OEM_ID_SHFT				0
#define MODEL_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0804)
#define MODEL_ID_BMSK				0x0000ffff
#define MODEL_ID_SHFT				0
#define JTAG_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0844)
#define JTAG_ID_AUTH_BMSK			0x0fffffff
#define JTAG_ID_AUTH_SHFT			0
/* Serial number sense register (CHIP_UNIQUE_ID_0) */
#define SERIAL_NUM_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0710)

/* TCSR SOC_HW_VERSION: UNCONFIRMED FOR NORD (verify before enabling PAS_AUTH) */
#define TCSR_SOC_HW_VERSION_ADDR		0x01FC8000
#define SOC_HW_VERSION_FAM_DEV_BMSK		0xffff0000
#define SOC_HW_VERSION_FAM_DEV_SHFT		16

/* OEM_CONFIG2: EKU_ENFORCEMENT_EN bit (always SHA-384, no per-segment select) */
#define OEM_CONFIG2_ADDR			(SECURITY_CONTROL_BASE + 0x0308)
#define EKU_ENFORCEMENT_EN_SHFT			30

#define SEGMENT_HASH_SELECT_SUPPORTED		0

/* MRC fuse fields: NOT YET AVAILABLE for nord (layout differs from hoya) */
#define OEM_CONFIG5_ADDR			(SECURITY_CONTROL_BASE + 0x0314)
#define ROOT_CERT_TOTAL_NUM_BMSK		0x00000700
#define ROOT_CERT_TOTAL_NUM_SHFT		8

/* OEM_CONFIG5 PIL_ANTI_ROLL_EN: gates the (not yet ported) PIL ARB fuses. */
#define PIL_ANTI_ROLLBACK_EN_BMSK		0x00000004

/* OEM image encryption enable bit: NOT YET LOCATED for nord */
#endif /* __QFPROM_TARGET_H__ */
