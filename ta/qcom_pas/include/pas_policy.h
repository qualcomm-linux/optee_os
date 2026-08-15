/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_POLICY_H
#define __PAS_POLICY_H

#include <stdint.h>
#include <tee_api_types.h>

enum pas_sign_authority {
	PAS_OEM_SIGNED = 0,
	PAS_QTI_SIGNED = 1,
	PAS_DOUBLE_SIGNED = 2,
};

TEE_Result pas_policy_expected_swid(uint32_t pas_id, uint32_t *swid);

enum pas_sign_authority pas_policy_signer(uint32_t swid);

#endif /* __PAS_POLICY_H */
