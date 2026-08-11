// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * Firmware authentication backend for the PAS TA. Built under
 * CFG_QCOM_PAS_AUTH; see pas_auth.h for the INIT_IMAGE/AUTH_AND_RESET flow.
 */

#include <pas_auth.h>
#include <pas_fuse.h>
#include <pas_mbn_parser.h>
#include <pas_sig_auth.h>
#include <pta_qcom_fuse.h>
#include <pta_qcom_pas.h>
#include <qcom_pas_priv.h>
#include <string.h>
#include <string_ext.h>
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

	size = params[1].memref.size;
	if (size && !params[1].memref.buffer)
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
	uint8_t anchor[PTA_QCOM_FUSE_ROOT_OF_TRUST_SIZE] = { };
	struct pas_md_slot *slot = get_meta_data_slot(s, pas_id);
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t hash_size = TEE_SHA384_HASH_SIZE;
	bool secboot_on = false;

	if (!slot) {
		EMSG("PAS auth: no metadata for pas_id=%"PRIu32
		     " (call INIT_IMAGE first)", pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	/*
	 * Read secure-boot enable state once and fork the entire
	 * authentication path on that one value.
	 */
	res = pas_fuse_get_secboot_and_root_anchor(anchor, &secboot_on);
	/*
	 * Fail closed on a fuse-read failure: treat as secure-boot enabled.
	 * Treating it as "not provisioned" would downgrade a secure-booted
	 * board to hash-only authentication for the rest of the boot.
	 */
	if (res)
		secboot_on = true;

	/*
	 * On secure-boot devices the segment-hash algorithm is selected by
	 * the OEM metadata's root_cert_sel field, read via the fuse PTA.
	 * On unprovisioned devices default to SHA-384.
	 */
	if (secboot_on) {
		res = pas_sig_auth_hash_size(slot, &hash_size);
		if (res) {
			EMSG("PAS auth: cannot pick hash size: %#"PRIx32, res);
			goto out;
		}
	}

	res = pas_mbn_parse(slot->meta_data, slot->meta_data_size, hash_size,
			    &slot->mbn);
	if (res) {
		EMSG("PAS auth: MBN parse failed: %#"PRIx32, res);
		goto out;
	}

	if (secboot_on) {
		res = pas_sig_auth_authenticate(&slot->mbn, slot->meta_data,
						slot->meta_data_size, pas_id,
						hash_size, anchor);
		if (res)
			goto out;
	}

	slot->hash_table_valid = true;
	res = TEE_SUCCESS;
out:
	memzero_explicit(anchor, sizeof(anchor));

	return res;
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
