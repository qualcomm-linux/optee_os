// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE BAM hash engine implementation.
 *
 * Provides SHA and HMAC processing through the
 * Qualcomm Crypto Engine (QCE) BAM interface.
 */

#include "hash.h"
#include "qce_env.h"
#include "ce_bam.h"

/*===========================================================================
                       SHA Common CE BAM APIs
===========================================================================*/
int SHA_CMN_BAM_xfer_payload(void *ctx, const uint8_t *buf, size_t sz, bool first, bool last, bool blocking)
{
  hash_ctx_t *me = CAST_PTR_TYPE(ctx, hash_ctx_t);
  UC_GUARD(HASH_IS_CTX_VALID(me), UCLIB_ERR_INVALID_CONTEXT, NULL);
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  /* Set CE Context */
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
  int ret = UCLIB_ERR_INV_DGST_CONTEXT;
  UC_JMP_IF_FALSE(HASH_IS_DGST_CTX_VALID(me, sizeof(CE_SHA_CTX)), __EXIT);

  size_t bam_max_data_sz = ENV_get_maxBamDataSize(me->env);
  do {
    size_t xfer_sz = (sz > bam_max_data_sz)?bam_max_data_sz:sz;
    UC_JMP_IF_FALSE (UCLIB_SUCCESS == (ret = CE_BAM_set_ce_ctx(c->ce_ctx,
                                                               0,                         // auth_start
                                                               xfer_sz,                   // auth_size
                                                               0,                         // cipher_start
                                                               0,                         // cipher_size
                                                               xfer_sz,
                                                               c->hmac_key,               // SW Key
                                                               c->hmac_key?me->iblk_sz:0, // SW Key Size
                                                               first,
                                                               last)),
                     __EXIT);

    /* Transfer data to CE */
    UC_JMP_IF_FALSE (UCLIB_SUCCESS == (ret = CE_BAM_xfer_data(c->ce_ctx,
                                                              buf,
                                                              xfer_sz,
                                                              NULL,      // *obuf
                                                              0)),       // osz
                     __EXIT);

    /* Get CE Context */
    UC_JMP_IF_FALSE (UCLIB_SUCCESS == (ret = CE_BAM_get_ce_ctx(c->ce_ctx, blocking, last)), __EXIT);

    buf += xfer_sz;
    sz -= xfer_sz;

    first = false;
  } while (sz);

__EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

/* Map a plain-hash UCLIB_HASH_ALG to the CE_HAL alg/auth-size constants
 * already used by that algorithm's own SHAx_CE_BAM_INIT() — SHA2_224
 * shares SHA2_256's HW alg select (FIPS 180-4: SHA-224 is SHA-256 with a
 * different IV and truncated output, both already handled elsewhere). */
static int hash_ce_hal_hmac_alg(UCLIB_HASH_ALG alg, CE_HW_ALG_t *ce_alg, uint32_t *auth_sz)
{
  switch (alg) {
    case UCLIB_SHA1:     *ce_alg = CE_HAL_SHA1;   *auth_sz = CE_HAL_SHA1_AUTH_SIZE;   return UCLIB_SUCCESS;
    /* Unlike plain hash mode (where software programs the auth_iv
     * registers explicitly via CE_HAL_set_auth_iv, so SHA224 and SHA256
     * can share one alg-select code), HMAC mode has the CE hardware
     * auto-load its own internal default IV purely from the alg-select
     * value — so SHA224 needs its own distinct CE_HAL_SHA224 code here,
     * or it silently computes HMAC using SHA256's IV instead (confirmed
     * by a real HMAC-SHA224 mismatch on target before this fix). The
     * AUTH_SIZE code is still shared — it selects the 64-byte-block
     * "family", not the per-digest IV/truncation. */
    case UCLIB_SHA2_224: *ce_alg = CE_HAL_SHA224; *auth_sz = CE_HAL_SHA256_AUTH_SIZE; return UCLIB_SUCCESS;
    case UCLIB_SHA2_256: *ce_alg = CE_HAL_SHA256; *auth_sz = CE_HAL_SHA256_AUTH_SIZE; return UCLIB_SUCCESS;
    case UCLIB_SHA2_384: *ce_alg = CE_HAL_SHA384; *auth_sz = CE_HAL_SHA384_AUTH_SIZE; return UCLIB_SUCCESS;
    case UCLIB_SHA2_512: *ce_alg = CE_HAL_SHA512; *auth_sz = CE_HAL_SHA512_AUTH_SIZE; return UCLIB_SUCCESS;
    default:              return UCLIB_ERR_NOT_SUPPORTED;
  }
}

/**
 *  @brief Switches an already-initialized plain-hash BAM context into
 *         HMAC mode. Caller must have already run the algorithm's normal
 *         SHAx_CE_BAM_init() (so me->ce_ctx exists) and must pass a key
 *         exactly me->iblk_sz bytes long (RFC 2104 normalization — hash
 *         down if longer, zero-pad if shorter — is the glue layer's job,
 *         same split of responsibility as the vendor's hmac_ce.c wrapper
 *         vs. this engine's ctrl path).
 *
 *         No IV re-init is needed: the CE hardware performs the full
 *         HMAC ipad/opad/double-hash construction internally once given
 *         the raw key and CE_HAL_HMAC mode (confirmed against the
 *         vendor's ce_bam.c register-programming path — no ipad/opad
 *         buffer or extra hashing exists in the uclib software layer).
 */
int HASH_CE_BAM_set_hmac_key(hash_ctx_t *me, const uint8_t *key, size_t key_len)
{
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
  CE_HW_ALG_t ce_alg;
  uint32_t auth_sz;
  int ret;

  UC_GUARD(key_len == me->iblk_sz, UCLIB_ERR_INVALID_ARG, me->env);
  UC_GUARD(key_len <= sizeof(c->hmac_key_buf), UCLIB_ERR_INVALID_ARG, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = hash_ce_hal_hmac_alg(me->alg, &ce_alg, &auth_sz)), ret, me->env);

  UC_GUARD(key_len == ENV_sec_memcpy(me->env, c->hmac_key_buf, sizeof(c->hmac_key_buf), key, key_len),
           UCLIB_ERR_INVALID_ARG, me->env);
  c->hmac_key = c->hmac_key_buf;

  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_init(c->ce_ctx,
                                               ce_alg,
                                               CE_HAL_HMAC,
                                               auth_sz,
                                               0,
                                               CE_HAL_BAM_MODE,
                                               CE_HAL_LITTLE_ENDIAN)),
           ret, me->env);

  return UCLIB_SUCCESS;
}

