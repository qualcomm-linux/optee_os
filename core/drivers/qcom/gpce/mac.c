// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE BAM MAC engine implementation.
 *
 * Provides AES-CMAC support through the GPCE BAM interface.
 */

#include "qce_env.h"
#include "cipher.h"
#include "ce_bam.h"

/* AES-CMAC key-size codes reuse the same CE_HAL_AES*_KEY_SIZE constants
 * cipher.c's cipher_ce_hal_alg() uses for plain AES — kept local here
 * since that helper is static to cipher.c. */
static int cmac_ce_hal_key_sz(UCLIB_CIPHER_ALG alg, uint8_t *ce_key_sz)
{
  switch (alg) {
    case UCLIB_AES128: *ce_key_sz = CE_HAL_AES128_KEY_SIZE; return UCLIB_SUCCESS;
    case UCLIB_AES256: *ce_key_sz = CE_HAL_AES256_KEY_SIZE; return UCLIB_SUCCESS;
    default:           return UCLIB_ERR_NOT_SUPPORTED;
  }
}

#define CMAC_AES_BLOCK_SIZE  16

/**
 *  @brief Init function for the BAM AES-CMAC engine. me->alg, me->key,
 *         me->key_sz must already be set (mode, xts_key, xts_key_sz, iv
 *         and flags are unused for MAC — cipher_ctx_t is reused as-is,
 *         not extended).
 */
int CMAC_CE_BAM_init(cipher_ctx_t *me)
{
  int ret;
  uint8_t ce_key_sz;

  UC_GUARD(UCLIB_SUCCESS == (ret = cmac_ce_hal_key_sz(me->alg, &ce_key_sz)), ret, me->env);

  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  me->ce_ctx = (struct ce_hal_ctx_s *)CE_HAL_new(me->env);
  ret = UCLIB_ERR_OUT_OF_MEMORY;
  UC_JMP_IF_FALSE(me->ce_ctx, __CMAC_INIT_EXIT);

  UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_HAL_init(me->ce_ctx,
                                                      CE_HAL_AES,
                                                      CE_HAL_CMAC,
                                                      (CMAC_AES_BLOCK_SIZE - 1), /* auth_sz */
                                                      ce_key_sz,
                                                      CE_HAL_BAM_MODE,
                                                      CE_HAL_LITTLE_ENDIAN)),
                  __CMAC_INIT_EXIT);

  ret = CE_BAM_init(me->ce_ctx);

__CMAC_INIT_EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

/**
 *  @brief BAM CMAC transfer — pushes data through the CE auth engine.
 *         Mirrors hash.c's SHA_CMN_BAM_xfer_payload() exactly, except the
 *         SW key is always me->key (unconditional, unlike hash's
 *         HMAC-only-when-set c->hmac_key).
 *
 *  @param [in] ctx   Pointer to cipher context (cipher_ctx_t*), reused for MAC
 *  @param [in] buf   Input buffer
 *  @param [in] sz    Input size in bytes
 *  @param [in] first First call for this MAC operation
 *  @param [in] last  Last call for this MAC operation
 *  @param [in] blocking Whether to block for CE completion
 *  @return Success/Error code.
 */
int CMAC_BAM_push(void *ctx, const uint8_t *buf, size_t sz, bool first, bool last, bool blocking)
{
  cipher_ctx_t *me = (cipher_ctx_t *)ctx;
  int ret = UCLIB_ERR_FAILURE;

  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_lock(me->env), UCLIB_ERR_MUTEX_LOCK, me->env);

  size_t bam_max_data_sz = ENV_get_maxBamDataSize(me->env);
  do {
    size_t xfer_sz = (sz > bam_max_data_sz) ? bam_max_data_sz : sz;

    UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_set_ce_ctx(me->ce_ctx,
                                                              0,          /* auth_start */
                                                              xfer_sz,    /* auth_size */
                                                              0,          /* cipher_start */
                                                              0,          /* cipher_size */
                                                              xfer_sz,
                                                              me->key,    /* SW key */
                                                              me->key_sz,
                                                              first,
                                                              last)),
                    __CMAC_XFER_EXIT);

    UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_xfer_data(me->ce_ctx,
                                                             buf,
                                                             xfer_sz,
                                                             NULL,       /* *obuf */
                                                             0)),        /* osz */
                    __CMAC_XFER_EXIT);

    UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_get_ce_ctx(me->ce_ctx, blocking, last)), __CMAC_XFER_EXIT);

    buf += xfer_sz;
    sz  -= xfer_sz;

    first = false;
  } while (sz);

__CMAC_XFER_EXIT:
  UC_GUARD(UCLIB_SUCCESS == ENV_mutex_release(me->env), UCLIB_ERR_MUTEX_RELEASE, me->env);
  return ret;
}

/**
 *  @brief Frees the CE HAL context allocated by CMAC_CE_BAM_init().
 */
int CMAC_BAM_cleanup(cipher_ctx_t *me)
{
  if (me->ce_ctx)
    return CE_HAL_free(me->ce_ctx);
  return UCLIB_SUCCESS;
}
