// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE BAM cipher engine implementation.
 *
 * Provides AES, DES, and 3DES cipher processing through
 * the Qualcomm Crypto Engine (QCE) BAM interface.
 */

#include "qce_env.h"
#include "cipher.h"
#include "ce_bam.h"

/*===========================================================================
                 DEFINITIONS AND TYPE DECLARATIONS
 ===========================================================================*/
static int cipher_ce_hal_alg(UCLIB_CIPHER_ALG alg, CE_HW_ALG_t *ce_alg, uint8_t *ce_key_sz)
{
  switch (alg) {
    case UCLIB_DES:    *ce_alg = CE_HAL_DES;  *ce_key_sz = CE_HAL_DES_KEY_SIZE;    return UCLIB_SUCCESS;
    case UCLIB_2DES:   *ce_alg = CE_HAL_2DES; *ce_key_sz = CE_HAL_2DES_KEY_SIZE;   return UCLIB_SUCCESS;
    case UCLIB_3DES:   *ce_alg = CE_HAL_3DES; *ce_key_sz = CE_HAL_3DES_KEY_SIZE;   return UCLIB_SUCCESS;
    case UCLIB_AES128: *ce_alg = CE_HAL_AES;  *ce_key_sz = CE_HAL_AES128_KEY_SIZE; return UCLIB_SUCCESS;
    case UCLIB_AES256: *ce_alg = CE_HAL_AES;  *ce_key_sz = CE_HAL_AES256_KEY_SIZE; return UCLIB_SUCCESS;
    default:           return UCLIB_ERR_NOT_SUPPORTED;
  }
}

static int cipher_ce_hal_mode(UCLIB_CIPHER_MODE mode, CE_HW_MODE_t *ce_mode)
{
  switch (mode) {
    case UCLIB_ECB: *ce_mode = CE_HAL_ECB; return UCLIB_SUCCESS;
    case UCLIB_CBC: *ce_mode = CE_HAL_CBC; return UCLIB_SUCCESS;
    case UCLIB_CTR: *ce_mode = CE_HAL_CTR; return UCLIB_SUCCESS;
    case UCLIB_CTS: *ce_mode = CE_HAL_CTS; return UCLIB_SUCCESS;
    case UCLIB_XTS: *ce_mode = CE_HAL_XTS; return UCLIB_SUCCESS;
    default:        return UCLIB_ERR_NOT_SUPPORTED;
  }
}

/**
 *  @brief Init function for the BAM cipher engine. Generic across all
 *         algorithm/mode combinations (unlike hash, cipher's HW setup is
 *         already fully parameterized by the CE_HW_ALG_t/CE_HW_MODE_t
 *         enums), so one function replaces the vendor's per-symbol
 *         config-generated dispatch table.
 *
 *  @param [in] me Pointer to cipher context. me->alg, me->mode, me->key,
 *              me->key_sz (and, for XTS, me->xts_key/me->xts_key_sz) must
 *              already be set.
 *  @return Success/Error code.
 */
int CIPHER_CE_BAM_init(cipher_ctx_t *me)
{
  int ret;
  CE_HW_ALG_t ce_alg;
  CE_HW_MODE_t ce_mode;
  uint8_t ce_key_sz;

  UC_GUARD(UCLIB_SUCCESS == (ret = cipher_ce_hal_alg(me->alg, &ce_alg, &ce_key_sz)), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = cipher_ce_hal_mode(me->mode, &ce_mode)), ret, me->env);

  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  me->ce_ctx = (struct ce_hal_ctx_s *)CE_HAL_new(me->env);
  ret = UCLIB_ERR_OUT_OF_MEMORY;
  UC_JMP_IF_FALSE(me->ce_ctx, __CIPHER_INIT_EXIT);

  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_HAL_init(me->ce_ctx,
                                                      ce_alg,
                                                      ce_mode,
                                                      0,          /* auth_sz — no auth for plain cipher */
                                                      ce_key_sz,
                                                      CE_HAL_BAM_MODE,
                                                      CE_HAL_LITTLE_ENDIAN)),
                  __CIPHER_INIT_EXIT);

  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_init(me->ce_ctx)), __CIPHER_INIT_EXIT);

  if (me->mode == UCLIB_XTS) {
    UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_HAL_set_xts_key(me->ce_ctx, me->xts_key, me->xts_key_sz)), __CIPHER_INIT_EXIT);
    me->flags |= UCLIB_CIPHER_XTS_KEY_READY;
    /* Data-unit size is set per-call in CIPHER_BAM_cipher() once the real
     * transfer length is known (see comment there) — not here. */
  }