/*===========================================================================
                       SHA-1 using CE BAM Engine
===========================================================================*/
static int SHA1_BAM_iv_init(hash_ctx_t *me)
{
  // standard initialization vector for SHA-1, source: FIPS 180-2 (little endian)
  int ret;
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 0, INIT_DATA_CE_SHA1_h0)), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 1, INIT_DATA_CE_SHA1_h1)), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 2, INIT_DATA_CE_SHA1_h2)), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 3, INIT_DATA_CE_SHA1_h3)), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 4, INIT_DATA_CE_SHA1_h4)), ret, me->env);

  return UCLIB_SUCCESS;
}

static int SHA1_CE_BAM_INIT(hash_ctx_t *me)
{
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  // Enable CE HW Clock
  int ret;

  // 64-byte context buffer
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);

  // Create CE HAL context
  c->ce_ctx = CE_HAL_new(me->env);
  ret = UCLIB_ERR_OUT_OF_MEMORY;
  UC_JMP_IF_FALSE(c->ce_ctx, __INIT_EXIT);

  // Set up HW CE configuration register if engine is CE HWIO
  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_HAL_init(c->ce_ctx,
                                                      CE_HAL_SHA1,
                                                      CE_HAL_HASH,
                                                      CE_HAL_SHA1_AUTH_SIZE,
                                                      0,
                                                      CE_HAL_BAM_MODE,
                                                      CE_HAL_LITTLE_ENDIAN)),
                  __INIT_EXIT);

  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_init(c->ce_ctx)), __INIT_EXIT);

  // standard initialization vector for SHA-1, source: FIPS 180-2 (little endian)
  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = SHA1_BAM_iv_init(me)), __INIT_EXIT);

  // Set up HW CE configuration register
  ret = CE_BAM_init(c->ce_ctx);

