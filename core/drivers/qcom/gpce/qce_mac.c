// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE hardware MAC driver for OP-TEE.
 *
 * Provides HMAC and AES-CMAC acceleration through
 * the Qualcomm Crypto Engine (QCE) BAM interface.
 */

#include <crypto/crypto.h>
#include <crypto/crypto_impl.h>
#include <drvcrypt.h>
#include <drvcrypt_mac.h>
#include <initcall.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <utee_defines.h>

#include "hash.h"

/* Forward declarations */
TEE_Result ucenv_optee_init(void);
env_t *ucenv_get_env(void);
int ENV_ce_clk_enable(env_t *env);

int SHA1_CE_BAM_init(hash_ctx_t *ctx);
int SHA2_256_CE_BAM_init(hash_ctx_t *ctx);  /* handles SHA2_224 + SHA2_256 */
int SHA2_384_CE_BAM_init(hash_ctx_t *ctx);
int SHA2_512_CE_BAM_init(hash_ctx_t *ctx);
int HASH_CE_BAM_set_hmac_key(hash_ctx_t *me, const uint8_t *key, size_t key_len);

int CMAC_CE_BAM_init(cipher_ctx_t *me);
int CMAC_BAM_push(void *ctx, const uint8_t *buf, size_t sz, bool first, bool last, bool blocking);
int CMAC_BAM_cleanup(cipher_ctx_t *me);

#define SHA1_BLOCK_SIZE     64
#define SHA2_224_BLOCK_SIZE 64
#define SHA2_256_BLOCK_SIZE 64
#define SHA2_384_BLOCK_SIZE 128
#define SHA2_512_BLOCK_SIZE 128
#define SHA_MAX_BLOCK_SIZE  128

#define AES_CMAC_BLOCK_SIZE 16

/* =========================================================================
 * HMAC-SHA1/224/256/384/512
 * =========================================================================
 */
struct qce_hmac_ctx {
	struct crypto_mac_ctx	base;
	hash_ctx_t		*bam_ctx;	/* uclib internal context */
	uint8_t			blk[SHA_MAX_BLOCK_SIZE]; /* OP-TEE block cache */
	size_t			blk_len;	/* bytes in partial block */
	size_t			blk_sz;		/* == RFC 2104 block size for this alg */
	size_t			md_sz;		/* digest size in bytes */
	bool			first;		/* first BAM transfer flag */
	UCLIB_HASH_ALG		uclib_alg;	/* uclib algorithm ID */
	uint32_t		tee_hash_algo;	/* TEE_ALG_SHA* — for key hash-down */

	bool			sha224_fallback;
	void			*sha224_inner;
	uint8_t		sha224_opad[SHA2_224_BLOCK_SIZE];
};

static const struct crypto_mac_ops qce_hmac_ops;

static struct qce_hmac_ctx *to_hmac_ctx(struct crypto_mac_ctx *ctx)
{
	return container_of(ctx, struct qce_hmac_ctx, base);
}

/* Map OP-TEE TEE_ALG_HMAC_* to uclib UCLIB_HASH_ALG + block/digest sizes */
static int tee_hmac_algo_to_uclib(uint32_t algo, UCLIB_HASH_ALG *uclib_alg,
				   size_t *blk_sz, size_t *md_sz,
				   uint32_t *tee_hash_algo)
{
	switch (algo) {
	case TEE_ALG_HMAC_SHA1:
		*uclib_alg = UCLIB_SHA1;
		*blk_sz = SHA1_BLOCK_SIZE;
		*md_sz = TEE_SHA1_HASH_SIZE;
		*tee_hash_algo = TEE_ALG_SHA1;
		return 0;
	case TEE_ALG_HMAC_SHA224:
		*uclib_alg = UCLIB_SHA2_224;
		*blk_sz = SHA2_224_BLOCK_SIZE;
		*md_sz = TEE_SHA224_HASH_SIZE;
		*tee_hash_algo = TEE_ALG_SHA224;
		return 0;
	case TEE_ALG_HMAC_SHA256:
		*uclib_alg = UCLIB_SHA2_256;
		*blk_sz = SHA2_256_BLOCK_SIZE;
		*md_sz = TEE_SHA256_HASH_SIZE;
		*tee_hash_algo = TEE_ALG_SHA256;
		return 0;
	case TEE_ALG_HMAC_SHA384:
		*uclib_alg = UCLIB_SHA2_384;
		*blk_sz = SHA2_384_BLOCK_SIZE;
		*md_sz = TEE_SHA384_HASH_SIZE;
		*tee_hash_algo = TEE_ALG_SHA384;
		return 0;
	case TEE_ALG_HMAC_SHA512:
		*uclib_alg = UCLIB_SHA2_512;
		*blk_sz = SHA2_512_BLOCK_SIZE;
		*md_sz = TEE_SHA512_HASH_SIZE;
		*tee_hash_algo = TEE_ALG_SHA512;
		return 0;
	default:
		return -1;
	}
}