__CIPHER_INIT_EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

/**
 *  @brief BAM encrypt/decrypt transfer — ported ~as-is from the vendor's
 *         CIPHER_CMN_BAM_cipher(). Handles ECB/CBC/CTR/CTS/XTS for
 *         AES/DES/2DES/3DES; the mode-specific behavior (IV vs no-IV,
 *         XTS tweak/DU-size, CTS ciphertext-stealing on the final block)
 *         is entirely inside CE_HAL/CE_BAM — this function is generic.
 *
 *  @param [in]  ctx   Pointer to cipher context (cipher_ctx_t*)
 *  @param [in]  ibuf  Input buffer
 *  @param [in]  isz   Input size in bytes
 *  @param [out] obuf  Output buffer
 *  @param [in]  osz   Output buffer size in bytes (must be >= isz)
 *  @param [in]  first First call for this operation (sets dir/IV/HW key)
 *  @param [in]  last  Last call for this operation
 *  @return Success/Error code.
 */
int CIPHER_BAM_cipher(void *ctx, const uint8_t *ibuf, size_t isz, uint8_t *obuf, size_t osz, bool first, bool last)
{
  cipher_ctx_t *me = (cipher_ctx_t *)ctx;
  int ret = UCLIB_ERR_FAILURE;

  UC_GUARD(osz >= isz, UCLIB_ERR_CIPHER_SMALL_OUT_BUF, me->env);

  size_t bam_max_data_sz = ENV_get_maxBamDataSize(me->env);

  /* Initialize Cipher IV and cipher direction */
  if (first) {
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_cipher_dir(me->ce_ctx, CIPHER_IS_FLAG_SET(me, UCLIB_CIPHER_FLAG_DIRECTION))), ret, me->env);
    if (me->mode != UCLIB_ECB) {
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_cipher_iv(me->ce_ctx, me->iv)), ret, me->env);
    }
    if (me->mode == UCLIB_XTS) {
      /* Single data-unit per operation — the whole buffer passed to this
       * call is one XTS data unit. Must equal the actual transfer length
       * (not a fixed constant): the CE hardware waits for exactly
       * xts_du_sz bytes before it will report the operation complete,
       * so a mismatch here left the engine reporting busy indefinitely
       * on any transfer shorter than the previously-hardcoded 4096. */
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_xts_du_size(me->ce_ctx, isz)), ret, me->env);
      me->flags |= UCLIB_CIPHER_XTS_DU_SIZE_READY;
    }
  }

  /* Synchronization for HW access */
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  do {
    size_t tmp_xfer_sz = (isz > bam_max_data_sz) ? bam_max_data_sz : isz;

    UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_set_ce_ctx(me->ce_ctx,
                                                              0,                 /* auth_start */
                                                              0,                 /* auth_size */
                                                              0,                 /* cipher_start */
                                                              tmp_xfer_sz,       /* cipher_size */
                                                              tmp_xfer_sz,
                                                              me->key,           /* SW Key */
                                                              me->key_sz,        /* SW Key Size */
                                                              first,
                                                              last)),
                    __CIPHER_XFER_EXIT);

    UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_xfer_data(me->ce_ctx,
                                                             ibuf,
                                                             tmp_xfer_sz,
                                                             obuf,
                                                             tmp_xfer_sz)),
                    __CIPHER_XFER_EXIT);

    UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_get_ce_ctx(me->ce_ctx, true, last)), __CIPHER_XFER_EXIT);

    ibuf += tmp_xfer_sz;
    obuf += tmp_xfer_sz;
    isz -= tmp_xfer_sz;

    first = false;
  } while (isz);

__CIPHER_XFER_EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

/**
 *  @brief Frees the CE HAL context allocated by CIPHER_CE_BAM_init().
 */
int CIPHER_BAM_cleanup(cipher_ctx_t *me)
{
  if (me->ce_ctx)
    return CE_HAL_free(me->ce_ctx);
  return UCLIB_SUCCESS;
}
