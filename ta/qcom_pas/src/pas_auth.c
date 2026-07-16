// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * Firmware integrity backend for the PAS TA. Built under
 * CFG_QCOM_PAS_AUTH; see pas_auth.h for the INIT_IMAGE/AUTH_AND_RESET flow.
 */

#include <pas_auth.h>
#include <pas_mbn_parser.h>
#include <pta_qcom_pas.h>
#include <qcom_pas_priv.h>
#include <string.h>
#include <tee_internal_api.h>
#include <utee_defines.h>

static struct pas_md_slot *get_meta_data_slot(struct qcom_pas_session *s,
					      uint32_t pas_id)
{
	size_t i = 0;

	for (i = 0; i < PAS_MD_SLOTS; i++) {
		if (s->slots[i].used && s->slots[i].pas_id == pas_id)
			return &s->slots[i];
	}

	return NULL;
}

TEE_Result pas_auth_save_metadata(struct qcom_pas_session *s, uint32_t pt,
				  TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	struct pas_md_slot *slot = NULL;
	uint32_t pas_id = 0;
	void *meta_data_copy = NULL;
	size_t size = 0;
	size_t i = 0;

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	pas_id = params[0].value.a;

	/* Reuse the existing slot for this pas_id, else take a free one. */
	slot = get_meta_data_slot(s, pas_id);
	if (!slot) {
		for (i = 0; i < PAS_MD_SLOTS; i++) {
			if (!s->slots[i].used) {
				slot = &s->slots[i];
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
		meta_data_copy = TEE_Malloc(size, TEE_MALLOC_FILL_ZERO);
		if (!meta_data_copy)
			return TEE_ERROR_OUT_OF_MEMORY;
		memcpy(meta_data_copy, params[1].memref.buffer, size);
	}

	TEE_Free(slot->meta_data);
	slot->meta_data = meta_data_copy;
	slot->meta_data_size = size;
	slot->pas_id = pas_id;
	slot->used = true;
	slot->hash_table_valid = false;
	memset(&slot->mbn, 0, sizeof(slot->mbn));

	return TEE_SUCCESS;
}

TEE_Result pas_auth_authenticate(struct qcom_pas_session *s, uint32_t pas_id)
{
	struct pas_md_slot *slot = get_meta_data_slot(s, pas_id);
	TEE_Result res = TEE_ERROR_GENERIC;

	if (!slot) {
		EMSG("PAS auth: no metadata for pas_id=%"PRIu32
		     " (call INIT_IMAGE first)", pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	res = pas_mbn_parse(slot->meta_data, slot->meta_data_size,
			    TEE_SHA384_HASH_SIZE, &slot->mbn);
	if (res) {
		EMSG("PAS auth: MBN parse failed: %#"PRIx32, res);
		return res;
	}

	slot->hash_table_valid = true;

	return TEE_SUCCESS;
}

TEE_Result pas_auth_verify_reset(struct qcom_pas_session *s,
				 TEE_TASessionHandle pta_session,
				 uint32_t pas_id,
				 TEE_Param params[TEE_NUM_PARAMS])
{
	struct pas_md_slot *slot = get_meta_data_slot(s, pas_id);
	TEE_Param vp[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	size_t buffer_size = 0;
	uint8_t *buffer = NULL;
	uint32_t pt = 0;

	if (!slot || !slot->hash_table_valid) {
		EMSG("PAS auth: pas_id=%"PRIu32
		     " has no hash table (call INIT_IMAGE first)", pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	if (ADD_OVERFLOW(slot->meta_data_size, slot->mbn.hash_table_size,
			 &buffer_size))
		return TEE_ERROR_OVERFLOW;

	buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
	if (!buffer)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(buffer, slot->meta_data, slot->meta_data_size);
	TEE_MemMove(buffer + slot->meta_data_size, slot->mbn.hash_table,
		    slot->mbn.hash_table_size);

	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
			     TEE_PARAM_TYPE_VALUE_INPUT,
			     TEE_PARAM_TYPE_MEMREF_INPUT,
			     TEE_PARAM_TYPE_VALUE_INPUT);

	vp[0].value.a = params[0].value.a;
	vp[0].value.b = params[0].value.b;
	vp[1].value.a = params[1].value.a;
	vp[1].value.b = params[1].value.b;
	vp[2].memref.buffer = buffer;
	vp[2].memref.size = buffer_size;
	vp[3].value.a = slot->mbn.hash_size;
	/* hash-table offset within the packed buffer */
	vp[3].value.b = slot->meta_data_size;

	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_PAS_VERIFY_IMAGE, pt, vp, NULL);

	TEE_Free(buffer);
	return res;
}
