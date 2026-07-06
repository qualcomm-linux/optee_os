// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <config.h>
#include <pas_hashseg.h>
#include <pta_qcom_pas.h>
#include <string.h>
#include <ta_qcom_pas.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <types_ext.h>
#include <utee_defines.h>
#include <util.h>

/*
 * Per-session context. The firmware metadata is saved at INIT_IMAGE inside a
 * TEE-private copy so the REE cannot alter it between INIT_IMAGE and
 * AUTH_AND_RESET. Keeping it here (rather than as a TA global) ensures two
 * concurrent sessions cannot observe each other's metadata.
 *
 * The kernel qcom_pas_tee driver opens one TEE session shared by every
 * peripheral, and DSPs load concurrently. Metadata must therefore be keyed
 * by pas_id to prevent one DSP's INIT_IMAGE overwriting another's slot
 * before its AUTH_AND_RESET runs. lemans/kodiak expose at most 7 PAS
 * subsystems; size the table with headroom.
 */
#define PAS_MD_SLOTS	8U

struct pas_md_slot {
	void *md;
	size_t md_size;
	uint32_t pas_id;
	bool used;
	struct pas_hashseg hs;
	bool authenticated;
};

struct qcom_pas_session {
	struct pas_md_slot md[PAS_MD_SLOTS];
};

static struct pas_md_slot *find_md_slot(struct qcom_pas_session *s,
					uint32_t pas_id)
{
	size_t i = 0;

	for (i = 0; i < PAS_MD_SLOTS; i++) {
		if (s->md[i].used && s->md[i].pas_id == pas_id)
			return &s->md[i];
	}

	return NULL;
}

/*
 * The PTA session is shared across all TA sessions: it wraps the core-side
 * PAS driver and does not carry per-image state. refcount tracks when to open
 * and close it.
 */
static size_t session_refcount;
static TEE_TASessionHandle pta_session;

/*
 * Save a private copy of the INIT_IMAGE metadata so it cannot be modified
 * by the REE before AUTH_AND_RESET. Keyed by pas_id so concurrent loads of
 * different peripherals on the shared session do not clobber each other.
 */
static TEE_Result save_metadata(struct qcom_pas_session *s, uint32_t pt,
				TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	struct pas_md_slot *slot = NULL;
	uint32_t pas_id = 0;
	void *copy = NULL;
	size_t size = 0;
	size_t i = 0;

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	pas_id = params[0].value.a;

	/* Reuse the existing slot for this pas_id, else take a free one. */
	slot = find_md_slot(s, pas_id);
	if (!slot) {
		for (i = 0; i < PAS_MD_SLOTS; i++) {
			if (!s->md[i].used) {
				slot = &s->md[i];
				break;
			}
		}
	}
	if (!slot) {
		EMSG("PAS auth: no free md slot (pas_id=%"PRIu32")", pas_id);
		return TEE_ERROR_OUT_OF_MEMORY;
	}

	size = params[1].memref.size;
	if (size) {
		copy = TEE_Malloc(size, TEE_MALLOC_FILL_ZERO);
		if (!copy)
			return TEE_ERROR_OUT_OF_MEMORY;
		memcpy(copy, params[1].memref.buffer, size);
	}

	TEE_Free(slot->md);
	slot->md = copy;
	slot->md_size = size;
	slot->pas_id = pas_id;
	slot->used = true;
	slot->authenticated = false;
	memset(&slot->hs, 0, sizeof(slot->hs));

	return TEE_SUCCESS;
}

#ifdef CFG_QCOM_PAS_HASH_VERIFY
/*
 * Pack [metadata | hash table] and hand it to the PAS PTA, which maps the
 * loaded firmware carveout and re-hashes each segment against the table.
 */