__INIT_EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

static int SHA1_BAM_reset(hash_ctx_t *me)
{
  int ret;
  return SHA1_BAM_iv_init(me);
}

int SHA1_CE_BAM_init (hash_ctx_t *me)
{
  // Initialize SHA1 BAM engine context for the first time
  me->md_sz = SHA_DIGEST_LENGTH;
  if (!HASH_IS_DGST_CTX_VALID(me, sizeof(CE_SHA_CTX))) {
    HASH_NEW_CTX(me,CE_SHA_CTX);
  } else {
    CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
    int ret;
    if (c->ce_ctx)
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_free(c->ce_ctx)), ret, me->env);
    ENV_mem_clear(me->env, me->ctx, sizeof(CE_SHA_CTX));
  }

  /* Assign environment BAM Driver Context to cipher context */
  me->iblk_sz = CE_MAX_BLOCK_SIZE;
  me->reset = SHA1_BAM_reset;

  // Initialize internal context
  int ret;
  UC_GUARD(UCLIB_SUCCESS == (ret = SHA1_CE_BAM_INIT(me)), ret, me->env);

  return UCLIB_SUCCESS;
}

/*===========================================================================
                    SHA2-256/224 using CE BAM Engine
===========================================================================*/
static int SHA2256_BAM_iv_init(hash_ctx_t *me)
{
  // standard initialization vector for SHA-256/224, source: FIPS 180-2 (little endian)
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
  int ret;
  if (UCLIB_SHA2_256 == me->alg) {
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 0, INIT_DATA_CE_SHA256_h0)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 1, INIT_DATA_CE_SHA256_h1)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 2, INIT_DATA_CE_SHA256_h2)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 3, INIT_DATA_CE_SHA256_h3)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 4, INIT_DATA_CE_SHA256_h4)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 5, INIT_DATA_CE_SHA256_h5)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 6, INIT_DATA_CE_SHA256_h6)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 7, INIT_DATA_CE_SHA256_h7)), ret, me->env);
  } else {
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 0, INIT_DATA_CE_SHA224_h0)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 1, INIT_DATA_CE_SHA224_h1)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 2, INIT_DATA_CE_SHA224_h2)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 3, INIT_DATA_CE_SHA224_h3)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 4, INIT_DATA_CE_SHA224_h4)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 5, INIT_DATA_CE_SHA224_h5)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 6, INIT_DATA_CE_SHA224_h6)), ret, me->env);
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 7, INIT_DATA_CE_SHA224_h7)), ret, me->env);
  }

  return UCLIB_SUCCESS;
}

static int SHA2_256_CE_BAM_INIT(hash_ctx_t *me)
{
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  // Enable CE HW Clock
  int ret;

  // 64-byte context buffer
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);

  // Create CE HAL context
  c->ce_ctx = CE_HAL_new(me->env);
  ret = UCLIB_ERR_OUT_OF_MEMORY;
  UC_JMP_IF_FALSE(c->ce_ctx, __INIT_EXIT);

  // Set up HW CE configuration register if engine is CE BAM
  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_HAL_init(c->ce_ctx,
                                                      CE_HAL_SHA256,
                                                      CE_HAL_HASH,
                                                      CE_HAL_SHA256_AUTH_SIZE,
                                                      0,
                                                      CE_HAL_BAM_MODE,
                                                      CE_HAL_LITTLE_ENDIAN)),
                  __INIT_EXIT);

  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_init(c->ce_ctx)), __INIT_EXIT);

  // standard initialization vector for SHA-256, source: FIPS 180-2 (little endian)
  ret = SHA2256_BAM_iv_init(me);

__INIT_EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