/* Select the correct plain-hash BAM init function for the algorithm —
 * same mapping qce_hash.c uses; HMAC mode is layered on top afterward. */
static int qce_hmac_bam_init(hash_ctx_t *bam_ctx, UCLIB_HASH_ALG alg)
{
	switch (alg) {
	case UCLIB_SHA1:     return SHA1_CE_BAM_init(bam_ctx);
	case UCLIB_SHA2_224: /* falls through — SHA2_256 init handles 224 */
	case UCLIB_SHA2_256: return SHA2_256_CE_BAM_init(bam_ctx);
	case UCLIB_SHA2_384: return SHA2_384_CE_BAM_init(bam_ctx);
	case UCLIB_SHA2_512: return SHA2_512_CE_BAM_init(bam_ctx);
	default:             return -1;
	}
}

/*
 * RFC 2104 key normalization: keys longer than the block size are hashed
 * down first, shorter keys are zero-padded. Uses the generic OP-TEE hash
 * dispatcher (which itself routes to our own BAM hash driver) rather than
 * touching bam_ctx directly, so this doesn't duplicate hash.c's BAM
 * plumbing.
 */
static TEE_Result normalize_hmac_key(uint32_t tee_hash_algo, size_t md_sz,
				      const uint8_t *key, size_t key_len,
				      uint8_t *out, size_t blk_sz)
{
	void *h = NULL;
	TEE_Result res;

	memset(out, 0, blk_sz);

	if (key_len <= blk_sz) {
		memcpy(out, key, key_len);
		return TEE_SUCCESS;
	}

	res = crypto_hash_alloc_ctx(&h, tee_hash_algo);
	if (res)
		return res;
	res = crypto_hash_init(h);
	if (!res)
		res = crypto_hash_update(h, key, key_len);
	if (!res)
		res = crypto_hash_final(h, out, md_sz);
	crypto_hash_free_ctx(h);

	return res;
}

static TEE_Result qce_hmac_init(struct crypto_mac_ctx *ctx, const uint8_t *key,
				 size_t key_len)
{
	struct qce_hmac_ctx *c = to_hmac_ctx(ctx);
	uint8_t keybuf[SHA_MAX_BLOCK_SIZE];
	uint8_t ipad[SHA2_224_BLOCK_SIZE];
	uint8_t opad[SHA2_224_BLOCK_SIZE];
	TEE_Result tres;
	int ret;
	size_t i;

	if (c->sha224_fallback) {
		tres = normalize_hmac_key(TEE_ALG_SHA224, TEE_SHA224_HASH_SIZE,
					   key, key_len, keybuf, SHA2_224_BLOCK_SIZE);
		if (tres)
			return tres;
		for (i = 0; i < SHA2_224_BLOCK_SIZE; i++) {
			ipad[i] = keybuf[i] ^ 0x36;
			opad[i] = keybuf[i] ^ 0x5c;
		}
		memcpy(c->sha224_opad, opad, sizeof(opad));
		tres = crypto_hash_alloc_ctx(&c->sha224_inner, TEE_ALG_SHA224);
		if (tres)
			return tres;
		tres = crypto_hash_init(c->sha224_inner);
		if (!tres)
			tres = crypto_hash_update(c->sha224_inner, ipad, sizeof(ipad));
		if (tres) {
			crypto_hash_free_ctx(c->sha224_inner);
			c->sha224_inner = NULL;
		}
		return tres;
	}

	ret = ENV_ce_clk_enable(c->bam_ctx->env);
	if (ret) return TEE_ERROR_GENERIC;
	ret = qce_hmac_bam_init(c->bam_ctx, c->uclib_alg);
	if (ret) return TEE_ERROR_GENERIC;
	tres = normalize_hmac_key(c->tee_hash_algo, c->md_sz, key, key_len,
				   keybuf, c->blk_sz);
	if (tres) return tres;
	ret = HASH_CE_BAM_set_hmac_key(c->bam_ctx, keybuf, c->blk_sz);
	if (ret) {
		EMSG("QCE hmac: HASH_CE_BAM_set_hmac_key failed ret=%d", ret);
		return TEE_ERROR_GENERIC;
	}
	c->blk_len = 0;
	c->first = true;
	return TEE_SUCCESS;
}

