/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_POLICY_H
#define __PAS_POLICY_H

#include <stdint.h>
#include <tee_api_types.h>

/*
 * PAS image-binding policy.
 *
 * Each subsystem is bound to a fixed software-id (SW_ID) so that a validly-
 * signed image for one peripheral cannot be loaded onto another, and each
 * SW_ID has a required signing authority.
 */

/*
 * Signing authority required for an image. pas_policy_signer() returns
 * PAS_OEM_SIGNED for every SW_ID this port supports; PAS_QTI_SIGNED and
 * PAS_DOUBLE_SIGNED are declared so callers can name every defined class,
 * even though QTI countersignature verification is not implemented here
 * and no supported peripheral requires it.
 */
enum pas_sign_authority {
	PAS_OEM_SIGNED = 0,
	PAS_QTI_SIGNED = 1,
	PAS_DOUBLE_SIGNED = 2,
};

/*
 * pas_policy_expected_swid() - expected SW_ID for a peripheral
 * @pas_id: unique remote-processor identifier (invoke params[0].a)
 * @swid:   expected SW_ID on success
 *
 * Returns TEE_ERROR_NOT_SUPPORTED if the pas_id has no known binding.
 */
TEE_Result pas_policy_expected_swid(uint32_t pas_id, uint32_t *swid);

/*
 * pas_policy_signer() - required signing authority for a SW_ID
 * @swid: image software id
 *
 * Returns PAS_OEM_SIGNED for every SW_ID this port supports.
 */
enum pas_sign_authority pas_policy_signer(uint32_t swid);

#endif /* __PAS_POLICY_H */
