// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE hardware cipher driver for OP-TEE.
 *
 * Provides AES, DES, and 3DES cipher acceleration through
 * the GPCE BAM interface.
 */

#include <crypto/crypto_impl.h>
#include <drvcrypt.h>
#include <drvcrypt_cipher.h>
#include <initcall.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <utee_defines.h>

#include "hash.h"

/* Forward declarations — */
TEE_Result ucenv_optee_init(void);
env_t *ucenv_get_env(void);
int ENV_ce_clk_enable(env_t *env);
int CIPHER_CE_BAM_init(cipher_ctx_t *me);
int CIPHER_BAM_cipher(void *ctx, const uint8_t *ibuf, size_t isz, uint8_t *obuf, size_t osz, bool first, bool last);
int CIPHER_BAM_cleanup(cipher_ctx_t *me);

#define AES_BLOCK_SIZE  16
#define DES_BLOCK_SIZE  8

/* =========================================================================
 * OP-TEE cipher context — wraps the uclib cipher_ctx_t directly
 * =========================================================================
 */
struct qce_cipher_ctx {
	cipher_ctx_t uctx;
	UCLIB_CIPHER_MODE mode;
	size_t blk_sz;
	bool first;
	bool inited;   /* CIPHER_CE_BAM_init() succeeded — cleanup owes CE_HAL_free */
};

/* Map a TEE_ALG_* cipher algorithm to its uclib mode + block size.
 * The concrete UCLIB_CIPHER_ALG (incl. AES key size) is resolved in
 * qce_cipher_init() once the actual key length is known.
 */
static int tee_alg_to_mode(uint32_t algo, bool *is_aes, UCLIB_CIPHER_ALG *des_alg, size_t *blk_sz, UCLIB_CIPHER_MODE *mode)
{
	*is_aes = false;
	switch (algo) {
	case TEE_ALG_AES_ECB_NOPAD:
		*is_aes = true; *blk_sz = AES_BLOCK_SIZE; *mode = UCLIB_ECB; return 0;
	case TEE_ALG_AES_CBC_NOPAD:
		*is_aes = true; *blk_sz = AES_BLOCK_SIZE; *mode = UCLIB_CBC; return 0;
	case TEE_ALG_AES_CTR:
		*is_aes = true; *blk_sz = AES_BLOCK_SIZE; *mode = UCLIB_CTR; return 0;
	case TEE_ALG_AES_CTS:
		*is_aes = true; *blk_sz = AES_BLOCK_SIZE; *mode = UCLIB_CTS; return 0;
	case TEE_ALG_AES_XTS:
		*is_aes = true; *blk_sz = AES_BLOCK_SIZE; *mode = UCLIB_XTS; return 0;
	case TEE_ALG_DES_ECB_NOPAD:
		*des_alg = UCLIB_DES; *blk_sz = DES_BLOCK_SIZE; *mode = UCLIB_ECB; return 0;
	case TEE_ALG_DES_CBC_NOPAD:
		*des_alg = UCLIB_DES; *blk_sz = DES_BLOCK_SIZE; *mode = UCLIB_CBC; return 0;
	case TEE_ALG_DES3_ECB_NOPAD:
		*des_alg = UCLIB_3DES; *blk_sz = DES_BLOCK_SIZE; *mode = UCLIB_ECB; return 0;
	case TEE_ALG_DES3_CBC_NOPAD:
		*des_alg = UCLIB_3DES; *blk_sz = DES_BLOCK_SIZE; *mode = UCLIB_CBC; return 0;
	default:
		return -1;
	}
}

static TEE_Result qce_cipher_alloc_ctx(void **ctx, uint32_t algo)
{
	struct qce_cipher_ctx *c;
	bool is_aes;
	UCLIB_CIPHER_ALG des_alg = UCLIB_CIPHER_MAX_ALG_COUNT;
	size_t blk_sz;
	UCLIB_CIPHER_MODE mode;

	if (tee_alg_to_mode(algo, &is_aes, &des_alg, &blk_sz, &mode)) {
		EMSG("QCE cipher: unsupported algo 0x%08"PRIx32, algo);
		return TEE_ERROR_NOT_IMPLEMENTED;
	}

	c = calloc(1, sizeof(*c));
	if (!c)
		return TEE_ERROR_OUT_OF_MEMORY;

	c->mode = mode;
	c->blk_sz = blk_sz;
	/* DES/DES3 alg is already known; AES128 vs AES256 waits for the key
	 * length in qce_cipher_init(). Stash the DES choice in uctx.alg for
	 * now so init() only has to special-case AES. */
	c->uctx.alg = is_aes ? UCLIB_CIPHER_MAX_ALG_COUNT : des_alg;

	*ctx = c;
	return TEE_SUCCESS;
}

static void qce_cipher_free_ctx(void *ctx)
{
	struct qce_cipher_ctx *c = ctx;

	if (c->inited)
		CIPHER_BAM_cleanup(&c->uctx);
	free(c);
}

