// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 *
 * Implementation of the Qualcomm Crypto Engine hash driver.
 */
#ifndef CFG_CRYPTO_DRIVER_DEBUG
#define CFG_CRYPTO_DRIVER_DEBUG 0
#endif

#include <crypto/crypto_impl.h>
#include <drvcrypt_hash.h>
#include <initcall.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <stdlib.h>
#include <string.h>
#include <string_ext.h>
#include <trace.h>
#include <tee_api_types.h>
#include <tee_api_defines.h>
#include <utee_defines.h>

#include "ce_hwio.h"
#include "hash.h"

struct qce_hash_ctx {
	struct crypto_hash_ctx hash_ctx;
	void *ctx_handle;
	uint32_t tee_alg;
};

static const struct crypto_hash_ops qce_hash_ops;
static struct qce_dev qce_dev = {
	.mutex = MUTEX_INITIALIZER,
};

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QCOM_GPCE_REG_BASE,
			QCOM_GPCE_REG_SIZE);

#define QCE_HANDLE_MASK_32	UINT32_C(0x3794A1FB)
#define QCE_HANDLE_MASK_64	UINT64_C(0x38FD256ECA764B19)

static uintptr_t qce_hash_handle_mask(void)
{
	return sizeof(uintptr_t) == sizeof(uint32_t) ? QCE_HANDLE_MASK_32 :
		QCE_HANDLE_MASK_64;
}

static void *qce_hash_scramble_ctx(hash_ctx_t *ctx)
{
	return (void *)((uintptr_t)ctx ^ qce_hash_handle_mask());
}

static hash_ctx_t *qce_hash_get_ctx(struct qce_hash_ctx *ctx)
{
	if (!ctx || !ctx->ctx_handle)
		return NULL;

	return (hash_ctx_t *)((uintptr_t)ctx->ctx_handle ^
			      qce_hash_handle_mask());
}

static void qce_hash_free_ctx(struct crypto_hash_ctx *ctx)
{
	struct qce_hash_ctx *qce_ctx = NULL;
	hash_ctx_t *state = NULL;

	if (!ctx || ctx->ops != &qce_hash_ops)
		return;

	qce_ctx = container_of(ctx, struct qce_hash_ctx, hash_ctx);
	state = qce_hash_get_ctx(qce_ctx);
	if (state && state->ctx)
		(void)SHA_CMN_CE_cleanup(state);
	if (state) {
		memset(state, 0, sizeof(*state));
		free(state);
	}

	memzero_explicit(qce_ctx, sizeof(*qce_ctx));
	free(qce_ctx);
}

static TEE_Result qce_hash_init(struct crypto_hash_ctx *ctx)
{
	struct qce_hash_ctx *qce_ctx = NULL;
	hash_ctx_t *state = NULL;
	TEE_Result ret = TEE_SUCCESS;

	if (!ctx || ctx->ops != &qce_hash_ops)
		return TEE_ERROR_BAD_PARAMETERS;

	qce_ctx = container_of(ctx, struct qce_hash_ctx, hash_ctx);
	state = qce_hash_get_ctx(qce_ctx);
	if (!hash_is_ctx_valid(state))
		return TEE_ERROR_BAD_STATE;

	if (state->ctx) {
		ret = SHA_CMN_CE_cleanup(state);
		if (ret != TEE_SUCCESS)
			return ret;
	}

	ret = SHA1_CE_HWIO_init(state);

	return ret;
}

static TEE_Result qce_hash_update(struct crypto_hash_ctx *ctx,
				  const uint8_t *data, size_t len)
{
	struct qce_hash_ctx *qce_ctx = NULL;
	hash_ctx_t *state = NULL;

	if (!ctx || ctx->ops != &qce_hash_ops || !data ||
	    (uintptr_t)data > UINTPTR_MAX - len)
		return TEE_ERROR_BAD_PARAMETERS;

	qce_ctx = container_of(ctx, struct qce_hash_ctx, hash_ctx);
	state = qce_hash_get_ctx(qce_ctx);
	if (!hash_is_ctx_valid(state) || !state->ctx)
		return TEE_ERROR_BAD_STATE;

	return SHA_CMN_CE_update(state, data, len);
}