static TEE_Result verify_integrity(struct qcom_pas_session *s, uint32_t pas_id,
				   const struct pas_hashseg *hs,
				   TEE_Param params[TEE_NUM_PARAMS])
{
	struct pas_md_slot *slot = find_md_slot(s, pas_id);
	TEE_Param vp[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	size_t combined_size = 0;
	uint8_t *combined = NULL;
	uint32_t pt = 0;

	if (!slot || !slot->md || !slot->md_size) {
		EMSG("PAS auth: no metadata for pas_id=%"PRIu32, pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	if (ADD_OVERFLOW(slot->md_size, hs->hash_table_size, &combined_size))
		return TEE_ERROR_OVERFLOW;

	combined = TEE_Malloc(combined_size, TEE_MALLOC_FILL_ZERO);
	if (!combined)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(combined, slot->md, slot->md_size);
	TEE_MemMove(combined + slot->md_size, hs->hash_table,
		    hs->hash_table_size);

	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
			     TEE_PARAM_TYPE_VALUE_INPUT,
			     TEE_PARAM_TYPE_MEMREF_INPUT,
			     TEE_PARAM_TYPE_VALUE_INPUT);

	vp[0].value.a = params[0].value.a;
	vp[0].value.b = params[0].value.b;
	vp[1].value.a = params[1].value.a;
	vp[1].value.b = params[1].value.b;
	vp[2].memref.buffer = combined;
	vp[2].memref.size = combined_size;
	vp[3].value.a = hs->hash_size;
	vp[3].value.b = slot->md_size; /* ht offset in combined buf */

	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_PAS_VERIFY_IMAGE, pt, vp, NULL);

	TEE_Free(combined);
	return res;
}

/*
 * Parse the INIT_IMAGE metadata into the per-segment hash table and cache it
 * in the slot so verify_segments() can re-check the loaded segments at
 * AUTH_AND_RESET without re-parsing.
 */
static TEE_Result authenticate_metadata(struct qcom_pas_session *s,
					uint32_t pas_id)
{
	struct pas_md_slot *slot = find_md_slot(s, pas_id);
	TEE_Result res = TEE_ERROR_GENERIC;

	if (!slot) {
		EMSG("PAS auth: no metadata pas_id=%"PRIu32
		     " (call INIT_IMAGE first)", pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	res = pas_hashseg_parse(slot->md, slot->md_size, TEE_SHA384_HASH_SIZE,
				&slot->hs);
	if (res) {
		EMSG("PAS auth: pas_hashseg_parse failed: %#"PRIx32, res);
		return res;
	}

	slot->authenticated = true;

	return TEE_SUCCESS;
}

/*
 * Re-verify the per-segment hashes of firmware the REE has now loaded into
 * its carveout, against the hash table parsed at INIT_IMAGE.
 */
static TEE_Result verify_segments(struct qcom_pas_session *s, uint32_t pas_id,
				  TEE_Param params[TEE_NUM_PARAMS])
{
	struct pas_md_slot *slot = find_md_slot(s, pas_id);

	if (!slot || !slot->authenticated) {
		EMSG("PAS auth: pas_id=%"PRIu32
		     " not authenticated (call INIT_IMAGE first)", pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	return verify_integrity(s, pas_id, &slot->hs, params);
}
#endif /* CFG_QCOM_PAS_HASH_VERIFY */

static TEE_Result qcom_pas_init_image(struct qcom_pas_session *s, uint32_t pt,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;

	if (IS_ENABLED(CFG_QCOM_PAS_HASH_VERIFY)) {
		res = save_metadata(s, pt, params);
		if (res != TEE_SUCCESS)
			return res;
	}

	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_PAS_INIT_IMAGE, pt, params, NULL);
	if (res)
		return res;

#ifdef CFG_QCOM_PAS_HASH_VERIFY
	res = authenticate_metadata(s, params[0].value.a);
	if (res)
		EMSG("PAS firmware auth failed: %#"PRIx32, res);
#endif

	return res;
}

static TEE_Result qcom_pas_auth_and_reset(struct qcom_pas_session *s,
					  uint32_t pt,
					  TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						TEE_PARAM_TYPE_VALUE_INPUT,
						TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE);

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

#ifdef CFG_QCOM_PAS_HASH_VERIFY
	{
		TEE_Result res = verify_segments(s, params[0].value.a, params);

		if (res) {
			EMSG("PAS auth: segment verification failed: %#"PRIx32,
			     res);
			return res;
		}
	}
#else
	(void)s;
#endif

	return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
				   PTA_QCOM_PAS_AUTH_AND_RESET,
				   pt, params, NULL);
}

TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t pt,
				    TEE_Param params[TEE_NUM_PARAMS],
				    void **sess_ctx)
{
	static const TEE_UUID uuid = PTA_QCOM_PAS_UUID;
	TEE_PropSetHandle h = TEE_HANDLE_NULL;
	struct qcom_pas_session *s = NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	TEE_Identity id = { };

	res = TEE_AllocatePropertyEnumerator(&h);
	if (res != TEE_SUCCESS)
		goto error;

	TEE_StartPropertyEnumerator(h, TEE_PROPSET_CURRENT_CLIENT);

	res = TEE_GetPropertyAsIdentity(h, NULL, &id);
	if (res != TEE_SUCCESS)
		goto error;

	if (id.login != TEE_LOGIN_REE_KERNEL) {
		res = TEE_ERROR_ACCESS_DENIED;
		goto error;
	}

	s = TEE_Malloc(sizeof(*s), TEE_MALLOC_FILL_ZERO);
	if (!s) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto error;
	}

	if (!session_refcount) {
		res = TEE_OpenTASession(&uuid, TEE_TIMEOUT_INFINITE, pt, params,
					&pta_session, NULL);
		if (res != TEE_SUCCESS)
			goto error_free;
	}

	session_refcount++;
	*sess_ctx = s;
	res = TEE_SUCCESS;
	goto out;

error_free:
	TEE_Free(s);
error:
	*sess_ctx = NULL;
out:
	if (h)
		TEE_FreePropertyEnumerator(h);

	return res;
}

void TA_CloseSessionEntryPoint(void *sess_ctx)
{
	struct qcom_pas_session *s = sess_ctx;

	if (s) {
		size_t i = 0;

		for (i = 0; i < PAS_MD_SLOTS; i++)
			TEE_Free(s->md[i].md);
		TEE_Free(s);
	}

	session_refcount--;

	if (!session_refcount)
		TEE_CloseTASession(pta_session);
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
				      uint32_t pt,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	struct qcom_pas_session *s = sess_ctx;

	switch (cmd_id) {
	case TA_QCOM_PAS_IS_SUPPORTED:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_IS_SUPPORTED,
					   pt, params, NULL);
	case TA_QCOM_PAS_CAPABILITIES:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_CAPABILITIES,
					   pt, params, NULL);
	case TA_QCOM_PAS_INIT_IMAGE:
		return qcom_pas_init_image(s, pt, params);
	case TA_QCOM_PAS_MEM_SETUP:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_MEM_SETUP,
					   pt, params, NULL);
	case TA_QCOM_PAS_GET_RESOURCE_TABLE:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_GET_RESOURCE_TABLE,
					   pt, params, NULL);
	case TA_QCOM_PAS_AUTH_AND_RESET:
		return qcom_pas_auth_and_reset(s, pt, params);
	case TA_QCOM_PAS_SET_REMOTE_STATE:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_SET_REMOTE_STATE,
					   pt, params, NULL);
	case TA_QCOM_PAS_SHUTDOWN:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_SHUTDOWN,
					   pt, params, NULL);
	default:
		return TEE_ERROR_NOT_IMPLEMENTED;
	}
}