static TEE_Result qce_hmac_update(struct crypto_mac_ctx *ctx,
				   const uint8_t *data, size_t len)
{
	struct qce_hmac_ctx *c = to_hmac_ctx(ctx);
	if (c->sha224_fallback)
		return crypto_hash_update(c->sha224_inner, data, len);
	while (len > 0) {
		if (c->blk_len == c->blk_sz) {
			int ret = SHA_CMN_BAM_xfer_payload(c->bam_ctx, c->blk, c->blk_sz,
							   c->first, false, true);
			if (ret) return TEE_ERROR_GENERIC;
			c->first = false;
			c->blk_len = 0;
		}
		size_t space = c->blk_sz - c->blk_len;
		size_t copy = (len < space) ? len : space;
		memcpy(c->blk + c->blk_len, data, copy);
		c->blk_len += copy; data += copy; len -= copy;
	}
	return TEE_SUCCESS;
}

static TEE_Result qce_hmac_final(struct crypto_mac_ctx *ctx, uint8_t *digest,
				  size_t len)
{
	struct qce_hmac_ctx *c = to_hmac_ctx(ctx);
	if (c->sha224_fallback) {
		uint8_t inner[TEE_SHA224_HASH_SIZE];
		void *outer = NULL;
		TEE_Result res = crypto_hash_final(c->sha224_inner, inner, sizeof(inner));
		if (res) return res;
		res = crypto_hash_alloc_ctx(&outer, TEE_ALG_SHA224);
		if (!res) res = crypto_hash_init(outer);
		if (!res) res = crypto_hash_update(outer, c->sha224_opad, sizeof(c->sha224_opad));
		if (!res) res = crypto_hash_update(outer, inner, sizeof(inner));
		if (!res) res = crypto_hash_final(outer, digest,
						  len < TEE_SHA224_HASH_SIZE ? len : TEE_SHA224_HASH_SIZE);
		if (outer) crypto_hash_free_ctx(outer);
		return res;
	}
	CE_SHA_CTX *sha_ctx;
	size_t md_sz;
	int ret = SHA_CMN_BAM_xfer_payload(c->bam_ctx, c->blk, c->blk_len,
					       c->first, true, true);
	if (ret) return TEE_ERROR_GENERIC;
	sha_ctx = HASH_GET_DIGEST_CTX(c->bam_ctx, CE_SHA_CTX);
	md_sz = (len < c->md_sz) ? len : c->md_sz;
	ret = CE_HAL_get_auth_iv(sha_ctx->ce_ctx, digest, md_sz);
	return ret ? TEE_ERROR_GENERIC : TEE_SUCCESS;
}

static void qce_hmac_free_ctx(struct crypto_mac_ctx *ctx)
{
	struct qce_hmac_ctx *c = to_hmac_ctx(ctx);
	if (c->sha224_inner) crypto_hash_free_ctx(c->sha224_inner);
	if (c->bam_ctx) {
		CE_SHA_CTX *sha_ctx = HASH_GET_DIGEST_CTX(c->bam_ctx, CE_SHA_CTX);
		if (sha_ctx && sha_ctx->ce_ctx) CE_HAL_free(sha_ctx->ce_ctx);
		free(c->bam_ctx->ctx); free(c->bam_ctx);
	}
	free(c);
}
static void qce_hmac_copy_state(struct crypto_mac_ctx *dst_ctx __unused,
				 struct crypto_mac_ctx *src_ctx __unused)
{
	/* Not implemented — would require a deep copy of CE HAL state */
}