static TEE_Result qce_hash_final(struct crypto_hash_ctx *ctx,
				 uint8_t *digest, size_t len)
{
	struct qce_hash_ctx *qce_ctx = NULL;
	hash_ctx_t *state = NULL;

	if (!ctx || ctx->ops != &qce_hash_ops || !digest || !len ||
	    (uintptr_t)digest > UINTPTR_MAX - len)
		return TEE_ERROR_BAD_PARAMETERS;

	qce_ctx = container_of(ctx, struct qce_hash_ctx, hash_ctx);
	state = qce_hash_get_ctx(qce_ctx);
	if (!hash_is_ctx_valid(state) || !state->ctx)
		return TEE_ERROR_BAD_STATE;
	if (len < state->md_sz)
		return TEE_ERROR_SHORT_BUFFER;

	return SHA_CMN_CE_final(state, digest, state->md_sz);
}

static void qce_hash_copy_state(struct crypto_hash_ctx *dst,
				struct crypto_hash_ctx *src)
{
	struct qce_hash_ctx *dst_ctx = NULL;
	struct qce_hash_ctx *src_ctx = NULL;
	hash_ctx_t *dst_state = NULL;
	hash_ctx_t *src_state = NULL;
	TEE_Result ret = TEE_ERROR_BAD_STATE;

	if (!dst || !src || dst->ops != &qce_hash_ops ||
	    src->ops != &qce_hash_ops)
		goto out;

	dst_ctx = container_of(dst, struct qce_hash_ctx, hash_ctx);
	src_ctx = container_of(src, struct qce_hash_ctx, hash_ctx);
	dst_state = qce_hash_get_ctx(dst_ctx);
	src_state = qce_hash_get_ctx(src_ctx);
	if (!hash_is_ctx_valid(dst_state) || !hash_is_ctx_valid(src_state) ||
	    dst_ctx->tee_alg != src_ctx->tee_alg)
		goto out;

	if (dst_state->ctx) {
		ret = SHA_CMN_CE_cleanup(dst_state);
		if (ret != TEE_SUCCESS)
			goto out;
	}

	*dst_state = *src_state;
	dst_state->ctx = NULL;
	if (src_state->ctx) {
		dst_state->ctx = calloc(1, src_state->ctx_sz);
		if (!dst_state->ctx) {
			ret = TEE_ERROR_OUT_OF_MEMORY;
			goto out;
		}
		ret = SHA_CMN_CE_copy(dst_state, src_state);
		if (ret != TEE_SUCCESS)
			goto out;
	}
	ret = TEE_SUCCESS;
out:
	if (ret != TEE_SUCCESS)
		EMSG("QCE hash state copy failed: %#"PRIx32, ret);
}

static const struct crypto_hash_ops qce_hash_ops = {
	.init = qce_hash_init,
	.update = qce_hash_update,
	.final = qce_hash_final,
	.free_ctx = qce_hash_free_ctx,
	.copy_state = qce_hash_copy_state,
};

static TEE_Result qce_hash_alloc_ctx(struct crypto_hash_ctx **ctx,
				     uint32_t algo)
{
	struct qce_hash_ctx *qce_ctx = NULL;
	hash_ctx_t *state = NULL;

	if (!ctx)
		return TEE_ERROR_BAD_PARAMETERS;
	*ctx = NULL;

	if (algo != TEE_ALG_SHA1)
		return TEE_ERROR_NOT_IMPLEMENTED;

	qce_ctx = calloc(1, sizeof(*qce_ctx));
	if (!qce_ctx)
		return TEE_ERROR_OUT_OF_MEMORY;

	qce_ctx->tee_alg = algo;
	state = calloc(1, sizeof(*state));
	if (!state) {
		free(qce_ctx);
		return TEE_ERROR_OUT_OF_MEMORY;
	}
	state->magic = HASH_CTX_MAGIC;
	state->dev = &qce_dev;
	qce_ctx->ctx_handle = qce_hash_scramble_ctx(state);
	qce_ctx->hash_ctx.ops = &qce_hash_ops;
	*ctx = &qce_ctx->hash_ctx;

	return TEE_SUCCESS;
}

static TEE_Result qce_hash_driver_init(void)
{
	TEE_Result res = TEE_SUCCESS;

	qce_dev.base = (vaddr_t)phys_to_virt(QCOM_GPCE_REG_BASE,
						     MEM_AREA_IO_SEC,
						     QCOM_GPCE_REG_SIZE);
	if (!qce_dev.base) {
		EMSG("QCE hash: failed to map the Crypto Engine registers");
		return TEE_ERROR_GENERIC;
	}

	IMSG("GPCE: CE base PA %#"PRIxPA" VA %#"PRIxVA,
	     (paddr_t)QCOM_GPCE_REG_BASE, qce_dev.base);
	res = drvcrypt_register_hash(qce_hash_alloc_ctx);

	if (res)
		EMSG("QCE hash: drvcrypt_register_hash failed: %#"PRIx32, res);

	return res;
}

driver_init(qce_hash_driver_init);
