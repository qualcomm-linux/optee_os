/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_AUTH_H
#define __PAS_AUTH_H

#include <qcom_pas_priv.h>
#include <tee_internal_api.h>

/*
 * Firmware integrity backend for the PAS TA.
 *
 * The REE drives two commands, and the checks are deliberately split between
 * them:
 *
 *   INIT_IMAGE      - before the REE loads anything. Saves a TEE-private copy
 *                     of the image metadata and locates the per-segment hash
 *                     table inside it.
 *   AUTH_AND_RESET  - after the REE has loaded the segments, before the
 *                     peripheral leaves reset. Re-hashes each loaded segment
 *                     and compares it against the table.
 *
 * Hashing after the load and before reset release is what closes the window
 * in which the REE could alter the carveout behind the TEE's back.
 *
 * The hash table itself comes from REE-supplied metadata and is not
 * authenticated.
 *
 * TODO: signature authentication of the hash table (certificate chain,
 * signature, fuse-bound SW/HW binding and anti-rollback) will be added
 * incrementally, at INIT_IMAGE, before the REE loads any segment.
 */

#ifdef CFG_QCOM_PAS_AUTH
/*
 * pas_auth_save_metadata() - save a private copy of the INIT_IMAGE metadata
 * @s:      per-session context
 * @pt:     INIT_IMAGE invocation parameter types
 * @params: INIT_IMAGE invocation parameters; params[0].value.a is the
 *          peripheral pas_id, params[1].memref is the ELF header +
 *          program-header table + MBN hash segment
 *
 * Copies the metadata into TEE-private memory keyed by pas_id so the REE
 * cannot alter it between INIT_IMAGE and AUTH_AND_RESET.
 */
TEE_Result pas_auth_save_metadata(struct qcom_pas_session *s, uint32_t pt,
				  TEE_Param params[TEE_NUM_PARAMS]);

/*
 * pas_auth_authenticate() - locate the hash table in the saved metadata
 * @s:      per-session context
 * @pas_id: peripheral identifier the metadata belongs to
 *
 * Parses the MBN hash segment to find the per-segment hash table and marks
 * the slot ready for pas_auth_verify_reset(). Call after the matching
 * pas_auth_save_metadata() and the PTA INIT_IMAGE call.
 */
TEE_Result pas_auth_authenticate(struct qcom_pas_session *s, uint32_t pas_id);

/*
 * pas_auth_verify_reset() - re-verify loaded segments at AUTH_AND_RESET
 * @s:           per-session context
 * @pta_session: shared PAS PTA session handle
 * @pas_id:      peripheral identifier the metadata belongs to
 * @params:      AUTH_AND_RESET invocation parameters
 *
 * Packs the metadata and hash table into a single memref and hands it to
 * the PAS PTA's VERIFY_IMAGE command, which re-hashes each loaded firmware
 * segment against the per-segment hash table.
 */
TEE_Result pas_auth_verify_reset(struct qcom_pas_session *s,
				 TEE_TASessionHandle pta_session,
				 uint32_t pas_id,
				 TEE_Param params[TEE_NUM_PARAMS]);
#else
static inline TEE_Result
pas_auth_save_metadata(struct qcom_pas_session *s __unused,
		       uint32_t pt __unused,
		       TEE_Param params[TEE_NUM_PARAMS] __unused)
{
	return TEE_SUCCESS;
}

static inline TEE_Result
pas_auth_authenticate(struct qcom_pas_session *s __unused,
		      uint32_t pas_id __unused)
{
	return TEE_SUCCESS;
}

static inline TEE_Result
pas_auth_verify_reset(struct qcom_pas_session *s __unused,
		      TEE_TASessionHandle pta_session __unused,
		      uint32_t pas_id __unused,
		      TEE_Param params[TEE_NUM_PARAMS] __unused)
{
	return TEE_SUCCESS;
}
#endif /* CFG_QCOM_PAS_AUTH */

#endif /* __PAS_AUTH_H */