static int SHA2256_BAM_reset(hash_ctx_t *me)
{
  int ret;
  return SHA2256_BAM_iv_init(me);
}

int SHA2_256_CE_BAM_init (hash_ctx_t *me)
{
  // Initialize SH2 224/256 bam context for the first time
  if (UCLIB_SHA2_256 == me->alg)
    me->md_sz = SHA256_DIGEST_LENGTH;
  else if (UCLIB_SHA2_224 == me->alg)
    me->md_sz = SHA224_DIGEST_LENGTH;
  else
    return UCLIB_ERR_NOT_SUPPORTED;

  if (!HASH_IS_DGST_CTX_VALID(me, sizeof(CE_SHA_CTX))) {
    HASH_NEW_CTX(me, CE_SHA_CTX);
  } else {
    CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
    int ret;
    if (c->ce_ctx)
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_free(c->ce_ctx)), ret, me->env);
    ENV_mem_clear(me->env, me->ctx, sizeof(CE_SHA_CTX));
  }

  me->iblk_sz = CE_MAX_BLOCK_SIZE;
  me->reset = SHA2256_BAM_reset;

  // Initialize internal context
  int ret;
  UC_GUARD(UCLIB_SUCCESS == (ret = SHA2_256_CE_BAM_INIT(me)), ret, me->env);

  return UCLIB_SUCCESS;
}

/*===========================================================================
                       SHA2-384 using CE BAM Engine
===========================================================================*/
static int SHA2384_BAM_iv_init(hash_ctx_t *me)
{
  int ret;
  // standard initialization vector for SHA2_384, source: FIPS 180-2 (little endian)
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 0, UINT32_C(0x5d9dbbcb))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 1, UINT32_C(0xd89e05c1))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 2, UINT32_C(0x2a299a62))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 3, UINT32_C(0x07d57c36))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 4, UINT32_C(0x5a015991))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 5, UINT32_C(0x17dd7030))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 6, UINT32_C(0xd8ec2f15))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 7, UINT32_C(0x39590ef7))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 8, UINT32_C(0x67263367))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 9, UINT32_C(0x310bc0ff))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 10, UINT32_C(0x874ab48e))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 11, UINT32_C(0x11155868))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 12, UINT32_C(0x0d2e0cdb))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 13, UINT32_C(0xa78ff964))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 14, UINT32_C(0x1d48b547))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 15, UINT32_C(0xa44ffabe))), ret, me->env);

  return UCLIB_SUCCESS;
}

static int SHA2_384_CE_BAM_INIT(hash_ctx_t *me)
{
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  // Enable CE HW Clock
  int ret;

  // 64-byte context buffer
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);

  // Create CE HAL context
  c->ce_ctx = CE_HAL_new(me->env);
  ret = UCLIB_ERR_OUT_OF_MEMORY;
  UC_JMP_IF_FALSE(c->ce_ctx, __INIT_EXIT);

  // Set up HW CE configuration register if engine is CE BAM
  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_HAL_init(c->ce_ctx,
                                                      CE_HAL_SHA384,
                                                      CE_HAL_HASH,
                                                      CE_HAL_SHA384_AUTH_SIZE,
                                                      0,
                                                      CE_HAL_BAM_MODE,
                                                      CE_HAL_LITTLE_ENDIAN)),
                  __INIT_EXIT);

  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_init(c->ce_ctx)), __INIT_EXIT);

  // standard initialization vector for SHA2_384, source: FIPS 180-2 (little endian)
  ret = SHA2384_BAM_iv_init(me);

__INIT_EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

static int SHA2384_BAM_reset(hash_ctx_t *me)
{
  int ret;
  return SHA2384_BAM_iv_init(me);
}

