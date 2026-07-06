// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <pas_auth.h>
#include <string.h>
#include <string_ext.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <util.h>

TEE_Result pas_auth_verify_hash(uint32_t hash_algo, const uint8_t *data,
				size_t data_len, const uint8_t *expected,
				size_t hash_size)
{
	uint8_t dgst[PAS_AUTH_MAX_HASH_SIZE] = { };
	TEE_OperationHandle op = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	size_t len = sizeof(dgst);

	if (!data || !expected || !hash_size || hash_size > sizeof(dgst))
		return TEE_ERROR_BAD_PARAMETERS;

	res = TEE_AllocateOperation(&op, hash_algo, TEE_MODE_DIGEST, 0);
	if (res != TEE_SUCCESS)
		return res;

	res = TEE_DigestDoFinal(op, data, data_len, dgst, &len);
	if (res != TEE_SUCCESS)
		goto out;

	if (len != hash_size) {
		res = TEE_ERROR_SECURITY;
		goto out;
	}

	if (consttime_memcmp(dgst, expected, hash_size) != 0)
		res = TEE_ERROR_SECURITY;
	else
		res = TEE_SUCCESS;
out:
	TEE_FreeOperation(op);
	memzero_explicit(dgst, sizeof(dgst));

	return res;
}
