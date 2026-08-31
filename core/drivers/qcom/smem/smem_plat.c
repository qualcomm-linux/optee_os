// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * Qualcomm Shared Memory (SMEM) - TFA platform integration
 *
 */
#include "drivers/qcom/smem/smem.h"
#include "smem_plat.h"

/*
 * qti_smem_plat_init() - Platform entry point for SMEM initialization.
 *
 * Return: TEE_SUCCESS on success, TEE_ERROR_* on failure.
 */
TEE_Result qti_smem_plat_init(struct qti_smem_plat_info *plat_info)
{
	TEE_Result ret;

	if (!plat_info)
		return TEE_ERROR_BAD_PARAMETERS;

	ret = qti_smem_host_id(QTI_SMEM_PROC_TZ, 0, 0, 0,
			       &plat_info->local_host);

	if (ret != TEE_SUCCESS)
		return ret;

	plat_info->max_items = 0xFFFF;
	plat_info->smem_size = SMEM_SIZE;

	return TEE_SUCCESS;
}