int SHA2_384_CE_BAM_init (hash_ctx_t *me)
{
  // Initialize SH2_384 BAM engine context for the first time
  me->md_sz = SHA2_384_DIGEST_LENGTH;
  if (!HASH_IS_DGST_CTX_VALID(me, sizeof(CE_SHA_CTX))) {
    HASH_NEW_CTX(me, CE_SHA_CTX);
  } else {
    CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
    int ret;
    if (c->ce_ctx)
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_free(c->ce_ctx)), ret, me->env);
    ENV_mem_clear(me->env, me->ctx, sizeof(CE_SHA_CTX));
  }

  me->iblk_sz = (2*CE_MAX_BLOCK_SIZE);
  me->reset = SHA2384_BAM_reset;

  // Initialize internal context
  int ret;
  UC_GUARD(UCLIB_SUCCESS == (ret = SHA2_384_CE_BAM_INIT(me)), ret, me->env);

  return UCLIB_SUCCESS;
}

/*===========================================================================
                       SHA2-512 using CE BAM Engine
===========================================================================*/
static int SHA2512_BAM_iv_init(hash_ctx_t *me)
{
  int ret;
  // standard initialization vector for SHA2_512, source: FIPS 180-2 (little endian)
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 0, UINT32_C(0x67e6096a))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 1, UINT32_C(0x08c9bcf3))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 2, UINT32_C(0x85ae67bb))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 3, UINT32_C(0x3ba7ca84))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 4, UINT32_C(0x72f36e3c))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 5, UINT32_C(0x2bf894fe))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 6, UINT32_C(0x3af54fa5))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 7, UINT32_C(0xf1361d5f))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 8, UINT32_C(0x7f520e51))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 9, UINT32_C(0xd182e6ad))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 10, UINT32_C(0x8c68059b))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 11, UINT32_C(0x1f6c3e2b))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 12, UINT32_C(0xabd9831f))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 13, UINT32_C(0x6bbd41fb))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 14, UINT32_C(0x19cde05b))), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(c->ce_ctx, 15, UINT32_C(0x79217e13))), ret, me->env);

  return UCLIB_SUCCESS;
}

static int SHA2_512_CE_BAM_INIT(hash_ctx_t *me)
{
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  // Enable CE HW Clock
  int ret;

  // 64-byte context buffer
  CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);

  // Create CE HAL context
  c->ce_ctx = CE_HAL_new(me->env);
  ret = UCLIB_ERR_OUT_OF_MEMORY;
  UC_JMP_IF_FALSE(c->ce_ctx, __INIT_EXIT);

  // Set up HW CE configuration register if engine is CE BAM
  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_HAL_init(c->ce_ctx,
                                                      CE_HAL_SHA512,
                                                      CE_HAL_HASH,
                                                      CE_HAL_SHA512_AUTH_SIZE,
                                                      0,
                                                      CE_HAL_BAM_MODE,
                                                      CE_HAL_LITTLE_ENDIAN)),
                  __INIT_EXIT);

  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_init(c->ce_ctx)), __INIT_EXIT);

  // standard initialization vector for SHA2_512, source: FIPS 180-2 (little endian)
  ret = SHA2512_BAM_iv_init(me);

__INIT_EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

static int SHA2512_BAM_reset(hash_ctx_t *me)
{
  int ret;
  return SHA2512_BAM_iv_init(me);
}

int SHA2_512_CE_BAM_init (hash_ctx_t *me)
{
  // Initialize SH2_512 BAM engine context for the first time
  me->md_sz = SHA2_512_DIGEST_LENGTH;
  if (!HASH_IS_DGST_CTX_VALID(me, sizeof(CE_SHA_CTX))) {
    HASH_NEW_CTX(me, CE_SHA_CTX);
  } else {
    CE_SHA_CTX *c = HASH_GET_DIGEST_CTX(me, CE_SHA_CTX);
    int ret;
    if (c->ce_ctx)
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_free(c->ce_ctx)), ret, me->env);
    ENV_mem_clear(me->env, me->ctx, sizeof(CE_SHA_CTX));
  }

  me->iblk_sz = (2*CE_MAX_BLOCK_SIZE);
  me->reset = SHA2512_BAM_reset;

  // Initialize internal context
  int ret;
  UC_GUARD(UCLIB_SUCCESS == (ret = SHA2_512_CE_BAM_INIT(me)), ret, me->env);

  return UCLIB_SUCCESS;
}