static TEE_Result qce_cipher_init(struct drvcrypt_cipher_init *dinit)
{
	struct qce_cipher_ctx *c = dinit->ctx;
	env_t *env;
	int ret;

	env = ucenv_get_env();
	if (!env) {
		EMSG("QCE cipher: ucenv_get_env returned NULL");
		return TEE_ERROR_GENERIC;
	}
	c->uctx.env = env;

	/* Resolve AES128 vs AES256 now that the key length is known */
	if (c->uctx.alg == UCLIB_CIPHER_MAX_ALG_COUNT) {
		if (dinit->key1.length == 16)
			c->uctx.alg = UCLIB_AES128;
		else if (dinit->key1.length == 32)
			c->uctx.alg = UCLIB_AES256;
		else {
			EMSG("QCE cipher: unsupported AES key length %zu (GPCE BAM supports 128/256 only)",
			     dinit->key1.length);
			return TEE_ERROR_NOT_IMPLEMENTED;
		}
	}

	if (dinit->key1.length > sizeof(c->uctx.key))
		return TEE_ERROR_BAD_PARAMETERS;
	memcpy(c->uctx.key, dinit->key1.data, dinit->key1.length);
	c->uctx.key_sz = dinit->key1.length;

	c->uctx.mode = c->mode;

	if (c->mode == UCLIB_XTS) {
		if (!dinit->key2.data || dinit->key2.length > sizeof(c->uctx.xts_key))
			return TEE_ERROR_BAD_PARAMETERS;
		memcpy(c->uctx.xts_key, dinit->key2.data, dinit->key2.length);
		c->uctx.xts_key_sz = dinit->key2.length;
	}

	if (dinit->iv.data && dinit->iv.length) {
		if (dinit->iv.length > sizeof(c->uctx.iv))
			return TEE_ERROR_BAD_PARAMETERS;
		memcpy(c->uctx.iv, dinit->iv.data, dinit->iv.length);
	}

	c->uctx.flags = dinit->encrypt ? UCLIB_CIPHER_FLAG_DIRECTION : 0;

	ret = ENV_ce_clk_enable(env);
	if (ret)
		return TEE_ERROR_GENERIC;

	/* Re-init is possible via copy_state()/reuse — free any prior CE ctx */
	if (c->inited) {
		CIPHER_BAM_cleanup(&c->uctx);
		c->inited = false;
	}

	ret = CIPHER_CE_BAM_init(&c->uctx);
	if (ret) {
		EMSG("QCE cipher: CIPHER_CE_BAM_init failed ret=%d", ret);
		return TEE_ERROR_GENERIC;
	}
	c->inited = true;
	c->first = true;

	return TEE_SUCCESS;
}

static TEE_Result qce_cipher_update(struct drvcrypt_cipher_update *dupdate)
{
	struct qce_cipher_ctx *c = dupdate->ctx;
	int ret;

	if (!dupdate->src.length) {
		/* Nothing to transfer — if this is also the last call there is
		 * no work to hand the BAM engine; treat as success. */
		return TEE_SUCCESS;
	}

	if (dupdate->dst.length < dupdate->src.length)
		return TEE_ERROR_SHORT_BUFFER;

	ret = CIPHER_BAM_cipher(&c->uctx, dupdate->src.data, dupdate->src.length,
				 dupdate->dst.data, dupdate->dst.length,
				 c->first, dupdate->last);
	if (ret) {
		EMSG("QCE cipher: CIPHER_BAM_cipher failed ret=%d", ret);
		return TEE_ERROR_GENERIC;
	}
	c->first = false;

	return TEE_SUCCESS;
}

static void qce_cipher_final(void *ctx __unused)
{
	/* Nothing to do — CIPHER_BAM_cipher() already saw last=true in the
	 * final update() call. CE HAL context is released in free_ctx(). */
}

static void qce_cipher_copy_state(void *dst_ctx, void *src_ctx)
{
	struct qce_cipher_ctx *dst = dst_ctx;
	struct qce_cipher_ctx *src = src_ctx;

	/* Copy the logical state only — dst must run its own init() to get
	 * its own CE HAL context before update()/final() are usable. */
	dst->uctx = src->uctx;
	dst->uctx.ce_ctx = NULL;
	dst->mode = src->mode;
	dst->blk_sz = src->blk_sz;
	dst->first = src->first;
	dst->inited = false;
}

static struct drvcrypt_cipher qce_cipher_ops = {
	.alloc_ctx = qce_cipher_alloc_ctx,
	.free_ctx = qce_cipher_free_ctx,
	.init = qce_cipher_init,
	.update = qce_cipher_update,
	.final = qce_cipher_final,
	.copy_state = qce_cipher_copy_state,
};

static TEE_Result qce_cipher_driver_init(void)
{
	TEE_Result res;

    res = ucenv_optee_init();
    if (res) {
            EMSG("QCE hash: ucenv_optee_init failed: %#"PRIx32, res);
            return res;
    }

	res = drvcrypt_register_cipher(&qce_cipher_ops);
	if (res) {
		EMSG("QCE cipher: drvcrypt_register_cipher failed: %#"PRIx32, res);
		return res;
	}

	return TEE_SUCCESS;
}

driver_init(qce_cipher_driver_init);
