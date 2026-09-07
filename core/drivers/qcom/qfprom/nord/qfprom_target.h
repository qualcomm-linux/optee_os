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
 * Nord's fuse controller is a TME (Trust Management Engine) sub-block
 * rather than a standalone always-on QFPROM macro (confirmed via IPCatalog:
 * both QFPROM_RAW and QFPROM_CORR are owned by the tme_fusecontroller
 * component). The register blocks are still directly memory-mapped and
 * readable via ordinary MMIO from the AP, matching how qfprom_read_row()
 * already accesses QFPROM_RAW_BASE/QFPROM_CORR_BASE on hoya - no TME
 * firmware call (TMECOMM) is required for these reads.
 *
 * QFPROM_RAW_BASE/QFPROM_CORR_BASE below are TME_FUSECONTROLLER_BASE-relative
 * (0x360c0000 + 0x0/+0x8000), not the 0x0078xxxx range hoya uses; the two
 * platforms' fuse controllers sit at different physical addresses.
 */
#define QFPROM_RAW_BASE                          0x360c0000
#define QFPROM_CORR_BASE                         0x360c8000
#define QFPROM_SIZE                              0x8000

/*
 * SECURE_BOOT secure-control register holding the AUTH_EN, PK_HASH_IN_FUSE
 * and USE_SERIAL_NUM bits, plus ROM_PK_HASH_INDEX (which root-of-trust table
 * entry the ROM selects when PK_HASH_IN_FUSE is not blown). Authentication
 * is required when AUTH_EN is blown; the root-of-trust anchor lives in the
 * PK_HASH fuse rows below rather than the ROM table when PK_HASH_IN_FUSE is
 * blown.
 *
 * Unlike hoya's per-code-segment SECURE_BOOTn array, nord has a single
 * SECURE_BOOT register (no code-segment index) - confirmed by
 * secfuses/inc/nord/SecHWIO.h, whose secboot_read_secure_bootn_*() helpers
 * all read this one address regardless of the code segment argument.
 */
#define SECURE_BOOT_APPS_ADDR			(SECURITY_CONTROL_BASE + 0x0704)
#define SECURE_BOOT_AUTH_EN_BMSK		0x20
#define SECURE_BOOT_USE_SERIAL_NUM_BMSK		0x40
#define SECURE_BOOT_PK_HASH_IN_FUSE_BMSK	0x10

/*
 * Size of the OEM root-of-trust digest compared during authentication.
 * SHA-384 (48 bytes), matching secboot_chipset.h's
 * SECBOOT_OTP_ROOT_OF_TRUST_BYTE_SIZE definition for this platform and
 * secboot_hw_sha3rot.c's SECBOOT_HASH_DIGEST_SIZE_SHA384 comparison length -
 * both confirmed against the reference root-of-trust read/compare call
 * despite the underlying PK_HASH fuse rows (below) spanning more bits than
 * this.
 */
#define QFPROM_ROOT_OF_TRUST_BYTE_SIZE		48

/*
 * PIL subsystem anti-rollback fuse layout: NOT YET AVAILABLE for nord.
 *
 * Unlike hoya's single shared PIL_SUBSYSTEM0/1 counter, nord's reference
 * anti-rollback implementation (tzbsp_arb_config.c) uses a separate
 * QFPROM_CORR/RAW_ANTIROLLBACK_ROWn_LSB/MSB fuse-row pair per subsystem
 * type (ADSP, ADSP1, ADSP2, CDSP, GVM, HCONFIG, camera, IPA, GPU microcode,
 * OEM VM, ...), each gated by a common PIL_ANTI_ROLL_EN bit in OEM_CONFIG5.
 * This has no single PIL_ARB_LSB/MSB pair to port faithfully, and the
 * existing fuse-PTA/pas_fuse.c API (one PTA_QCOM_FUSE_GET/BLOW_PIL_ROLLBACK_
 * VERSION call, no per-subsystem selector) would need to change shape to
 * support it. Left undefined rather than defining a scope-narrowed
 * approximation; CFG_QCOM_PAS_AUTH stays off for this platform until this
 * is resolved (see core/arch/arm/plat-qcom/wildcat/nord/target.mk).
 */

/*
 * Device-identity sense registers (hardware shadow of the underlying
 * fuse rows), read via tzbsp_fusecontroller_hwio.h/SecHWIO.h in the
 * reference. Used to bind signed image metadata to this device.
 *
 * Unlike hoya's single OEM_ID_SENSE_ADDR word packing both OEM_ID (bits
 * 31:16) and MODEL_ID (bits 15:0), nord exposes OEM_ID and MODEL_ID
 * (named OEM_HW_ID/OEM_PRODUCT_ID in the reference) as two separate
 * 16-bit registers - each BMSK/SHFT pair below is kept for source
 * compatibility with hoya's accessor shape even though nord's fields do
 * not need a shift.
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
 * OEM_CONFIG2 fuse register, holding the EKU_ENFORCEMENT_EN bit at the same
 * bit position as hoya. Nord's OEM_CONFIG2 does not carry a per-segment
 * hash-algorithm-select field the way lemans's does; MBN v6 segment hashing
 * on this platform always uses SHA-384, matching kodiak's
 * SEGMENT_HASH_SELECT_SUPPORTED=0 behavior (confirmed absent: no
 * SEGMENT_HASH_FUNCTION_SELECT field exists in nord's OEM_CONFIG2 register
 * definition).
 */
#define OEM_CONFIG2_ADDR			(SECURITY_CONTROL_BASE + 0x0308)
#define EKU_ENFORCEMENT_EN_SHFT			30

#define SEGMENT_HASH_SELECT_SUPPORTED		0

/*
 * Multiple-root-certificate (MRC) fuse fields. Nord's MRC_0/MRC_1 register
 * pair (SW_RANGE4 offsets 0x730/0x734) has a materially richer layout than
 * hoya's single-word 4-bit activation/4-bit revocation model: MRC_0 packs
 * QC_ROOT_CERT_ACTIVATION_LIST (bits 5:0), QC_ROOT_CERT_REVOCATION_LIST
 * (bits 11:6), a 5-bit MRC_16_12 field, OEM_ROOT_CERT_ACTIVATION_LIST (bits
 * 22:17), OEM_ROOT_CERT_REVOCATION_LIST (bits 28:23) and a 3-bit MRC_31_29
 * field, with MRC_1 holding MRC_63_32. There is no single field matching
 * hoya's ROOT_CERT_TOTAL_NUM/MRC_ACTIVATION_LIST/MRC_REVOCATION_LIST shape
 * with 4-bit lists, and no evidence was found resolving what MRC_16_12/
 * MRC_31_29/MRC_63_32 represent. Left undefined rather than mapping onto
 * hoya's narrower model; MRC/root-selection support for this platform needs
 * its own accessor design once these fields are understood, not a
 * find-and-replace of hoya's macros.
 *
 * ROOT_CERT_TOTAL_NUM (number of provisioned roots - 1) is present at the
 * same bit position hoya's kodiak/lemans use, but in the newer OEM_CONFIG5
 * sense register rather than OEM_CONFIG0 - both an OEM_CONFIG5 location and
 * a legacy QFPROM_RAW/CORR_OEM_CONFIG_ROW2_MSB location exist in the
 * reference register header with identical bit position; OEM_CONFIG5 is
 * used here for consistency with every other sense-register field above.
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
