/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_POLICY_H
#define __PAS_POLICY_H

#include <stdbool.h>
#include <stdint.h>
#include <tee_api_types.h>

/*
 * PAS image-binding policy.
 *
 * Mirrors the peripheral image loader's binding policy: each subsystem is
 * bound to a fixed software-id (SW_ID) so that a validly-signed image for
 * one peripheral cannot be loaded onto another, and each SW_ID has a
 * required signing authority.
 */

/* Signing authority required for an image. */
enum pas_sign_authority {
	PAS_OEM_SIGNED = 0,
	PAS_QTI_SIGNED,
	PAS_DOUBLE_SIGNED,
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
 * DOUBLE_SIGNED only for the documented hoya SW_ID list, OEM_SIGNED
 * otherwise.
 */
enum pas_sign_authority pas_policy_signer(uint32_t swid);

#endif /* __PAS_POLICY_H */
