/* SPDX-License-Identifier: BSD-2-Clause */
/*
 *  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef CHIPINFO_INTERNAL_H
#define CHIPINFO_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qcom/chipinfo/chipinfo.h>
#include <drivers/qcom/platforminfo/platforminfodefs.h>

/*
 * struct qti_chipinfo_ctxt - driver state.
 * @disabled_features: indexed by enum qti_chipinfo_part; non-zero means
 *                      disabled on this SKU.
 * @part_info:          per-instance table pointing into SMEM; NULL if
 *                      format < 23.
 */
struct qti_chipinfo_ctxt {
	bool initialized;
	enum qti_chipinfo_id chipinfo_id;
	uint32_t version;
	enum qti_chipinfo_family family_id;
	uint32_t disabled_features[QTI_CHIPINFO_NUM_PARTS];
	const struct qti_platforminfo_part_info *part_info;
	uint32_t num_part_info;
};

/*
 * QTI_CHIPINFO_SMEM_ITEM_HW_SW_BUILD_ID - SMEM item ID for the socinfo
 * item, This value is part of a fixed hardware/firmware ABI 
 * and must not be changed.
 */
#define QTI_CHIPINFO_SMEM_ITEM_HW_SW_BUILD_ID	137U

#endif /* CHIPINFO_INTERNAL_H */
