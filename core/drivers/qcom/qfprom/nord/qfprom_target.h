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

/*
 * Nord's fuse controller is a TME sub-block (not standalone QFPROM macro).
 * Directly memory-mapped MMIO, no TME firmware call needed.
 * QFPROM_RAW_BASE/QFPROM_CORR_BASE are TME_FUSECONTROLLER_BASE-relative
 * (0x360c0000 + 0x0/+0x8000), not hoya's 0x0078xxxx range.
 */
#define QFPROM_RAW_BASE                          0x360c0000
#define QFPROM_CORR_BASE                         0x360c8000
#define QFPROM_SIZE                              0x8000

/*
 * SECURE_BOOT register: AUTH_EN, PK_HASH_IN_FUSE, USE_SERIAL_NUM bits.
 * Nord has single register (not per-segment like hoya).
 * Root-of-trust anchor in PK_HASH fuse rows when PK_HASH_IN_FUSE is blown.
 */
#define SECURE_BOOT_APPS_ADDR			(SECURITY_CONTROL_BASE + 0x0704)
#define SECURE_BOOT_AUTH_EN_BMSK		0x20
#define SECURE_BOOT_USE_SERIAL_NUM_BMSK		0x40
#define SECURE_BOOT_PK_HASH_IN_FUSE_BMSK	0x10

/*
 * OEM root-of-trust digest size: SHA-384 (48 bytes).
 * Matches secboot_chipset.h and secboot_hw_sha3rot.c definitions.
 */
#define QFPROM_ROOT_OF_TRUST_BYTE_SIZE		48

/*
 * PIL subsystem anti-rollback fuse layout: NOT YET AVAILABLE for nord.
 * Nord's per-subsystem layout differs from hoya's shared counter model;
 * the existing fuse-PTA API cannot support it without redesign.
 * CFG_QCOM_PAS_AUTH must stay off until this is resolved.
 */

/*
 * Device-identity sense registers (hardware shadow of fuse rows).
 * Nord: separate OEM_ID and MODEL_ID registers (vs hoya's packed word).
 * BMSK/SHFT pairs kept for source compatibility with hoya accessors.
 */
#define OEM_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0800)
#define OEM_ID_BMSK				0x0000ffff
#define OEM_ID_SHFT				0
#define MODEL_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0804)
#define MODEL_ID_BMSK				0x0000ffff
#define MODEL_ID_SHFT				0
#define JTAG_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0844)
#define JTAG_ID_AUTH_BMSK			0x0fffffff
#define JTAG_ID_AUTH_SHFT			0
/*
 * Serial number sense register (named CHIP_UNIQUE_ID_0/SERIAL_NUM in the
 * reference). 32-bit, full-word, matching hoya's field width; a second
 * 32-bit CHIP_UNIQUE_ID_1/CHIP_ID register exists alongside it but is not
 * part of the serial-number binding this driver's callers need.
 */
#define SERIAL_NUM_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0710)

/*
 * SOC hardware version lives in a TCSR register (not a fuse), same as
 * hoya. The metadata soc_vers binding compares against the family|device
 * field (bits 31:16).
 *
 * UNCONFIRMED FOR NORD: this is hoya's address, carried over as a
 * placeholder. Nord's TCSR HWIO register definitions were not found in the
 * accessible fuse-controller register header (TCSR is generated as a
 * separate IP block from the fuse controller, and no nord-specific TCSR
 * header was located during this pass). Do not enable CFG_QCOM_PAS_AUTH for
 * nord until this address is verified against a nord TCSR register spec -
 * an unverified address here would make the SOC_HW_VERSION binding check
 * silently read the wrong register.
 */
#define TCSR_SOC_HW_VERSION_ADDR		0x01FC8000
#define SOC_HW_VERSION_FAM_DEV_BMSK		0xffff0000
#define SOC_HW_VERSION_FAM_DEV_SHFT		16

/*
 * OEM_CONFIG2: EKU_ENFORCEMENT_EN bit (same position as hoya).
 * No per-segment hash-algorithm-select field; always SHA-384.
 */
#define OEM_CONFIG2_ADDR			(SECURITY_CONTROL_BASE + 0x0308)
#define EKU_ENFORCEMENT_EN_SHFT			30

#define SEGMENT_HASH_SELECT_SUPPORTED		0

/*
 * Multiple-root-certificate (MRC) fuse fields: NOT YET AVAILABLE for nord.
 * Nord's MRC_0/MRC_1 layout differs materially from hoya's model and includes
 * undefined fields (MRC_16_12, MRC_31_29, MRC_63_32). MRC/root-selection
 * support needs its own accessor design once these fields are understood.
 *
 * ROOT_CERT_TOTAL_NUM is at the same bit position as hoya/kodiak/lemans,
 * but in OEM_CONFIG5 (not OEM_CONFIG0) for consistency with other sense regs.
 */
#define OEM_CONFIG5_ADDR			(SECURITY_CONTROL_BASE + 0x0314)
#define ROOT_CERT_TOTAL_NUM_BMSK		0x00000700
#define ROOT_CERT_TOTAL_NUM_SHFT		8

/* OEM_CONFIG5 PIL_ANTI_ROLL_EN: gates the (not yet ported) PIL ARB fuses. */
#define PIL_ANTI_ROLLBACK_EN_BMSK		0x00000004

/*
 * OEM image encryption enable bit: not yet located for nord. Hoya's
 * IMAGE_ENCRYPTION_ENABLE lives in OEM_CONFIG0; no equivalent field was
 * found in nord's OEM_CONFIG0-OEM_CONFIG11 register definitions during this
 * pass. Left undefined; a caller needing this must locate the field before
 * relying on it.
 */

#endif /* __QFPROM_TARGET_H__ */
