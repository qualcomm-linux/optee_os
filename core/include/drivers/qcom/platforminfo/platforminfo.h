/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef PLATFORMINFO_H
#define PLATFORMINFO_H

#include <stdbool.h>
#include <stdint.h>
#include <tee_api_types.h>

/*
 * Platform version layout:
 *
 *   bits 31:16 - major version
 *   bits 15:0  - minor version
 */
#define PLATFORMINFO_VERSION(major, minor) \
	(((uint32_t)(major) << 16) | (uint32_t)(minor))

#define PLATFORMINFO_VERSION_UNKNOWN	0U
#define PLATFORMINFO_OEM_VARIANT_UNKNOWN	0U

/*
 * Values are part of the SMEM ABI and must not be reordered.
 */
enum platforminfo_type {
	PLATFORMINFO_TYPE_UNKNOWN = 0x00,
	PLATFORMINFO_TYPE_SURF = 0x01,
	PLATFORMINFO_TYPE_FFA = 0x02,
	PLATFORMINFO_TYPE_FLUID = 0x03,
	PLATFORMINFO_TYPE_FUSION = 0x04,
	PLATFORMINFO_TYPE_OEM = 0x05,
	PLATFORMINFO_TYPE_QT = 0x06,

	PLATFORMINFO_TYPE_CDP = PLATFORMINFO_TYPE_SURF,

	PLATFORMINFO_TYPE_MTP = 0x08,
	PLATFORMINFO_TYPE_MTP_MDM = PLATFORMINFO_TYPE_MTP,
	PLATFORMINFO_TYPE_MTP_MSM = PLATFORMINFO_TYPE_MTP,

	PLATFORMINFO_TYPE_LIQUID = 0x09,
	PLATFORMINFO_TYPE_DRAGONBOARD = 0x0A,
	PLATFORMINFO_TYPE_QRD = 0x0B,
	PLATFORMINFO_TYPE_EVB = 0x0C,
	PLATFORMINFO_TYPE_HRD = 0x0D,
	PLATFORMINFO_TYPE_DTV = 0x0E,
	PLATFORMINFO_TYPE_RUMI = 0x0F,
	PLATFORMINFO_TYPE_VIRTIO = 0x10,
	PLATFORMINFO_TYPE_GOBI = 0x11,
	PLATFORMINFO_TYPE_CBH = 0x12,
	PLATFORMINFO_TYPE_BTS = 0x13,
	PLATFORMINFO_TYPE_XPM = 0x14,
	PLATFORMINFO_TYPE_RCM = 0x15,
	PLATFORMINFO_TYPE_DMA = 0x16,
	PLATFORMINFO_TYPE_STP = 0x17,
	PLATFORMINFO_TYPE_SBC = 0x18,
	PLATFORMINFO_TYPE_ADP = 0x19,
	PLATFORMINFO_TYPE_CHI = 0x1A,
	PLATFORMINFO_TYPE_SDP = 0x1B,
	PLATFORMINFO_TYPE_RRP = 0x1C,
	PLATFORMINFO_TYPE_CLS = 0x1D,
	PLATFORMINFO_TYPE_TTP = 0x1E,
	PLATFORMINFO_TYPE_HDK = 0x1F,
	PLATFORMINFO_TYPE_IOT = 0x20,
	PLATFORMINFO_TYPE_ATP = 0x21,
	PLATFORMINFO_TYPE_IDP = 0x22,
	PLATFORMINFO_TYPE_AEDK = 0x23,
	PLATFORMINFO_TYPE_WDP = 0x24,
	PLATFORMINFO_TYPE_QAM = 0x25,
	PLATFORMINFO_TYPE_QXR = 0x26,
	PLATFORMINFO_TYPE_X100 = 0x27,
	PLATFORMINFO_TYPE_CRD = 0x28,
	PLATFORMINFO_TYPE_QQVP = 0x29,
	PLATFORMINFO_TYPE_DCP = 0x2A,
	PLATFORMINFO_TYPE_QCB = 0x2B,
	PLATFORMINFO_TYPE_QAR = 0x2C,
	PLATFORMINFO_TYPE_WRD = 0x2D,
	PLATFORMINFO_TYPE_COME = 0x2E,
	PLATFORMINFO_TYPE_EVK = 0x2F,
	PLATFORMINFO_TYPE_TDP = 0x30,
	PLATFORMINFO_TYPE_OMTP = 0x31,
	PLATFORMINFO_TYPE_ITPS = 0x32,
	PLATFORMINFO_TYPE_ITP = 0x33,
	PLATFORMINFO_TYPE_EITP = 0x34,

	PLATFORMINFO_NUM_TYPES,
	PLATFORMINFO_TYPE_32BITS = 0x7FFFFFFF
};

struct platforminfo_info {
	enum platforminfo_type platform;
	uint32_t version;
	uint32_t subtype;
	bool fusion;
	uint32_t oem_variant_id;
};

/*
 * Initialize the PlatformInfo driver from SMEM_HW_SW_BUILD_ID.
 */
TEE_Result qti_platforminfo_init(void);

/*
 * Return the current platform type.
 *
 * Returns PLATFORMINFO_TYPE_UNKNOWN if the driver has not initialized.
 */
enum platforminfo_type platforminfo_get_platform(void);

/*
 * Copy the complete platform information into @info.
 */
TEE_Result platforminfo_get_platform_info(struct platforminfo_info *info);

/*
 * Return true when the platform is a Fusion variant.
 */
bool platforminfo_is_fusion(void);

/*
 * Return the OEM variant ID.
 *
 * Returns PLATFORMINFO_OEM_VARIANT_UNKNOWN when unavailable.
 */
uint32_t platforminfo_get_oem_variant(void);

#endif /* PLATFORMINFO_H */