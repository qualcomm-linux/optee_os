/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qcom/chipinfo/chipinfo.h>
#include <drivers/qcom/platforminfo/platforminfodefs.h>
#include <drivers/qcom/smem/smem.h>
#include <initcall.h>

#include "chipinfo_internal.h"

static struct qti_chipinfo_ctxt chipinfo_ctxt;

uint32_t qti_chipinfo_get_chip_version(void)
{
	if (!chipinfo_ctxt.initialized)
		return CHIPINFO_VERSION_UNKNOWN;

	return chipinfo_ctxt.version;
}

enum qti_chipinfo_id qti_chipinfo_get_chip_id(void)
{
	if (!chipinfo_ctxt.initialized)
		return QTI_CHIPINFO_ID_UNKNOWN;

	return chipinfo_ctxt.chipinfo_id;
}

enum qti_chipinfo_family qti_chipinfo_get_chip_family(void)
{
	if (!chipinfo_ctxt.initialized)
		return QTI_CHIPINFO_FAMILY_UNKNOWN;

	return chipinfo_ctxt.family_id;
}

bool qti_chipinfo_is_part_disabled(enum qti_chipinfo_part part,
				   uint32_t part_idx)
{
	uint32_t i;

	if (!chipinfo_ctxt.initialized)
		return false;

	if (part == QTI_CHIPINFO_PART_UNKNOWN ||
	    (uint32_t)part >= QTI_CHIPINFO_NUM_PARTS)
		return false;

	if (part_idx == 0U) {
		/* Flat array reflects the overall part fuse state reported by BL2. */
		return chipinfo_ctxt.disabled_features[part] != 0U;
	}

	/* Search the per-instance table; assume present if absent or not found. */
	for (i = 0U; i < chipinfo_ctxt.num_part_info; i++) {
		const struct qti_platforminfo_part_info *entry =
			&chipinfo_ctxt.part_info[i];

		if ((uint32_t)entry->part == (uint32_t)part &&
		    (uint32_t)entry->instance == part_idx)
			return entry->disabled != 0U;
	}

	return false;
}

TEE_Result qti_chipinfo_init(void)
{
	struct qti_platforminfo_smem *smem;
	void *smem_ptr;
	size_t size;
	uint32_t fmt;
	uint32_t chip_id;
	uint32_t chip_family;
	TEE_Result res;

	res = qti_smem_init();
	if (res != TEE_SUCCESS && res != TEE_ERROR_BAD_STATE)
		return res;

	/* Access the socinfo SMEM item populated by the boot firmware. */
	res = qti_smem_lookup(QTI_SMEM_HOST_COMMON,
			      QTI_CHIPINFO_SMEM_ITEM_HW_SW_BUILD_ID,
			      QTI_SMEM_FLAG_NONE, &smem_ptr, &size);
	if (res != TEE_SUCCESS)
		return res;
	smem = smem_ptr;

	if (size < sizeof(uint32_t))
		return TEE_ERROR_BAD_FORMAT;

	/*
	 * Each SMEM format version defines a fixed set of fields; the boot
	 * firmware only populates fields up to the version it reports.
	 * Reading a field added by a later version than smem->format would
	 * read data the boot firmware never wrote. chip_id and chip_version
	 * are defined from format version 1; chip_family was added in
	 * format version 12.
	 */
	fmt = smem->format;

	if (fmt < QTI_PLATFORMINFO_FORMAT_V1)
		return TEE_ERROR_BAD_FORMAT;

	/*
	 * The format version only says which fields the layout defines; it
	 * does not prove the SMEM item is actually large enough to hold
	 * them. Validate size against the corresponding
	 * QTI_PLATFORMINFO_SMEM_SIZE_Vn constant before dereferencing, so a
	 * truncated SMEM item is rejected instead of read out of bounds.
	 */
	if (size < QTI_PLATFORMINFO_SMEM_SIZE_V1)
		return TEE_ERROR_BAD_FORMAT;

	/* Bound the raw values before casting into the driver enums. */
	chip_id = smem->chip_id;
	if (chip_id >= QTI_CHIPINFO_NUM_IDS)
		chip_id = QTI_CHIPINFO_ID_UNKNOWN;
	chipinfo_ctxt.chipinfo_id = (enum qti_chipinfo_id)chip_id;
	chipinfo_ctxt.version = smem->chip_version;

	if (fmt >= QTI_PLATFORMINFO_FORMAT_V12 &&
	    size >= QTI_PLATFORMINFO_SMEM_SIZE_V12) {
		chip_family = smem->chip_family;
		if (chip_family >= QTI_CHIPINFO_NUM_FAMILIES)
			chip_family = QTI_CHIPINFO_FAMILY_UNKNOWN;
		chipinfo_ctxt.family_id = (enum qti_chipinfo_family)chip_family;
	}

	/*
	 * Disabled-features array: offset from the SMEM base, one uint32_t
	 * per part, non-zero means disabled. Entries past num_parts stay
	 * at their zero-initialized default (present).
	 */
	if (fmt >= QTI_PLATFORMINFO_FORMAT_V14 &&
	    size >= QTI_PLATFORMINFO_SMEM_SIZE_V14) {
		uint32_t offset = smem->disabled_features_array_offset;
		uint32_t num = smem->num_parts;
		const uint32_t *features;
		uint32_t i;

		if (num > QTI_CHIPINFO_NUM_PARTS)
			num = QTI_CHIPINFO_NUM_PARTS;

		/* offset <= size first, so (size - offset) below cannot underflow. */
		if (offset != 0U && num != 0U && offset <= size &&
		    num <= (size - offset) / sizeof(uint32_t)) {
			features = (const uint32_t *)((uintptr_t)smem +
						      offset);
			for (i = 0U; i < num; i++)
				chipinfo_ctxt.disabled_features[i] =
					features[i];
		}
	}

	/* Per-instance disabled features table (format >= 23): point into SMEM, no copy. */
	if (fmt >= QTI_PLATFORMINFO_FORMAT_V23 &&
	    size >= QTI_PLATFORMINFO_SMEM_SIZE_V23) {
		uint32_t offset = smem->part_instances_offset;
		uint32_t num = smem->num_part_instances;

		/* offset <= size first, so (size - offset) below cannot underflow. */
		if (offset != 0U && num != 0U && offset <= size &&
		    num <= (size - offset) /
			   sizeof(struct qti_platforminfo_part_info)) {
			chipinfo_ctxt.part_info =
				(const struct qti_platforminfo_part_info *)
				((uintptr_t)smem + offset);
			chipinfo_ctxt.num_part_info = num;
		}
	}

	chipinfo_ctxt.initialized = true;

	return TEE_SUCCESS;
}

early_init(qti_chipinfo_init);
