/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef PLATFORMINFODEFS_H
#define PLATFORMINFODEFS_H

#include <stddef.h>
#include <stdint.h>

/*
 * Generate a platform version number.
 */
#define QTI_PLATFORMINFO_VERSION(major, minor) (((major) << 16) | (minor))

/*
 * SMEM format version numbers. Each version adds fields at the end of
 * struct qti_platforminfo_smem; only access fields whose version is <=
 * smem->format.
 *
 * QTI_PLATFORMINFO_SMEM_SIZE_Vn is the minimum SMEM item size needed to
 * safely read every field through that version. Check both format and
 * size before reading a version-gated field.
 */
#define QTI_PLATFORMINFO_FORMAT_V1	1U	/* chip_id, chip_version */
#define QTI_PLATFORMINFO_SMEM_SIZE_V1 \
	(offsetof(struct qti_platforminfo_smem, chip_version) + \
	 sizeof(uint32_t))

/* chip_family, raw_device_family/number */
#define QTI_PLATFORMINFO_FORMAT_V12	12U
#define QTI_PLATFORMINFO_SMEM_SIZE_V12 \
	(offsetof(struct qti_platforminfo_smem, raw_device_number) + \
	 sizeof(uint32_t))

/* num_parts, disabled_features_array_offset */
#define QTI_PLATFORMINFO_FORMAT_V14	14U
#define QTI_PLATFORMINFO_SMEM_SIZE_V14 \
	(offsetof(struct qti_platforminfo_smem, \
		  disabled_features_array_offset) + sizeof(uint32_t))

/* part_instances_offset, num_part_instances */
#define QTI_PLATFORMINFO_FORMAT_V23	23U
#define QTI_PLATFORMINFO_SMEM_SIZE_V23 \
	(offsetof(struct qti_platforminfo_smem, num_part_instances) + \
	 sizeof(uint32_t))

/*
 * struct qti_platforminfo_part_info - per-instance IP entry; array
 * located at qti_platforminfo_smem.part_instances_offset (format >= 23).
 */
struct qti_platforminfo_part_info {
	uint16_t part; /* enum qti_chipinfo_part value */
	uint8_t instance; /* 0-based instance index */
	uint8_t disabled; /* non-zero if this instance is fused off */
	uint32_t disabled_features; /* per-feature bitmask */
};

/*
 * Length of the build ID buffer in struct qti_platforminfo_smem.
 */
#define QTI_PLATFORMINFO_SMEM_BUILD_ID_LENGTH 32

/*
 * Length of the chip ID buffer in struct qti_platforminfo_smem.
 */
#define QTI_PLATFORMINFO_CHIP_NAME_MAX_LENGTH 32

/*
 * Maximum number of PMIC devices in struct qti_platforminfo_smem.
 */
#define QTI_PLATFORMINFO_SMEM_MAX_PMIC_DEVICES 3

/*
 * struct qti_platforminfo_pmic - PMIC information.
 */
struct qti_platforminfo_pmic {
	/* PMIC device model type, for Badger matches the revision id subtype */
	uint32_t pmic_model;
	uint32_t pmic_version; /* PMIC version, same format as Platform version */
};

/*
 * struct qti_platforminfo_smem - shared memory location used to store
 * platform, chip, build, and PMIC information.
 */
struct qti_platforminfo_smem {
	uint32_t format; /* Format of the structure. */
	uint32_t chip_id; /* Chip ID. */
	uint32_t chip_version; /* Chip version. */
	char build_id[QTI_PLATFORMINFO_SMEM_BUILD_ID_LENGTH]; /* Build ID. */
	uint32_t raw_chip_id; /* Raw chip ID. */
	uint32_t raw_chip_version; /* Raw chip version. */
	uint32_t platform_type; /* Platform type. */
	uint32_t platform_version; /* Platform version. */
	uint32_t fusion; /* TRUE if Fusion; FALSE otherwise. */
	uint32_t platform_subtype; /* Platform subtype. */
	struct qti_platforminfo_pmic
		pmic_info[QTI_PLATFORMINFO_SMEM_MAX_PMIC_DEVICES];
	/* DEPRECATED; may be incomplete. Use pmic_array_offset instead. */
	uint32_t foundry_id; /* Chip foundry ID. */
	uint32_t chip_serial; /* Chip serial number. */
	uint32_t num_pmics; /* Number of PMICs in array. */
	uint32_t pmic_array_offset; /* Offset to array of qti_platforminfo_pmic */
	uint32_t chip_family; /* Chip family. */
	uint32_t raw_device_family; /* Raw device family. */
	uint32_t raw_device_number; /* Raw device number. */
	uint32_t qfprom_chip_id; /* QFPROM Chip ID */
	char chip_id_str[QTI_PLATFORMINFO_CHIP_NAME_MAX_LENGTH]; /* Chip name */
	uint32_t num_clusters; /* Number of clusters for the disabled-cpus API */
	uint32_t cluster_array_offset; /* Offset to disabled-cluster array */
	uint32_t num_parts; /* Number of parts in disabled_features array */
	uint32_t disabled_features_array_offset; /* Offset to disabled-parts array */
	uint32_t modem_supported; /* 0 if not supported, nonzero if supported */
	uint32_t feature_code; /* Feature Code enum for this device */
	uint32_t p_code; /* PCode for this device */
	uint32_t part_name_map_offset; /* Offset of part name strings */
	uint32_t num_part_name_mappings; /* Number of part name strings. */
	uint32_t oem_variant_id; /* OEM Variant ID */
	uint32_t num_kvps; /* Number of KVPS */
	uint32_t kvps_offset; /* Offset of the KVPS */
	uint32_t num_functional_clusters; /* Clusters with >=1 enabled core */
	uint32_t boot_cluster; /* Boot cluster index */
	uint32_t boot_core; /* Boot core index */
	uint32_t raw_package_type; /* Raw package type. */
	uint32_t partial_features_array_offset; /* Offset to partial-features array */
	uint32_t cpu_cores_array_offset; /* Offset to per-cluster core-count array */
	uint32_t part_instances_offset; /* Offset to part-instance array */
	uint32_t num_part_instances; /* Length of the part-instance array */
};

#endif /* PLATFORMINFODEFS_H */
