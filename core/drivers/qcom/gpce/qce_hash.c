// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * Qualcomm Crypto Engine (QCE) hash driver for OP-TEE.
 * Provides SHA and HMAC acceleration through the GPCE BAM interface.
 */

#include <crypto/crypto_impl.h>
#include <drvcrypt_hash.h>
#include <initcall.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <utee_defines.h>
#include "hash.h"

TEE_Result ucenv_optee_init(void);
env_t *ucenv_get_env(void);

/* BAM engine init functions — from sha*_bam.c */
int SHA1_CE_BAM_init(hash_ctx_t *ctx);
int SHA2_256_CE_BAM_init(hash_ctx_t *ctx);  /* handles SHA2_224 + SHA2_256 */
int SHA2_384_CE_BAM_init(hash_ctx_t *ctx);
int SHA2_512_CE_BAM_init(hash_ctx_t *ctx);

/* =========================================================================
 * SHA block sizes (bytes) — CE requires full blocks for intermediate xfers
 * =========================================================================
 */
#define SHA1_BLOCK_SIZE     64
#define SHA2_224_BLOCK_SIZE 64
#define SHA2_256_BLOCK_SIZE 64
#define SHA2_384_BLOCK_SIZE 128
#define SHA2_512_BLOCK_SIZE 128
#define SHA_MAX_BLOCK_SIZE  128   /* max across all supported algorithms */

/* =========================================================================
 * OP-TEE hash context — wraps uclib hash_ctx_t directly
 * =========================================================================
 */
struct qce_hash_ctx {
        struct crypto_hash_ctx  base;
        hash_ctx_t             *bam_ctx;        /* uclib internal context */
        uint8_t                 blk[SHA_MAX_BLOCK_SIZE]; /* OP-TEE block cache */
        size_t                  blk_len;        /* bytes in partial block */
        size_t                  blk_sz;         /* SHA block size for this alg */
        size_t                  md_sz;          /* digest size in bytes */
        bool                    first;          /* first BAM transfer flag */
        UCLIB_HASH_ALG          uclib_alg;      /* uclib algorithm ID */
};

static const struct crypto_hash_ops qce_hash_ops;

static struct qce_hash_ctx *to_qce_ctx(struct crypto_hash_ctx *ctx)
{
        return container_of(ctx, struct qce_hash_ctx, base);
}

/* Map OP-TEE TEE_ALG_* to uclib UCLIB_HASH_ALG + block/digest sizes */
static int tee_alg_to_uclib(uint32_t algo, UCLIB_HASH_ALG *uclib_alg,
                             size_t *blk_sz, size_t *md_sz)
{
        switch (algo) {
        case TEE_ALG_SHA1:
                *uclib_alg = UCLIB_SHA1;
                *blk_sz    = SHA1_BLOCK_SIZE;
                *md_sz     = TEE_SHA1_HASH_SIZE;
                return 0;
        case TEE_ALG_SHA224:
                *uclib_alg = UCLIB_SHA2_224;
                *blk_sz    = SHA2_224_BLOCK_SIZE;
                *md_sz     = TEE_SHA224_HASH_SIZE;
                return 0;
        case TEE_ALG_SHA256:
                *uclib_alg = UCLIB_SHA2_256;
                *blk_sz    = SHA2_256_BLOCK_SIZE;
                *md_sz     = TEE_SHA256_HASH_SIZE;
                return 0;
        case TEE_ALG_SHA384:
                *uclib_alg = UCLIB_SHA2_384;
                *blk_sz    = SHA2_384_BLOCK_SIZE;
                *md_sz     = TEE_SHA384_HASH_SIZE;
                return 0;
        case TEE_ALG_SHA512:
                *uclib_alg = UCLIB_SHA2_512;
                *blk_sz    = SHA2_512_BLOCK_SIZE;
                *md_sz     = TEE_SHA512_HASH_SIZE;
                return 0;
        default:
                return -1;
        }
}