static const struct crypto_mac_ops qce_hmac_ops = {
	.init       = qce_hmac_init,
	.update     = qce_hmac_update,
	.final      = qce_hmac_final,
	.free_ctx   = qce_hmac_free_ctx,
	.copy_state = qce_hmac_copy_state,
};

static TEE_Result qce_hmac_alloc_ctx(struct crypto_mac_ctx **ctx, uint32_t algo)
{
	struct qce_hmac_ctx *c;
	UCLIB_HASH_ALG uclib_alg; size_t blk_sz, md_sz; uint32_t tee_hash_algo;
	env_t *env;
	if (tee_hmac_algo_to_uclib(algo, &uclib_alg, &blk_sz, &md_sz, &tee_hash_algo))
		return TEE_ERROR_NOT_IMPLEMENTED;
	c = calloc(1, sizeof(*c)); if (!c) return TEE_ERROR_OUT_OF_MEMORY;
	c->uclib_alg = uclib_alg; c->blk_sz = blk_sz; c->md_sz = md_sz;
	c->tee_hash_algo = tee_hash_algo; c->sha224_fallback = (algo == TEE_ALG_HMAC_SHA224);
	c->base.ops = &qce_hmac_ops; c->first = true;
	if (!c->sha224_fallback) {
		env = ucenv_get_env();
		if (!env) { free(c); return TEE_ERROR_GENERIC; }
		c->bam_ctx = calloc(1, sizeof(hash_ctx_t));
		if (!c->bam_ctx) { free(c); return TEE_ERROR_OUT_OF_MEMORY; }
		c->bam_ctx->magic = HASH_CTX_MAGIC; c->bam_ctx->env = env;
		c->bam_ctx->alg = uclib_alg; c->bam_ctx->engine = UCLIB_CE_BAM;
	}
	*ctx = &c->base; return TEE_SUCCESS;
}
/* =========================================================================
 * AES-CMAC (128/256)
 * =========================================================================
 */
struct qce_cmac_ctx {
	struct crypto_mac_ctx	base;
	cipher_ctx_t		uctx;	/* reused: env/alg/key/key_sz/ce_ctx */
	uint8_t			blk[AES_CMAC_BLOCK_SIZE]; /* OP-TEE block cache */
	size_t			blk_len;
	bool			first;
	bool			inited;	/* CMAC_CE_BAM_init() succeeded */
};

static const struct crypto_mac_ops qce_cmac_ops;

static struct qce_cmac_ctx *to_cmac_ctx(struct crypto_mac_ctx *ctx)
{
	return container_of(ctx, struct qce_cmac_ctx, base);
}

static TEE_Result qce_cmac_alloc_ctx(struct crypto_mac_ctx **ctx, uint32_t algo)
{
	struct qce_cmac_ctx *c;

	if (algo != TEE_ALG_AES_CMAC) {
		EMSG("QCE cmac: unsupported algo 0x%08"PRIx32, algo);
		return TEE_ERROR_NOT_IMPLEMENTED;
	}

	c = calloc(1, sizeof(*c));
	if (!c)
		return TEE_ERROR_OUT_OF_MEMORY;

	c->base.ops = &qce_cmac_ops;
	*ctx = &c->base;

	return TEE_SUCCESS;
}

static TEE_Result qce_cmac_init(struct crypto_mac_ctx *ctx, const uint8_t *key,
				 size_t key_len)
{
	struct qce_cmac_ctx *c = to_cmac_ctx(ctx);
	env_t *env;
	int ret;

	if (key_len == 16)
		c->uctx.alg = UCLIB_AES128;
	else if (key_len == 32)
		c->uctx.alg = UCLIB_AES256;
	else {
		EMSG("QCE cmac: unsupported AES key length %zu (GPCE BAM supports 128/256 only)",
		     key_len);
		return TEE_ERROR_NOT_IMPLEMENTED;
	}

	env = ucenv_get_env();
	if (!env) {
		EMSG("QCE cmac: ucenv_get_env returned NULL");
		return TEE_ERROR_GENERIC;
	}
	c->uctx.env = env;

	memcpy(c->uctx.key, key, key_len);
	c->uctx.key_sz = key_len;

	ret = ENV_ce_clk_enable(env);
	if (ret)
		return TEE_ERROR_GENERIC;

	if (c->inited) {
		CMAC_BAM_cleanup(&c->uctx);
		c->inited = false;
	}

	ret = CMAC_CE_BAM_init(&c->uctx);
	if (ret) {
		EMSG("QCE cmac: CMAC_CE_BAM_init failed ret=%d", ret);
		return TEE_ERROR_GENERIC;
	}
	c->inited  = true;
	c->first   = true;
	c->blk_len = 0;

	return TEE_SUCCESS;
}

