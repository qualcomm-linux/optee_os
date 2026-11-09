/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef PLATFORMINFO_INTERNAL_H
#define PLATFORMINFO_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include <drivers/qcom/platforminfo/platforminfo.h>


/*
 * Private PlatformInfo driver context.
 *
 * Platform information is snapshotted from SMEM during initialization.
 * Consumers access this information through the public PlatformInfo APIs.
 */
struct platforminfo_ctxt {
	bool initialized;

	enum platforminfo_type platform;
	uint32_t version;
	uint32_t subtype;
	bool fusion;
	uint32_t oem_variant_id;
};

#endif /* PLATFORMINFO_INTERNAL_H */