/* Select the correct BAM init function for the algorithm */
static int qce_bam_init(hash_ctx_t *bam_ctx, UCLIB_HASH_ALG alg)
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

/* =========================================================================
 * OP-TEE crypto_hash_ops callbacks — direct BAM engine path
 * =========================================================================
 */

/*
 * init — call SHA*_CE_BAM_init() directly, bypassing uclib_hash_init().
 * Resets the OP-TEE block cache and first-transfer flag.
 */
static TEE_Result qce_hash_init_ctx(struct crypto_hash_ctx *ctx)
{
        struct qce_hash_ctx *c = to_qce_ctx(ctx);
        int ret;

        /* Enable CE hardware clock — required before any CE operation */
        ret = ENV_ce_clk_enable(c->bam_ctx->env);
        if (ret)
                return TEE_ERROR_GENERIC;

        ret = qce_bam_init(c->bam_ctx, c->uclib_alg);
        if (ret)
                return TEE_ERROR_GENERIC;

        c->blk_len = 0;
        c->first   = true;
        return TEE_SUCCESS;
}

/*
 * update — OP-TEE block cache + SHA_CMN_BAM_xfer_payload() directly.
 *
 * Accumulates data in blk[] until a full SHA block is ready, then
 * calls SHA_CMN_BAM_xfer_payload() with last=false. Partial blocks
 * are held in blk[] until final() is called.
 *
 * This replaces uclib's util_blkbuf block cache.
 */
static TEE_Result qce_hash_update_ctx(struct crypto_hash_ctx *ctx,
                                      const uint8_t *data, size_t len)
{
        struct qce_hash_ctx *c = to_qce_ctx(ctx);

        while (len > 0) {
                /* A full block already held from a previous pass is only
                 * safe to flush (last=false) once we know more data is
                 * still coming — i.e. now, since len > 0 here. Flushing
                 * it eagerly as soon as it filled (instead of deferring
                 * to this check) would send the true final block with
                 * last=false whenever the message length is an exact
                 * multiple of the block size, corrupting the digest. */
                if (c->blk_len == c->blk_sz) {
                        int ret = SHA_CMN_BAM_xfer_payload(c->bam_ctx,
                                                           c->blk,
                                                           c->blk_sz,
                                                           c->first,
                                                           false,   /* last */
                                                           true);   /* blocking */
                        if (ret)
                                return TEE_ERROR_GENERIC;
                        c->first   = false;
                        c->blk_len = 0;
                }

                size_t space = c->blk_sz - c->blk_len;
                size_t copy  = (len < space) ? len : space;

                memcpy(c->blk + c->blk_len, data, copy);
                c->blk_len += copy;
                data       += copy;
                len        -= copy;
        }
        return TEE_SUCCESS;
}

/*
 * final — flush remaining data with last=true, then extract digest.
 *
 * Calls SHA_CMN_BAM_xfer_payload() with the remaining partial block
 * and last=true, then uses CE_HAL_get_auth_iv() to copy the digest.
 *
 * This replaces uclib's util_blk_buf_flush() + HASH_MAKE_STRING.
 */
static TEE_Result qce_hash_final_ctx(struct crypto_hash_ctx *ctx,
                                     uint8_t *digest, size_t len)
{
        struct qce_hash_ctx *c = to_qce_ctx(ctx);
        CE_SHA_CTX *sha_ctx;
        size_t md_sz;
        int ret;

        /* Transfer remaining data (last=true, blocking=true) */
        ret = SHA_CMN_BAM_xfer_payload(c->bam_ctx,
                                       c->blk,
                                       c->blk_len,
                                       c->first,
                                       true,    /* last */
                                       true);   /* blocking */
        if (ret)
                return TEE_ERROR_GENERIC;

        /* Extract digest via CE_HAL_get_auth_iv() */
        sha_ctx = HASH_GET_DIGEST_CTX(c->bam_ctx, CE_SHA_CTX);
        md_sz   = (len < c->md_sz) ? len : c->md_sz;
        ret = CE_HAL_get_auth_iv(sha_ctx->ce_ctx, digest, md_sz);
        if (ret)
                return TEE_ERROR_GENERIC;

        return TEE_SUCCESS;
}