/* update/final buffer into 16-byte AES blocks and only push to hardware
 * with the correct first/last flags — CMAC's on-chip finalization needs
 * to know which push is the last one, but crypto_mac_ops.update() has no
 * "last" flag (unlike drvcrypt_cipher_update), so all data is cached and
 * the actual last push happens inside final(), mirroring the same
 * block-cache split qce_hash.c/the HMAC path above already use. A full
 * block already held is only flushed (last=false) once we know more
 * data is coming — deferred to the top of the next pass through this
 * loop — so an exact-multiple-of-16-byte message correctly leaves its
 * true final block for final() to send with last=true; flushing it
 * eagerly instead breaks CMAC's on-chip K1/K2 subkey selection for
 * exactly that case. */
static TEE_Result qce_cmac_update(struct crypto_mac_ctx *ctx,
				   const uint8_t *data, size_t len)
{
	struct qce_cmac_ctx *c = to_cmac_ctx(ctx);

	while (len > 0) {
		if (c->blk_len == AES_CMAC_BLOCK_SIZE) {
			int ret = CMAC_BAM_push(&c->uctx, c->blk,
						AES_CMAC_BLOCK_SIZE, c->first,
						false, true);
			if (ret)
				return TEE_ERROR_GENERIC;
			c->first = false;
			c->blk_len = 0;
		}

		size_t space = AES_CMAC_BLOCK_SIZE - c->blk_len;
		size_t copy = (len < space) ? len : space;

		memcpy(c->blk + c->blk_len, data, copy);
		c->blk_len += copy;
		data += copy;
		len -= copy;
	}
	return TEE_SUCCESS;
}

static TEE_Result qce_cmac_final(struct crypto_mac_ctx *ctx, uint8_t *digest,
				  size_t len)
{
	struct qce_cmac_ctx *c = to_cmac_ctx(ctx);
	int ret;

	ret = CMAC_BAM_push(&c->uctx, c->blk, c->blk_len, c->first, true, true);
	if (ret)
		return TEE_ERROR_GENERIC;

	ret = CE_HAL_get_auth_iv(c->uctx.ce_ctx, digest, len);
	if (ret)
		return TEE_ERROR_GENERIC;

	return TEE_SUCCESS;
}

static void qce_cmac_free_ctx(struct crypto_mac_ctx *ctx)
{
	struct qce_cmac_ctx *c = to_cmac_ctx(ctx);

	if (c->inited)
		CMAC_BAM_cleanup(&c->uctx);
	free(c);
}

static void qce_cmac_copy_state(struct crypto_mac_ctx *dst_ctx __unused,
				 struct crypto_mac_ctx *src_ctx __unused)
{
	/* Not implemented — would require a deep copy of CE HAL state */
}

static const struct crypto_mac_ops qce_cmac_ops = {
	.init       = qce_cmac_init,
	.update     = qce_cmac_update,
	.final      = qce_cmac_final,
	.free_ctx   = qce_cmac_free_ctx,
	.copy_state = qce_cmac_copy_state,
};

/* =========================================================================
 * Driver init — register both MAC drivers with OP-TEE drvcrypt framework
 * =========================================================================
 */
static TEE_Result qce_mac_driver_init(void)
{
	TEE_Result res;

    res = ucenv_optee_init();
    if (res) {
            EMSG("QCE hash: ucenv_optee_init failed: %#"PRIx32, res);
            return res;
    }

	res = drvcrypt_register_hmac(&qce_hmac_alloc_ctx);
	if (res) {
		EMSG("QCE mac: drvcrypt_register_hmac failed: %#"PRIx32, res);
		return res;
	}

	res = drvcrypt_register_cmac(&qce_cmac_alloc_ctx);
	if (res) {
		EMSG("QCE mac: drvcrypt_register_cmac failed: %#"PRIx32, res);
		return res;
	}

	return TEE_SUCCESS;
}

driver_init(qce_mac_driver_init);