static void qce_hash_free_ctx(struct crypto_hash_ctx *ctx)
{
        struct qce_hash_ctx *c = to_qce_ctx(ctx);

        if (c->bam_ctx) {
                /* Free CE HAL context — util_blkbuf is not used in direct BAM path */
                CE_SHA_CTX *sha_ctx = HASH_GET_DIGEST_CTX(c->bam_ctx, CE_SHA_CTX);
                if (sha_ctx && sha_ctx->ce_ctx)
                        CE_HAL_free(sha_ctx->ce_ctx);
                free(c->bam_ctx->ctx);
                free(c->bam_ctx);
        }
        free(c);
}

static void qce_hash_copy_state(struct crypto_hash_ctx *dst_ctx,
                                struct crypto_hash_ctx *src_ctx)
{
        /* Not implemented — copy_state requires deep copy of CE HAL state */
        (void)dst_ctx;
        (void)src_ctx;
}

static const struct crypto_hash_ops qce_hash_ops = {
        .init       = qce_hash_init_ctx,
        .update     = qce_hash_update_ctx,
        .final      = qce_hash_final_ctx,
        .free_ctx   = qce_hash_free_ctx,
        .copy_state = qce_hash_copy_state,
};

/* =========================================================================
 * Allocate hash context — allocate hash_ctx_t directly (no uclib handle)
 * =========================================================================
 */
static TEE_Result qce_hash_alloc(struct crypto_hash_ctx **ctx, uint32_t algo)
{
        struct qce_hash_ctx *c;
        UCLIB_HASH_ALG uclib_alg;
        size_t blk_sz, md_sz;
        env_t *env;

        if (tee_alg_to_uclib(algo, &uclib_alg, &blk_sz, &md_sz)) {
                EMSG("QCE hash: unsupported algo 0x%08"PRIx32, algo);
                return TEE_ERROR_NOT_IMPLEMENTED;
        }

        env = ucenv_get_env();
        if (!env) {
                EMSG("QCE hash: ucenv_get_env returned NULL");
                return TEE_ERROR_GENERIC;
        }

        c = calloc(1, sizeof(*c));
        if (!c)
                return TEE_ERROR_OUT_OF_MEMORY;

        /* Allocate hash_ctx_t directly — same as uclib_hash_new() internals */
        c->bam_ctx = calloc(1, sizeof(hash_ctx_t));
        if (!c->bam_ctx) {
                free(c);
                return TEE_ERROR_OUT_OF_MEMORY;
        }

        /* Initialize hash_ctx_t fields required by SHA*_CE_BAM_init() */
        c->bam_ctx->magic  = HASH_CTX_MAGIC;
        c->bam_ctx->env    = env;
        c->bam_ctx->alg    = uclib_alg;
        c->bam_ctx->engine = UCLIB_CE_BAM;

        c->uclib_alg   = uclib_alg;
        c->blk_sz      = blk_sz;
        c->md_sz       = md_sz;
        c->blk_len     = 0;
        c->first       = true;
        c->base.ops    = &qce_hash_ops;
        *ctx = &c->base;

        return TEE_SUCCESS;
}

/* =========================================================================
 * Driver init — register with OP-TEE drvcrypt framework
 * =========================================================================
 */
static TEE_Result qce_hash_driver_init(void)
{
        TEE_Result res;

        res = ucenv_optee_init();
        if (res) {
                EMSG("QCE hash: ucenv_optee_init failed: %#"PRIx32, res);
                return res;
        }

        res = drvcrypt_register_hash(&qce_hash_alloc);
        if (res) {
                EMSG("QCE hash: drvcrypt_register_hash failed: %#"PRIx32, res);
                return res;
        }

        return TEE_SUCCESS;
}

driver_init(qce_hash_driver_init);
