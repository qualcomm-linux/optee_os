// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * Crypto Engine HAL implementation.
 *
 * Provides hardware abstraction for the Qualcomm Crypto Engine
 * including algorithm configuration, context management, key
 * programming, engine status handling, and hardware capability
 * discovery.
 */

#include "qce_env.h"
#include "ce_hal.h"

#define CE_HAL_XTS_DU_SIZE_MAX        0x100000         /* NIST spec: data units size Should be less 2^20 bytes */
#define CE_HAL_XTS_DU_SIZE_MIN        16               /* at least one CIPHER_AES_BLK_SIZE in bytes */

/* 24bits GLOBAL_TIMER_HI_MIRROR + 32 bits GLOBAL_TIMER_LO_MIRROR */
/* Timer in microseconds must expire in fewer than 2^56 19.2 MHz clock cycles */
#define MAX_TIMER_US (((UINT64_C(1) << 56) * 10) / 192)

#define EXTRACT_FIELD(io, field, status) (((status) & CE_HWIO_FMSK(io, field)) >> CE_HWIO_SHFT(io, field))

#define TIMER_TO_U64(hi, lo) ((((uint64_t) (hi)) << 32) | ((uint64_t) (lo)))
#define CLK_CYCLES_TO_US(cycles) (((cycles) * 10) / 192)

/*===========================================================================
                 DEFINITIONS AND TYPE DECLARATIONS
 ===========================================================================*/
static bool CE_HAL_is_eng_supported(ce_hal_ctx_t *me, CE_HW_ALG_t alg, CE_HW_MODE_t mode)
{
  env_t *env = me->env;
  volatile uint32_t eng_avail_msk = 0;
  volatile uint32_t eng_avail2_msk = 0;
  volatile bool is_mode_supported = false;

  switch (alg) {
    case CE_HAL_SHA1:
    case CE_HAL_SHA256:
      eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL , AUTH_SHA_SEL);
      is_mode_supported = true; /* no mode for HASH algorithms */
      break;

    case CE_HAL_SHA384:
    case CE_HAL_SHA512:
      eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL , AUTH_SHA512_SEL);
      is_mode_supported = true; /* no mode for HASH algorithms */
      break;

    case CE_HAL_SHA3_224:
    case CE_HAL_SHA3_256:
    case CE_HAL_SHA3_384:
    case CE_HAL_SHA3_512:
    case CE_HAL_SHAKE128:
    case CE_HAL_SHAKE256:
    case CE_HAL_CSHAKE128:
    case CE_HAL_CSHAKE256:
#ifdef HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_AUTH_SHA3_SEL_BMSK
      eng_avail2_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL2 , AUTH_SHA3_SEL);
      is_mode_supported = true; /* no mode for HASH algorithms */
#else
      is_mode_supported = true; /* no mode for HASH algorithms */
      /* Should check AUTH_SHA3_SEL when it is added in CRYPTO0_CRYPTO_ENGINES_AVAIL2 as it is not defined for lanai yet */
      //eng_avail2_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL2 , AUTH_SHA3_SEL);
      /* the following checking should be removed when AUTH_SHA3_SEL is added in CRYPTO0_CRYPTO_ENGINES_AVAIL2 */
      if (CE_HAL_HMAC != mode && CE_HAL_KMAC != mode) {
        return is_mode_supported;
      }
#endif
      break;

    case CE_HAL_DES:
    case CE_HAL_3DES:
      eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, ENCR_DES_SEL);
      break;

    case CE_HAL_2DES:
      if (me->hw_ver >= CE_HW_VERSION(5,5,2))
        eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, ENCR_DES_SEL);
      break;

    case CE_HAL_AES:
      eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, ENCR_AES_SEL);
      break;

   case CE_HAL_KASUMI:
     eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, ENCR_KASUMI_SEL);
     break;

   case CE_HAL_SNOW3G:
     eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, ENCR_SNOW3G_SEL);
     break;

   case CE_HAL_ZUC:
     eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, ENCR_ZUC_SEL);
     break;

    case CE_HAL_SM3:
      #ifdef HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_ADDR
      if (me->hw_ver >= CE_HW_VERSION(5,7,0) || me->hw_ver == CE_HW_VERSION(5,6,3))
        eng_avail2_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL2, AUTH_SM3_SEL);
      #endif
      break;

    case CE_HAL_SM4:
       #ifdef HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_ADDR
      if (me->hw_ver >= CE_HW_VERSION(5,7,0) || me->hw_ver == CE_HW_VERSION(5,6,3))
        eng_avail2_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL2, ENCR_SM4_SEL);
      #endif
      break;

    default:
      break;
  }

  switch (mode) {
    case CE_HAL_HASH:
    case CE_HAL_ECB:
    case CE_HAL_CBC:
    case CE_HAL_CTS:
    case CE_HAL_CTR:
    case CE_HAL_XTS:
    case CE_HAL_IDSA:
    case CE_HAL_HC_CBC:
    case CE_HAL_HC_CTS:
    case CE_HAL_HC_CTR:
    case CE_HAL_CBC_VA:
    case CE_HAL_CTR_VA:
      is_mode_supported = true;
      break;

    case CE_HAL_HMAC:
    case CE_HAL_KMAC:
      eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, AUTH_SHA_SEL);
      eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, AUTH_SHA512_SEL);

#ifdef HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_AUTH_SM3_SEL_BMSK
      eng_avail2_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL2, AUTH_SM3_SEL);
#endif

#ifdef HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_AUTH_SHA3_SEL_BMSK
      eng_avail2_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL2, AUTH_SHA3_SEL);
#endif

      is_mode_supported = true;
      break;

    case CE_HAL_GCM:
      eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, AUTH_AES_SEL);
      if (me->hw_ver >= CE_HW_VERSION(5,6,0))
        is_mode_supported = true;
      break;

    case CE_HAL_CCM:
    case CE_HAL_CMAC:
      eng_avail_msk |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENGINES_AVAIL, AUTH_AES_SEL);
      is_mode_supported = true;
      break;

    default:
      break;
  }

#ifdef HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_ADDR
    /* There is the register CRYPTO0_CRYPTO_ENGINES_AVAIL2 with these GPCE versions */
    return is_mode_supported && (eng_avail_msk & CE_HWIO_IN(CRYPTO0_CRYPTO_ENGINES_AVAIL) ||
                                 eng_avail2_msk & CE_HWIO_IN(CRYPTO0_CRYPTO_ENGINES_AVAIL2));
#else
    return is_mode_supported && (eng_avail_msk & CE_HWIO_IN(CRYPTO0_CRYPTO_ENGINES_AVAIL));
#endif
}

/* Helper function to clear and free a single wrapped key (auth or encr key) */
static void release_wrapped_key
(
  env_t *env,
  uint8_t **pWrappedKey,
  size_t *pWrappedKeySz
)
{
  if (*pWrappedKey)
  {
    ENV_mem_clear(env, *pWrappedKey, *pWrappedKeySz);
    ENV_mem_free(env, *pWrappedKey);
    *pWrappedKey = NULL;
  }
}

/* Helper function to clear and free the wrapped keys if they exist */
static void release_wrapped_keys(ce_hal_ctx_t *me)
{
  env_t *env = me->env;
  release_wrapped_key(env, &me->auth_wrapped_key, &me->auth_wrapped_key_sz);
  release_wrapped_key(env, &me->encr_wrapped_key, &me->encr_wrapped_key_sz);
}

/**
 * @brief Create a new CE HAL context
 *
 * @param env            [in]    Pointer to env
 *
 * @return pointer to CE HAL context.
 */
ce_hal_ctx_t * CE_HAL_new(env_t *env)
{
  UC_GUARD(env, NULL, NULL);

  ce_hal_ctx_t *ctx = ENV_zalloc(env, sizeof(ce_hal_ctx_t));
  UC_GUARD(ctx, NULL, NULL);

  ctx->env = env;
  ctx->magic_number = CE_HAL_MAGIC_NUMBER;
  return ctx;
}

/**
 * @brief Free CE HAL context
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_free(ce_hal_ctx_t *me)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);

  env_t *env = me->env;
  if (me->container) {
    size_t  cache_line_sz = ENV_dcache_line_size(me->env);
    ENV_mem_clear(env, me->container, 4*cache_line_sz);
  }
  release_wrapped_keys(me);

  ENV_mem_clear(env, me, sizeof(ce_hal_ctx_t));
  ENV_mem_free(env, me);
  return UCLIB_SUCCESS;
}

/**
 * @brief Resets certain fields of CE HAL context which are
 *        updated by engine.
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_reset(ce_hal_ctx_t *me)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t *env = me->env;
  if (me->container) {
    size_t  cache_line_sz = ENV_dcache_line_size(me->env);
    ENV_mem_clear(env, me->container, 4*cache_line_sz);
  }

  ENV_mem_clear(env, me->auth_iv, sizeof(me->auth_iv));
  ENV_mem_clear(env, me->auth_ivm, sizeof(me->auth_ivm));
  ENV_mem_clear(env, me->auth_byte_cnt, sizeof(me->auth_byte_cnt));

  /* not releasing wrapped keys here to allow re-use of the wrapped keys
     using the same handle */

  return UCLIB_SUCCESS;
}

/**
 * @brief Init CE HAL context
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param alg           [in]    Crypto Engine Algorithm
 * @param mode          [in]    Crypto Engine Mode
 * @param auth_sz       [in]    Crypto Engine Auth size [0..15]
 * @param key_sz        [in]    Crypto Engine Key size [0, 2]
 * @param high_speed    [in]    BAM vs. HWIO
 * @param little_endian [in]    Little/Big Endian
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_init(ce_hal_ctx_t *me,
                CE_HW_ALG_t  alg,
                CE_HW_MODE_t mode,
                uint8_t      auth_sz,
                uint8_t      key_sz,
                bool         high_speed,
                bool         little_endian)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD((alg > CE_HAL_ALG_NONE) && (alg < CE_HAL_ALG_MAX), UCLIB_ERR_INVALID_ARG, me->env);
  UC_GUARD((mode > CE_HAL_MODE_NONE) && (mode < CE_HAL_MODE_MAX), UCLIB_ERR_INVALID_ARG, me->env);
  UC_GUARD(auth_sz <= 63, UCLIB_ERR_INVALID_ARG, me->env);
  UC_GUARD(
    !key_sz ||
    (CE_HAL_AES128_KEY_SIZE == key_sz) ||
    (CE_HAL_AES256_KEY_SIZE == key_sz) ||
    (CE_HAL_DES_KEY_SIZE == key_sz) ||
    (CE_HAL_2DES_KEY_SIZE == key_sz) ||
    (CE_HAL_3DES_KEY_SIZE == key_sz) ||
    (CE_HAL_SM4_KEY_SIZE == key_sz),
    UCLIB_ERR_INVALID_ARG,
    me->env);

  env_t *env = me->env;
  me->hw_ver = CE_HWIO_IN(CRYPTO0_CRYPTO_VERSION);
  UC_GUARD(CE_HAL_is_eng_supported(me, alg, mode), UCLIB_ERR_NOT_SUPPORTED, me->env);
  me->alg = alg;
  me->mode = mode;
  me->auth_sz = auth_sz;
  me->key_sz = key_sz;
  if (high_speed)
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_HIGH_SPEED);
  if (little_endian)
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_LITTLE_ENDIAN);

  uint8_t dflt_pipeset_num = ENV_get_default_pipeset_num(me->env);

  uint8_t pipe_pair_max = ENV_BAM_get_ce_bam_pipe_set_cnt(env);
  me->pipe_set_num = (dflt_pipeset_num >= pipe_pair_max)? 0: dflt_pipeset_num;

  me->mask[0] = UINT32_C(0xFFFFFFFF);
  me->mask[1] = UINT32_C(0xFFFFFFFF);
  me->mask[2] = UINT32_C(0xFFFFFFFF);
  me->mask[3] = UINT32_C(0xFFFFFFFF);

  switch (mode) {
    case CE_HAL_HASH:
    case CE_HAL_HMAC:
    case CE_HAL_CMAC:
    case CE_HAL_KMAC:
      CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_AUTH_ENG_EN);
      break;

    case CE_HAL_ECB:
    case CE_HAL_CBC:
    case CE_HAL_CTS:
    case CE_HAL_CTR:
    case CE_HAL_XTS:
    case CE_HAL_IDSA:
    case CE_HAL_CBC_VA:
    case CE_HAL_CTR_VA:
      CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_CIPHER_ENG_EN);
      break;

    case CE_HAL_CCM:
    case CE_HAL_GCM:
    case CE_HAL_HC_CBC:
    case CE_HAL_HC_CTS:
    case CE_HAL_HC_CTR:
      CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_AUTH_ENG_EN);
      CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_CIPHER_ENG_EN);
      break;
    default:
      return UCLIB_ERR_INVALID_ARG;
  }

  return UCLIB_SUCCESS;
}

/**
 * @brief Set Counter Mask
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param msk_idx       [in]    Mask Index
 * @param mask          [in]    Mask Value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_mask(ce_hal_ctx_t *me, uint8_t msk_idx, uint32_t mask)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(msk_idx < 4, UCLIB_ERR_INVALID_ARG, me->env);
  me->mask[msk_idx] = mask;
  return UCLIB_SUCCESS;
}

/**
 * @brief Set CCM init. counter buffer
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param cntr          [in]    Pointer to CCM counter buffer
 * @param cntr_sz       [in]    buffer size in byte
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_ccm_init_cntr(ce_hal_ctx_t *me, uint8_t *cntr, size_t cntr_sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(cntr && cntr_sz, UCLIB_ERR_INVALID_ARG, me->env);
  ENV_sec_memcpy(me->env, me->cntr, sizeof(me->cntr), cntr, cntr_sz);
  return UCLIB_SUCCESS;
}

/**
 * @brief Set nonce
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param nonce         [in]    Pointer to nonce buffer
 * @param nonce_sz      [in]    buffer size in byte
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_nonce(ce_hal_ctx_t *me, uint8_t *nonce, size_t nonce_sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(nonce && nonce_sz, UCLIB_ERR_INVALID_ARG, me->env);
  ENV_sec_memcpy(me->env, me->nonce, sizeof(me->nonce), nonce, nonce_sz);
  return UCLIB_SUCCESS;
}

/**
 * @brief Set expected MAC
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param mac           [in]    Pointer to mac buffer
 * @param mac_sz        [in]    buffer size in byte
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_exp_mac(ce_hal_ctx_t *me, uint8_t *mac, size_t mac_sz)
{
  return UCLIB_ERR_NOT_SUPPORTED;
}

/**
 * @brief Set Auth. Algorithm in Hash Cipher Modes
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param alg           [in]    Hash Algorithm
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_alg(ce_hal_ctx_t *me, CE_HW_ALG_t alg)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD((CE_HAL_SHA1 == alg) ||
           (CE_HAL_SHA224 == alg) ||
           (CE_HAL_SHA256 == alg) ||
           (CE_HAL_SHA384 == alg) ||
           (CE_HAL_SHA512 == alg), UCLIB_ERR_CE_HAL_CTX_INVALID, me->env);

  me->hc_alg = alg;
  return UCLIB_SUCCESS;
}

/**
 * @brief Get Auth. Algorithm in Hash Cipher Modes
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param alg           [in]    Pointer to Hash Algorithm
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_auth_alg(ce_hal_ctx_t *me, CE_HW_ALG_t *alg)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(alg, UCLIB_ERR_CE_HAL_CTX_INVALID, me->env);
  *alg = me->hc_alg;
  return UCLIB_SUCCESS;
}

/**
 * @brief Set Auth. IV
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param idx           [in]    IV array index
 * @param iv            [in]    Initial value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_iv(ce_hal_ctx_t *me, uint8_t idx, uint32_t iv)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(idx < ARRAY_SIZE(me->auth_iv), UCLIB_ERR_INVALID_ARG, me->env);
  me->auth_iv[idx] = iv;
  return UCLIB_SUCCESS;
}

/**
 * @brief Set Auth. IVM
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param idx           [in]    IVM array index
 * @param iv            [in]    Initial value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_ivm(ce_hal_ctx_t *me, uint8_t idx, uint32_t iv)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(idx < ARRAY_SIZE(me->auth_ivm), UCLIB_ERR_INVALID_ARG, me->env);
  me->auth_ivm[idx] = iv;
  return UCLIB_SUCCESS;
}

/**
 * @brief Set Cipher. IV
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param iv            [in]    Pointer to IV member of Cipher
 *                      Context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_cipher_iv(ce_hal_ctx_t *me, uint32_t* iv)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(iv, UCLIB_ERR_INVALID_ARG, me->env);
  me->cipher_iv = iv;
  return UCLIB_SUCCESS;
}

/**
 * @brief Get Cipher. IV
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return Pointer to IV member.
 */
uint32_t *CE_HAL_get_cipher_iv(ce_hal_ctx_t *me)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), NULL, NULL);
  return me->cipher_iv;
}

/**
 * @brief Set Cipher XTS key
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param xts_key       [in]    Pointer to xts_key buffer
 * @param xts_key_sz    [in]    xts_key buffer size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_xts_key(ce_hal_ctx_t *me, uint8_t *xts_key, size_t xts_key_sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(xts_key, UCLIB_ERR_CIPHER_INV_XTS_KEY_PTR, me->env);
  UC_GUARD(xts_key_sz <= MAX_CIPHER_KEY_COUNT, UCLIB_ERR_CIPHER_INV_XTS_KEY_SZ, me->env);
  ENV_mem_clear(me->env, me->xts_key, sizeof(me->xts_key));
  ENV_sec_memcpy(me->env, me->xts_key, sizeof(me->xts_key), xts_key, xts_key_sz);
  return UCLIB_SUCCESS;
}

/**
 * @brief Set Cipher XTS Data Unit size
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param xts_du_sz     [in]    xts_du_size value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_xts_du_size(ce_hal_ctx_t *me, size_t xts_du_sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  size_t bam_max_data_sz = ENV_get_maxBamDataSize(me->env);
  UC_GUARD(((xts_du_sz < CE_HAL_XTS_DU_SIZE_MAX) && (xts_du_sz <= bam_max_data_sz) && (xts_du_sz >= CE_HAL_XTS_DU_SIZE_MIN)), UCLIB_ERR_CIPHER_INV_XTS_DU_SIZE, me->env);
  me->xts_du_sz = xts_du_sz;
  return UCLIB_SUCCESS;
}

/**
 * @brief Get Cipher XTS Data Unit size
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param xts_du_sz     [in]    pointer to xts_du_size value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_xts_du_size(ce_hal_ctx_t *me, size_t *xts_du_sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me) && xts_du_sz, UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  *xts_du_sz = me->xts_du_sz;
  return UCLIB_SUCCESS;
}

/**
 * @brief Get Auth. IV
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param buf           [in]    Pointer to IV buffer
 * @param sz            [in]    IV buffer size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_auth_iv(ce_hal_ctx_t *me, uint8_t *buf, size_t sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(sz <= sizeof(me->auth_iv), UCLIB_ERR_INVALID_ARG, me->env);
  ENV_sec_memcpy(me->env, buf, sz, me->auth_iv, sizeof(me->auth_iv));
  return UCLIB_SUCCESS;
}

/**
 * @brief Get Auth. IVM
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param buf           [in]    Pointer to IV buffer
 * @param sz            [in]    IV buffer size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_auth_ivm(ce_hal_ctx_t *me, uint8_t *buf, size_t sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(sz <= sizeof(me->auth_ivm), UCLIB_ERR_INVALID_ARG, me->env);
  ENV_sec_memcpy(me->env, buf, sz, me->auth_ivm, sizeof(me->auth_ivm));
  return UCLIB_SUCCESS;
}

/**
 * @brief Set Cipher Direction
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param encryption    [in]    true if encryption is enable,
 *                      false otherwise.
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_cipher_dir(ce_hal_ctx_t *me, bool encryption)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  if (encryption)
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_ENCRYPTION);
  else
    CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_IS_ENCRYPTION);
  return UCLIB_SUCCESS;
}

/**
 * @brief Returns CE HW state
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param live          [in]    if true, reads HW CE Status
 *                      register or reads internal copy of HW CE
 *                      Status otherwise.
 *
 * @return 0 if no error registered in CE HW. negative value otherwise.
 */
int CE_HAL_get_ce_state(ce_hal_ctx_t *me, bool live)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t *env = me->env;
  if (!live)
    return me->ce_hw_status;

  me->ce_hw_status = UCLIB_SUCCESS;

  uint32_t ce_status = CE_HWIO_IN(CRYPTO0_CRYPTO_STATUS);

  if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, MAC_FAILED))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_MAC_FAILED;
  }
  else if (ce_status & (CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, ERR_INTR) |
                        CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, SW_ERR)))
  {
    if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, ACCESS_VIOL)) {
      me->ce_hw_status = UCLIB_SUCCESS /* UCLIB_ERR_HAL_ACCESS_VIOL */; /* TODO: Remove WA for UEFITZT failure caused by AC test */
    }
    else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, PIPE_ACTIVE_ERR))
    {
      me->ce_hw_status = UCLIB_ERR_HAL_PIPE_ACTIVE_ERR;
    }
    else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, CFG_CHNG_ERR))
    {
      me->ce_hw_status = UCLIB_ERR_HAL_CFG_CHNG_ERR;
    }
    else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, DOUT_ERR))
    {
      me->ce_hw_status = UCLIB_ERR_HAL_DOUT_ERR;
    }
    else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, DIN_ERR))
    {
      me->ce_hw_status = UCLIB_ERR_HAL_DIN_ERR;
    }
    else
    {
      me->ce_hw_status = UCLIB_ERR_HAL_AXI_ERR;
    }
  }
  else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, HSD_ERR))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_HSD_ERR;
  }
  else if (!(ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, OPERATION_DONE)) ||
            (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, ENCR_BUSY)) ||
            (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS, AUTH_BUSY)))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_CE_BUSY_ERR;
  }

  ce_status = CE_HWIO_IN(CRYPTO0_CRYPTO_STATUS2);
  if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, BIST_ERROR))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_BIST_ERR;
  }
  else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, KEY_ERR))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_KEY_ERR;
  }
  else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, PIPE_KEY_USAGE_ERR))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_PIPE_KEY_USAGE_ERR;
  }
  else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, PIPE_KEY_TIMER_ERR))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_PIPE_KEY_TIMER_ERR;
  }
  else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, PIPE_KEY_PAUSE_ERR))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_PIPE_KEY_PAUSE_ERR;
  }
  else if ((me->hw_ver >= CE_HW_VERSION(5,5,2)) &&
      (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, HW_KEY_USAGE_ERR)))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_HW_KEY_USAGE_ERR;
  }
#ifdef HWIO_CRYPTO0_CRYPTO_STATUS2_HMAC_KEY_ERR_BMSK
  else if ((me->hw_ver >= CE_HW_VERSION(5,9,0)) &&
      (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, HMAC_KEY_ERR)))   // Field HMAC_KEY_ERR[3] added for Pakala
  {
    me->ce_hw_status = UCLIB_ERR_HAL_HMAC_KEY_ERR;
  }
#endif
#ifdef HWIO_CRYPTO0_CRYPTO_STATUS2_INVALID_ALGO_CFG_BMSK
  else if ((me->hw_ver >= CE_HW_VERSION(5,9,0)) &&
      (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, INVALID_ALGO_CFG)))   // Field INVALID_ALGO_CFG[2] added for Pakala
  {
    me->ce_hw_status = UCLIB_ERR_HAL_INVALID_ALGO_CFG_ERR;
  }
#endif
  #if defined HWIO_CRYPTO0_CRYPTO_STATUS5_ADDR && defined CE_050502
  else if (me->hw_ver == CE_HW_VERSION(5,5,2) &&
      (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS2, HMAC_KEY_ERR)))
  {
    me->ce_hw_status = UCLIB_ERR_HAL_HW_KEY_USAGE_ERR;
  }
  #endif
  /* Added status register check for crypto_status5 for Kalpeni also added check for crypto_status4 for Kalpeni and 5.7 and above crypto versions */
  #if defined HWIO_CRYPTO0_CRYPTO_STATUS5_ADDR && defined CE_050502
  if(me->hw_ver == CE_HW_VERSION(5,5,2))
  {
    ce_status = CE_HWIO_IN(CRYPTO0_CRYPTO_STATUS5);
    if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS5, KEY_USAGE_RULE_MISMATCH))
    {
      me->ce_hw_status = UCLIB_ERR_HAL_KEY_USAGE_RULE_MISMATCH;
    }
    else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS5, KEY_INVALIDATION_ERR))
    {
      me->ce_hw_status = UCLIB_ERR_HAL_KEY_INVALIDATION_ERR;
    }
  }
  #endif
  if (me->hw_ver >= CE_HW_VERSION(5,7,0) || me->hw_ver == CE_HW_VERSION(5,5,2))
  {
    if(ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS4, PHK_USAGE_ERR))
    {
      me->ce_hw_status = UCLIB_ERR_HAL_PHK_USAGE_ERR;
    }
    else if (ce_status & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS4, SHK_USAGE_ERR))
    {
      me->ce_hw_status = UCLIB_ERR_HAL_SHK_USAGE_ERR;
    }
  }
  return me->ce_hw_status;
}

/**
 * @brief Set Auth Size
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param auth_sz       [in]    Auth size in byte
 *                      false otherwise.
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_size(ce_hal_ctx_t *me, size_t auth_sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  me->auth_sz = auth_sz;
  return UCLIB_SUCCESS;
}

/**
 * @brief Clear HW Key flags
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_clear_hw_key_flags(ce_hal_ctx_t *me)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_IS_HW_AUTH_KEY);
  CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_IS_HW_CIPHER_KEY);
  CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_IS_SHK_EN);
  CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_IS_PHK_EN);
  return UCLIB_SUCCESS;
}

/**
 * @brief Set HW Key type
 *
 * @param me               [in]    Pointer to CE HAL context
 * @param is_cipher_hw_key [in]    true to use hw key for cipher
 *                         operation.
 * @param is_auth_hw_key   [in]    true to use hw key for auth
 *                         operation.
 * @param is_SHK    [in]    true to use OEM hw key for CE
 *                         operation, false for QC HW key
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_hw_key_type(ce_hal_ctx_t *me, bool is_cipher_hw_key, bool is_auth_hw_key, bool is_SHK)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  CE_HAL_clear_hw_key_flags(me);

  if (is_cipher_hw_key)
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_HW_CIPHER_KEY);
  if (is_auth_hw_key)
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_HW_AUTH_KEY);

  if (is_SHK)
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_SHK_EN);
  else
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_PHK_EN);

  return UCLIB_SUCCESS;
}

/**
 * @brief Set Crypto Operation Mode using Virtual Address
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param op            [in]    Crypto Operation Mode using VA
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_va_op(ce_hal_ctx_t *me, VA_OP_MODE_t op)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  me->va_op = op;
  return UCLIB_SUCCESS;
}

/**
 * @brief Get Crypto Operation Mode using Virtual Address
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param op            [in]    Pointer Crypto Operation Mode
 *                      using VA
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_va_op(ce_hal_ctx_t *me, VA_OP_MODE_t *op)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  *op = me->va_op;
  return UCLIB_SUCCESS;
}

/**
 * @brief Set pipe set number for BAM use cases
 *
 * @param me             [in]  Pointer to CE HAL context
 * @param pipe_set_num   [in]  pipe set number
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_pipe_set_num(ce_hal_ctx_t *me, uint8_t pipe_set_num )
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  uint8_t pipe_pair_max = ENV_BAM_get_ce_bam_pipe_set_cnt(me->env);
  UC_GUARD(pipe_set_num < pipe_pair_max, UCLIB_ERR_INVALID_ARG, NULL);
  me->pipe_set_num = pipe_set_num;
  return UCLIB_SUCCESS;
}

/**
 * @brief Get pipe set number for BAM use cases
 *
 * @param me              [in]  Pointer to CE HAL context
 * @param *pipe_set_num   [in]  pointer to pipe set number
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_pipe_set_num(ce_hal_ctx_t *me, uint8_t *pipe_set_num )
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  *pipe_set_num = me->pipe_set_num;
  return UCLIB_SUCCESS;
}

/**
 * @brief Sets the order of auth and cipher operations.
 *
 * @param me               [in]    Pointer to CE HAL context
 * @param is_before_cipher [in]    If true, auth engine
 *                         processes the data before cipher
 *                         engine, false for reverse order.
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_cipher_order(ce_hal_ctx_t *me, bool is_before_cipher)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  if (is_before_cipher) {
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_HASH_BEFORE_ENCR);
  } else {
    CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_HASH_BEFORE_ENCR);
  }
  return UCLIB_SUCCESS;
}

/**
 * @brief Sets the mode for hc-hmac operation.
 *
 * @param me               [in]    Pointer to CE HAL context
 * @param mode             [in]    Mode for hc operation
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_hc_mode(ce_hal_ctx_t *me, CE_HW_MODE_t mode)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  me->hc_mode = mode;
  CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_HC_KEYS);
  return UCLIB_SUCCESS;
}


/**
 * @brief Copies the CE HAL context from one handle to another
 *
 * @param dest             [in]    Pointer to the destination CE HAL context
 * @param src              [in]    Pointer to the source CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_copy(ce_hal_ctx_t *dest, ce_hal_ctx_t *src)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(src), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t *env = src->env;

  ENV_sec_memcpy(env, dest, sizeof(*dest), src, sizeof(*src));

  /* The below fields are set at the time of a data transfer so do not need
   * to be copied
   *    container
   *    header
   *    header_size
   *    tailer
   *    tailer_size
   *    aligned_obuf
   *    aligned_obuf_size
   *    obuf
   *    obuf_size
   *    spare
   *
   * However in the interest of not leaking any information / not causing
   * confusion by leaving these values populated, we should clear them out
   * here.
   */
  dest->container =
  dest->header =
  dest->tailer =
  dest->aligned_obuf =
  dest->obuf =
  dest->spare =
  NULL;

  dest->header_size =
  dest->tailer_size =
  dest->aligned_obuf_size =
  dest->obuf_size =
  0;

  /* The below fields should be set separately by the client if needed, and
   * should not be copied over as is
   */
  dest->cipher_iv = NULL;

  return UCLIB_SUCCESS;
}

/**
 * Check if HW supports pattern processing
 * @param me [in]  Pointer to the CE HAL context
 *
 * @return true if pattern processing is supported, false otherwise.
 */
bool CE_HAL_are_CE_550_features_supported(ce_hal_ctx_t *me)
{
  env_t *env = me->env;
  return (me->hw_ver >= CE_HW_VERSION(5,5,0)) && ENV_is_use_550_features(env);
}

/**
 * Set pattern processing offset
 * @param me [in]      Pointer to the CE HAL context
 * @param offset [in]  Offset in the pattern to start running the algorithm
 *
 * @return UCLIB_SUCCESS if successful
 */
int CE_HAL_set_patt_proc_offset(ce_hal_ctx_t *me, uint32_t offset)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(CE_HAL_are_CE_550_features_supported(me), UCLIB_ERR_NOT_SUPPORTED, me->env);

  // Not supported in combination with partial block offset
  UC_GUARD(me->partial_block_offset == 0, UCLIB_ERR_NOT_SUPPORTED, me->env);
  UC_GUARD(me->pattern_size >= offset, UCLIB_ERR_INVALID_ARG, me->env);

  me->pattern_offset = offset;

  return UCLIB_SUCCESS;
}

/**
 * Set pattern processing data size
 * @param me [in]         Pointer to the CE HAL context
 * @param data_size [in]  Amount of data to process in the pattern
 *
 * @return UCLIB_SUCCESS if successful
 */
int CE_HAL_set_patt_proc_data_size(ce_hal_ctx_t *me, uint32_t data_size)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(CE_HAL_are_CE_550_features_supported(me), UCLIB_ERR_NOT_SUPPORTED, me->env);

  // Not supported in combination with partial block offset
  UC_GUARD(me->partial_block_offset == 0, UCLIB_ERR_NOT_SUPPORTED, me->env);

  me->process_data_size = data_size;

  return UCLIB_SUCCESS;
}

/**
 * Set pattern processing process data size
 * @param me [in]            Pointer to the CE HAL context
 * @param pattern_size [in]  Size of the pattern
 *
 * @return UCLIB_SUCCESS if successful
 */
int CE_HAL_set_patt_proc_size(ce_hal_ctx_t *me, uint32_t pattern_size)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(CE_HAL_are_CE_550_features_supported(me), UCLIB_ERR_NOT_SUPPORTED, me->env);

  // Not supported in combination with partial block offset
  UC_GUARD(me->partial_block_offset == 0, UCLIB_ERR_NOT_SUPPORTED, me->env);

  me->pattern_size = pattern_size;

  return UCLIB_SUCCESS;
}

/**
 * Toggle CE lock
 *
 * @param me   [in] Pointer to CE HAL context
 * @param lock [in] Whether to lock or unlock
 */
static int CE_HAL_toggle_engine_lock(ce_hal_ctx_t* me, bool lock)
{
  return me->engine_toggle_lock ? me->engine_toggle_lock(me, lock) : UCLIB_ERR_NOT_SUPPORTED;
}

/**
 * Set pipe timer
 * @param me    [in]      Pointer to the CE HAL context
 * @param pipe  [in]      Pipe number
 * @param timer [in]      Number of microseconds before timeout
 *
 * @return UCLIB_SUCCESS if successful
 */
static int CE_HAL_set_pipe_timer(ce_hal_ctx_t *me, int pipe_set, uint64_t timer)
{
#ifdef DRM_PIPE_NOT_SUPPORTED
  /* GPCE ver 5.8.1 not support DRM pipe features */
  return UCLIB_ERR_NOT_SUPPORTED;
#else

  env_t *env = me->env;

  /* Check timer will fit in timeout register and not overflow conversion to
   * clock cycles
   */
  UC_GUARD(timer < MAX_TIMER_US, UCLIB_ERR_INVALID_ARG, me->env);

  UC_GUARD(CE_HAL_toggle_engine_lock(me, true) == UCLIB_SUCCESS, UCLIB_ERR_FAILURE, env);

  CE_HWIO_OUT(CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN, UINT32_C(1));
  uint32_t global_timer_lo = CE_HWIO_IN(CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR);
  uint32_t global_timer_hi = CE_HWIO_IN(CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR);

  UC_GUARD(CE_HAL_toggle_engine_lock(me, false) == UCLIB_SUCCESS, UCLIB_ERR_FAILURE, env);

  uint64_t global_time = (((uint64_t) global_timer_hi) << 32) | (uint64_t) global_timer_lo;

  if (0 == timer)
  {
  #ifdef HWIO_CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_EN_ADDR
    CE_HWIO_OUTI(CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_EN, pipe_set, UINT32_C(0));
  #elif defined HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_ADDR && defined HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_ADDR
    if (pipe_set < 8) {
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYm_TIMER_EN, pipe_set, UINT32_C(0));
    } else {
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYj_TIMER_EN, pipe_set, UINT32_C(0));
    }
  #endif
  }
  else
  {
    /* Convert to 19.2 MHz clock cycles */
    uint64_t clock_cycles = (timer * 192) / 10;

    /* Check for add overflow */
    UC_GUARD(UINT64_MAX - global_time > clock_cycles, UCLIB_ERR_ADD_OVERFLOW, env);

    uint64_t offset_time = global_time + clock_cycles;
    uint32_t timer_hi = (offset_time >> 32) & UINT32_C(0xFFFF);
    uint32_t timer_lo = offset_time & UINT32_C(0xFFFFFFFF);

    UC_GUARD(offset_time < CE_MAX_PIPE_TIMER_VALUE, UCLIB_ERR_INVALID_ARG, env);
  #if defined HWIO_CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_EN_ADDR
    CE_HWIO_OUTI(CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_LIMIT_LO, pipe_set, timer_lo);
    CE_HWIO_OUTI(CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_LIMIT_HI, pipe_set, timer_hi);
    CE_HWIO_OUTI(CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_EN, pipe_set, UINT32_C(1));
  #elif defined HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_ADDR && defined HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_ADDR
    if (pipe_set < 8) {
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO, pipe_set, timer_lo);
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI, pipe_set, timer_hi);
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYm_TIMER_EN, pipe_set, UINT32_C(1));
    } else {
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO, pipe_set, timer_lo);
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI, pipe_set, timer_hi);
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYj_TIMER_EN, pipe_set, UINT32_C(1));
    }
  #endif
  }

  return UCLIB_SUCCESS;
#endif
}

/**
 * Set partial block offset
 * @param me [in]      Pointer to the CE HAL context
 * @param offset [in]  Offset first block to start running the algorithm
 *
 * @return UCLIB_SUCCESS if successful
 */
int CE_HAL_set_partial_block_offset(ce_hal_ctx_t *me, uint32_t offset)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(CE_HAL_are_CE_550_features_supported(me), UCLIB_ERR_NOT_SUPPORTED, me->env);

  // Not supported in combination with pattern processing
  UC_GUARD(me->pattern_size == 0, UCLIB_ERR_NOT_SUPPORTED, me->env);

  me->partial_block_offset = offset;

  return UCLIB_SUCCESS;
}

/**
 * Determine whether crpyto operations will use even or odd keys
 * This is intended to be used to provision odd keys into pipes
 *
 * @param me [in]      Pointer to the CE HAL context
 * @param odd [in]     Whether this context is using odd or even keys
 *
 * @return UCLIB_SUCCESS if successful
 */
int CE_HAL_set_odd_key_usage(ce_hal_ctx_t *me, bool odd)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(CE_HAL_are_CE_550_features_supported(me), UCLIB_ERR_NOT_SUPPORTED, me->env);

  if (odd)
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_USE_ODD_KEYS);
  else
    CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_USE_ODD_KEYS);

  return UCLIB_SUCCESS;
}

/**
 * @brief Checks if the CE HW is ready for a data transfer. This is typically
 * used when there is only a single client in the system, which wants to use
 * the CE HW in BAM mode and in a non-blocking/asynchronous manner.
 *
 * @param me        [in]    Pointer to CE HAL context
 * @param blocking  [in]    Boolean indicating whether to poll for completion
 *
 * @return 0 if the CE HW is ready, error value otherwise.
 */
int CE_HAL_ready_for_xfer(ce_hal_ctx_t *me, bool blocking)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  if (!me->ready)
    return UCLIB_SUCCESS;
  return me->ready(me, blocking);
}

/**
 * @brief Checks if current operation is a Virtual Address mode operation.
 *
 * @param me        [in]    Pointer to CE HAL context
 *
 * @return 'true' if VA operation is ongoing else 'false'.
 */
bool CE_HAL_is_va_op(ce_hal_ctx_t *me)
{
  return (VA_OP_INVALID < me->va_op && MAX_VA_OP_COUNT > me->va_op);
}

/**
 * @brief Gets the CE HW version.
 *
 * @param me        [in]    Pointer to environment
 *
 * @return Crypto Engine HW version.
 */
uint32_t CE_HAL_get_ce_version(env_t *env)
{
  return CE_HWIO_IN(CRYPTO0_CRYPTO_VERSION);
}

/**
 * @brief Checks if Hardware Key Manager functionality is supported
 *
 * @param me        [in]    Pointer to CE HAL context
 *
 * @return true if HW Key Manager functionality is supported, false otherwise
 */
bool CE_HAL_is_hwkm_supported(ce_hal_ctx_t *me)
{
  return (me->hw_ver >= CE_HW_VERSION(5,6,0));
}

/**
 * Store a copy of the provided wrapped key.
 *
 * @param me          [in]  Pointer to the CE HAL context
 * @param key         [in]  Pointer to the wrapped key
 * @param sz          [in]  Length of the wrapped key
 * @param is_auth_key [in]  Is auth or cipher key
 *
 * @return UCLIB_SUCCESS if successful
 */
int CE_HAL_set_wrapped_key
(
  ce_hal_ctx_t *me,
  const uint8_t *key,
  size_t sz,
  bool is_auth_key
)
{
  uint8_t **pWrappedKey = NULL;
  size_t *pWrappedKeySz = 0;

  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD(CE_HAL_is_hwkm_supported(me), UCLIB_ERR_NOT_SUPPORTED, me->env);

  if (is_auth_key)
  {
    pWrappedKey = &me->auth_wrapped_key;
    pWrappedKeySz = &me->auth_wrapped_key_sz;
  }
  else
  {
    pWrappedKey = &me->encr_wrapped_key;
    pWrappedKeySz = &me->encr_wrapped_key_sz;
  }

  /* Clear earlier keys */
  if (sz != *pWrappedKeySz)
  {
    if (*pWrappedKey)
    {
      ENV_mem_free(me->env, *pWrappedKey);
      *pWrappedKey = NULL;
    }
    *pWrappedKeySz = 0;
  }

  UC_PASS(0 == sz, UCLIB_SUCCESS);

  /* Allocate space for new key */
  if (!(*pWrappedKey))
  {
    *pWrappedKey = ENV_zalloc(me->env, sz);
    UC_GUARD(*pWrappedKey, UCLIB_ERR_OUT_OF_MEMORY, me->env);
  }

  *pWrappedKeySz = sz;

  /* Copy over new key */
  ENV_sec_memcpy(
    me->env,
    *pWrappedKey,
    *pWrappedKeySz,
    key,
    sz);

  return UCLIB_SUCCESS;
}

/**
 * Returns whether a wrapped key has been provided or not
 *
 * @param me          [in]  Pointer to the CE HAL context
 * @param is_auth_key [in]  Is auth or cipher key
 *
 * @return true if a wrapped key was provided, false otherwise.
 */
bool CE_HAL_wrapped_key_present(ce_hal_ctx_t *me, bool is_auth_key)
{
  return
    is_auth_key ?
    (NULL != me->auth_wrapped_key) :
    (NULL != me->encr_wrapped_key);
}

/**
 * Reads key pipe status.
 *
 * @param me            [in] Pointer to CE HAL context.
 * @param pipe_set_num  [in] Pipe-set number
 * @param status        [in] Pointer to key pipe status container.
 *
 * @return UCLIB_SUCCESS if successful.
 */
static int CE_HAL_get_pipe_status
(
  ce_hal_ctx_t* me,
  int pipe_set_num,
  uint32_t *status
)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t* env = me->env;

  /* (pipe_set_num % 8) is for the pipe set greater than 8 */
  uint32_t mask = UINT32_C(1) << (pipe_set_num % 8);
  *status = 0;

  uint32_t status3 = CE_HWIO_IN(CRYPTO0_CRYPTO_STATUS3);
#if defined CE_VER_0507_N_BELOW || defined CE_050502 //The bit fields we are checking below are for 5.7.0 and below CE versions and these fields got changed in Lanai(5.8.0) and above.
  if ((EXTRACT_FIELD(CRYPTO0_CRYPTO_STATUS3, PIPE_KEY_PAUSE_DET_MIRROR_VEC, status3) & mask) != 0)
  {
    *status |= UCLIB_CIPHER_PIPE_KEY_PAUSE_DET;
  }

  if ((EXTRACT_FIELD(CRYPTO0_CRYPTO_STATUS3, PIPE_KEY_TIMER_EXPIRED_VEC, status3) & mask) != 0)
  {
    *status |= UCLIB_CIPHER_PIPE_KEY_TIMER_EXPIRED;
  }
#ifdef HWIO_CRYPTO0_CRYPTO_STATUS6_ADDR //The bit fields we are checking below are for 5.7.0 and below CE versions and these fields got changed in Lanai(5.8.0) and above.
  if (CE_HW_VERSION(5,6,3) == me->hw_ver || me->hw_ver >= CE_HW_VERSION(5,7,0))
  {
    uint32_t status6 = CE_HWIO_IN(CRYPTO0_CRYPTO_STATUS6);
    if ((EXTRACT_FIELD(CRYPTO0_CRYPTO_STATUS6, PIPE_KEY_PAUSE_DET_MIRROR_VEC, status6) & mask) != 0)
    {
      *status |= UCLIB_CIPHER_PIPE_KEY_PAUSE_DET;
    }

    if ((EXTRACT_FIELD(CRYPTO0_CRYPTO_STATUS6, PIPE_KEY_TIMER_EXPIRED_VEC, status6) & mask) != 0)
    {
      *status |= UCLIB_CIPHER_PIPE_KEY_TIMER_EXPIRED;
    }
  }
#endif
#elif defined DRM_PIPE_NOT_SUPPORTED
  if ((EXTRACT_FIELD(CRYPTO0_CRYPTO_STATUS3, PIPE_KEY_PAUSE_DET_MIRROR, status3) & mask) != 0)
  {
    *status |= UCLIB_CIPHER_PIPE_KEY_PAUSE_DET;
  }

  if ((EXTRACT_FIELD(CRYPTO0_CRYPTO_STATUS3, PIPE_KEY_TIMER_EXPIRED, status3) & mask) != 0)
  {
    *status |= UCLIB_CIPHER_PIPE_KEY_TIMER_EXPIRED;
  }
#else
  // for CE ver >= 5.8.0
  uint32_t key_index = EXTRACT_FIELD(CRYPTO0_CRYPTO_STATUS3, KEY_INDEX, status3);
  uint32_t pause_det = (pipe_set_num < 8) ? CE_HWIO_INI(CRYPTO0_CRYPTO_KEYm_PAUSE_DET, pipe_set_num) :
	                                        CE_HWIO_INI(CRYPTO0_CRYPTO_KEYj_PAUSE_DET, pipe_set_num);
  uint32_t global_lo = CE_HWIO_IN(CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR);
  uint32_t global_hi = CE_HWIO_IN(CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR);
  uint32_t pipe_lo = (pipe_set_num < 8) ? CE_HWIO_INI(CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO, pipe_set_num) :
                                          CE_HWIO_INI(CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO, pipe_set_num);
  uint32_t pipe_hi = (pipe_set_num < 8) ? CE_HWIO_INI(CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI, pipe_set_num) :
                                          CE_HWIO_INI(CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI, pipe_set_num);
  uint64_t global_time = TIMER_TO_U64(global_hi, global_lo);
  uint64_t timeout = TIMER_TO_U64(pipe_hi, pipe_lo);

  if ((key_index == pipe_set_num && (status3 & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS3, PIPE_KEY_PAUSE_DET_MIRROR)) != 0) ||
      (0 != pause_det))
  {
    *status |= UCLIB_CIPHER_PIPE_KEY_PAUSE_DET;
  }

  if ((key_index == pipe_set_num && (status3 & CE_HWIO_FMSK(CRYPTO0_CRYPTO_STATUS3, PIPE_KEY_TIMER_EXPIRED)) != 0) ||
      (timeout <= global_time))
  {
    *status |= UCLIB_CIPHER_PIPE_KEY_TIMER_EXPIRED;
  }
#endif

  return UCLIB_SUCCESS;
}

/**
 * Get the pipe statistic in microseconds
 *
 * @param me    [in]  Pointer to CE HAL context
 * @param stat  [in]  Requested statistic
 * @param pipe  [in]  Pipe-set number
 * @param out   [out] Microseconds since first use
 *
 * @return UCLIB_SUCCESS if successful.
 */
static int CE_HAL_get_pipe_statistic
(
  ce_hal_ctx_t* me,
  UCLIB_CIPHER_PARAM_TYPE stat,
  int pipe_set_num,
  uint64_t* out
)
{
#ifdef DRM_PIPE_NOT_SUPPORTED
  *out = 0;
  return UCLIB_ERR_NOT_SUPPORTED;
#else

  int status = UCLIB_SUCCESS;
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t *env = me->env;
  uint64_t clock_cycles = 0;

  // Need to lock CE to avoid NoC error caused by HWIO accesses to CE registers
  // while CE is locked by another execution environment
  UC_GUARD(
    UCLIB_SUCCESS == (status = CE_HAL_toggle_engine_lock(me, true)),
    status,
    me->env);

  switch (stat)
  {
    case UCLIB_CIPHER_GET_PIPE_FIRST_USE:
    {
      uint32_t lo = 0;
      uint32_t hi = 0;

    #ifdef HWIO_CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_FIRST_USE_TIMER_LO_ADDR
      lo = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_FIRST_USE_TIMER_LO, pipe_set_num);
      hi = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_FIRST_USE_TIMER_HI, pipe_set_num);
    #elif defined HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_ADDR && defined HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_ADDR
      if (pipe_set_num < 8) {
        lo = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO, pipe_set_num);
        hi = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI, pipe_set_num);
      } else {
        lo = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO, pipe_set_num);
        hi = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI, pipe_set_num);
      }
    #endif
      clock_cycles = TIMER_TO_U64(hi, lo);
      break;
    }
    case UCLIB_CIPHER_GET_PIPE_LAST_USE:
    {
      uint32_t lo = 0;
      uint32_t hi = 0;

    #ifdef HWIO_CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_LAST_USE_TIMER_LO_ADDR
      lo = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_LAST_USE_TIMER_LO, pipe_set_num);
      hi = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_LAST_USE_TIMER_HI, pipe_set_num);
    #elif defined HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_ADDR && defined HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_ADDR
      if (pipe_set_num < 8) {
        lo = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO, pipe_set_num);
        hi = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI, pipe_set_num);
      } else {
        lo = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO, pipe_set_num);
        hi = CE_HWIO_INI(CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI, pipe_set_num);
      }
    #endif
      clock_cycles = TIMER_TO_U64(hi, lo);
      break;
    }
    case UCLIB_CIPHER_GET_PIPE_TIME_LEFT:
    {
      uint32_t global_lo = CE_HWIO_IN(CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR);
      uint32_t global_hi = CE_HWIO_IN(CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR);
      uint32_t pipe_lo = 0;
      uint32_t pipe_hi = 0;

    #ifdef HWIO_CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_LIMIT_LO_ADDR
      pipe_lo = CE_HWIO_INI(CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_LIMIT_LO, pipe_set_num);
      pipe_hi = CE_HWIO_INI(CRYPTO0_CRYPTO_PIPEm_KEY_TIMER_LIMIT_HI, pipe_set_num);
    #elif defined HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_ADDR && defined HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_ADDR
      if (pipe_set_num < 8) {
        pipe_lo = CE_HWIO_INI(CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO, pipe_set_num);
        pipe_hi = CE_HWIO_INI(CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI, pipe_set_num);
      } else {
        pipe_lo = CE_HWIO_INI(CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO, pipe_set_num);
        pipe_hi = CE_HWIO_INI(CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI, pipe_set_num);
      }
    #endif
      uint64_t global_time = TIMER_TO_U64(global_hi, global_lo);
      uint64_t timeout = TIMER_TO_U64(pipe_hi, pipe_lo);

      clock_cycles = (timeout > global_time) ? (timeout - global_time) : 0;
      break;
    }

    default:
      break;
  }

  // Unlock when done reading from CE registers
  UC_GUARD(
    UCLIB_SUCCESS == (status = CE_HAL_toggle_engine_lock(me, false)),
    status,
    me->env);

  *out = CLK_CYCLES_TO_US(clock_cycles);
  return status;
#endif
}

/**
 * Configure key pause.
 *
 * @param me            [in] Pointer to CE HAL context. Should have all configuration options desired for the pipe.
 * @param pipe_set_num  [in] Pipe-set number
 * @param pause_mask    [in] Pipe Pause Config.
 *
 * @return UCLIB_SUCCESS if successful.
 */
static int CE_HAL_set_key_pause_cfg
(
  ce_hal_ctx_t* me,
  int pipe_set_num,
  uint32_t pause_mask
)
{
#ifdef DRM_PIPE_NOT_SUPPORTED
  return UCLIB_ERR_NOT_SUPPORTED;
#else

  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t *env = me->env;

  uint32_t pipe_pause_cfg = 0;

  // bit 4 : S/W source - write of a 1 to the CRYPTO_KEY_PAUSE_SW_SRC register
  if ((pause_mask & UCLIB_CIPHER_PIPE_KEY_PAUSE_SW) != 0)
  {
    if (me->hw_ver >= CE_HW_VERSION(5, 6, 1) || me->hw_ver == CE_HW_VERSION(5, 5, 2) )
    {
      SET_U32_BIT(pipe_pause_cfg, 16);
    }
    else
    {
      SET_U32_BIT(pipe_pause_cfg, 4);
    }
  }

  #ifdef HWIO_CRYPTO0_CRYPTO_PIPEm_KEY_PAUSE_CFG_ADDR
  CE_HWIO_OUTI(CRYPTO0_CRYPTO_PIPEm_KEY_PAUSE_CFG, pipe_set_num, pipe_pause_cfg);
  #else
  if (pipe_set_num < 8) {
    CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYm_PAUSE_CFG, pipe_set_num, pipe_pause_cfg);
  } else {
    CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYj_PAUSE_CFG, pipe_set_num, pipe_pause_cfg);
  }
  #endif

  return UCLIB_SUCCESS;
#endif
}

/**
 * Set pause event
 *
 * @param me            [in]  Pointer to CE HAL context
 * @param pipe_set_num  [in]  Pipe-set number
 * @param val           [in]  Pause value indicating pause or un-pause
 *
 * @return UCLIB_SUCCESS if successful.
 */
static int CE_HAL_set_pause
(
  ce_hal_ctx_t* me,
  int pipe_set_num,
  uint32_t val
)
{
#ifdef DRM_PIPE_NOT_SUPPORTED
  return UCLIB_ERR_NOT_SUPPORTED;
#else
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t *env = me->env;

  UC_GUARD(
    CE_HAL_toggle_engine_lock(me, true) == UCLIB_SUCCESS,
    UCLIB_ERR_FAILURE,
    env);

  switch (val)
  {
    case UCLIB_CIPHER_PIPE_UNPAUSE:
    #ifdef HWIO_CRYPTO0_CRYPTO_PIPEm_KEY_PAUSE_DET_ADDR
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_PIPEm_KEY_PAUSE_DET, pipe_set_num, UINT32_C(0));
    #else
      if (pipe_set_num < 8) {
        CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYm_PAUSE_DET, pipe_set_num, UINT32_C(0));
      } else {
        CE_HWIO_OUTI(CRYPTO0_CRYPTO_KEYj_PAUSE_DET, pipe_set_num, UINT32_C(0));
      }
    #endif
      break;
    case UCLIB_CIPHER_PIPE_PAUSE:
      CE_HWIO_OUT(CRYPTO0_CRYPTO_PIPE_KEY_PAUSE_SW_SRC, UINT32_C(1));
      break;
  }

  UC_GUARD(
    CE_HAL_toggle_engine_lock(me, false) == UCLIB_SUCCESS,
    UCLIB_ERR_FAILURE,
    env);

  return UCLIB_SUCCESS;
#endif
}

/**
 * Configure a BAM pipe with a key, key usage rules
 *
 * @param me         [in] Pointer to CE HAL context. Should have all configuration options desired for the pipe.
 * @param pipe_id    [in] The pipe to configure. Should be in range [0,7] since there are only 8 pipes.
 * @param sw_key     [in] Pointer to the key to configure the pipe to use.
 * @param sw_key_sz  [in] Size of the key in bytes.
 *
 * @return UCLIB_SUCCESS if successful.
 */
static int CE_HAL_set_pipe_key
(
  ce_hal_ctx_t* me,
  uint32_t pipe,
  const uint8_t* key,
  size_t key_sz
)
{
#ifdef DRM_PIPE_NOT_SUPPORTED
  return UCLIB_ERR_NOT_SUPPORTED;
#else

  if (CE_HAL_is_hwkm_supported(me))
  {
    env_t *env = me->env;

    /* Need to clear first and last use registers for pipe so that they are
     * consistent with this pipe key */
    UC_GUARD(
      CE_HAL_toggle_engine_lock(me, true) == UCLIB_SUCCESS,
      UCLIB_ERR_FAILURE,
      me->env);
    #ifdef HWIO_CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_FIRST_USE_TIMER_LO_ADDR
    CE_HWIO_OUTI(CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_FIRST_USE_TIMER_LO, pipe, UINT32_C(0));
    CE_HWIO_OUTI(CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_FIRST_USE_TIMER_HI, pipe, UINT32_C(0));
    #elif defined HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_ADDR && defined HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_ADDR
    if (pipe < 8) {
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO, pipe, UINT32_C(0));
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI, pipe, UINT32_C(0));
    } else {
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO, pipe, UINT32_C(0));
      CE_HWIO_OUTI(CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI, pipe, UINT32_C(0));
    }
    #endif

    UC_GUARD(
      CE_HAL_toggle_engine_lock(me, false) == UCLIB_SUCCESS,
      UCLIB_ERR_FAILURE,
      me->env);

    return CE_HAL_install_wrapped_keys(me, pipe);
  }
  else
  {
    return CE_HAL_set_pipe_key_legacy(me, pipe, key, key_sz);
  }
#endif
}

static int CE_HAL_hw_ctrl_helper
(
  ce_hal_ctx_t *me,
  UCLIB_CIPHER_PARAM_TYPE type,
  int val,
  uint8_t *ibuf,
  size_t isz,
  size_t *osz
)
{
  int ret;
  uint8_t pipe_pair_max = ENV_BAM_get_ce_bam_pipe_set_cnt(me->env);

  switch (type)
  {
    case UCLIB_CIPHER_SET_PIPE_KEY:
    {
      UC_GUARD(
        val >= 0 && val < pipe_pair_max,
        UCLIB_ERR_INVALID_ARG,
        me->env);
      return CE_HAL_set_pipe_key(me, val, ibuf, isz);
    }

    case UCLIB_CIPHER_SET_PIPE_TIMER:
    {
      UC_GUARD(NULL != ibuf, UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(isz >= sizeof(uint64_t), UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(
        val > 0 && val < pipe_pair_max,
        UCLIB_ERR_INVALID_ARG,
        me->env);
      UC_GUARD(
        me->hw_ver >= CE_HW_VERSION(5, 6, 0),
        UCLIB_ERR_NOT_SUPPORTED,
        me->env);

      uint64_t timer = 0;
      ENV_sec_memcpy(me->env, &timer, sizeof(timer), ibuf, isz);
      return CE_HAL_set_pipe_timer(me, val, timer);
    }

    case UCLIB_CIPHER_GET_PIPE_KEY_STATUS:
    {
      UC_GUARD(NULL != ibuf, UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(sizeof(uint32_t) <= isz, UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(
        val > 0 && val < pipe_pair_max,
        UCLIB_ERR_INVALID_ARG,
        me->env);
      UC_GUARD(
        me->hw_ver >= CE_HW_VERSION(5, 6, 0),
        UCLIB_ERR_NOT_SUPPORTED,
        me->env);

      uint32_t pipe_key_status = 0;
      ret = CE_HAL_get_pipe_status(me, val, &pipe_key_status);
      if (UCLIB_SUCCESS == ret)
      {
        ENV_sec_memcpy(me->env, ibuf, isz, &pipe_key_status, sizeof(pipe_key_status));
      }
      return ret;
    }

    case UCLIB_CIPHER_GET_PIPE_FIRST_USE:
    case UCLIB_CIPHER_GET_PIPE_LAST_USE:
    case UCLIB_CIPHER_GET_PIPE_TIME_LEFT:
    {
      UC_GUARD(NULL != ibuf, UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(sizeof(uint64_t) <= isz, UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(
        val > 0 && val < pipe_pair_max,
        UCLIB_ERR_INVALID_ARG,
        me->env);
      UC_GUARD(
        me->hw_ver >= CE_HW_VERSION(5, 6, 0),
        UCLIB_ERR_NOT_SUPPORTED,
        me->env);

      uint64_t out = 0;
      ret = CE_HAL_get_pipe_statistic(me, type, val, &out);
      if (UCLIB_SUCCESS == ret)
      {
        ENV_sec_memcpy(me->env, ibuf, isz, &out, sizeof(out));
      }
      return ret;
    }

    case UCLIB_CIPHER_SET_KEY_PAUSE_CFG:
    {
      UC_GUARD(NULL != ibuf, UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(isz >= sizeof(uint32_t), UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(
        val > 0 && val < pipe_pair_max,
        UCLIB_ERR_INVALID_ARG,
        me->env);
      UC_GUARD(
        me->hw_ver >= CE_HW_VERSION(5, 6, 0),
        UCLIB_ERR_NOT_SUPPORTED,
        me->env);

      uint32_t pause_cfg = 0;
      ENV_sec_memcpy(me->env, &pause_cfg, sizeof(pause_cfg), ibuf, isz);

      return CE_HAL_set_key_pause_cfg(me, val, pause_cfg);
    }

    case UCLIB_CIPHER_SET_PIPE_PAUSE:
    {
      UC_GUARD(NULL != ibuf, UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(isz >= sizeof(uint32_t), UCLIB_ERR_INVALID_ARG, me->env);
      UC_GUARD(
        val > 0 && val < pipe_pair_max,
        UCLIB_ERR_INVALID_ARG,
        me->env);
      UC_GUARD(
        me->hw_ver >= CE_HW_VERSION(5, 6, 0),
        UCLIB_ERR_NOT_SUPPORTED,
        me->env);

      uint32_t pause = 0;
      ENV_sec_memcpy(me->env, &pause, sizeof(pause), ibuf, isz);

      UC_GUARD(
        UCLIB_CIPHER_PIPE_UNPAUSE == pause || UCLIB_CIPHER_PIPE_PAUSE == pause,
        UCLIB_ERR_INVALID_ARG,
        me->env);

      return CE_HAL_set_pause(me, val, pause);
    }

    default:
      return UCLIB_ERR_NOT_SUPPORTED;
  }
  return UCLIB_SUCCESS;
}

/**
 * @brief Writes/reads HW parameters. These reads/writes are consolidated into
 *        a single function to allow mutex locking/unlocking and clock voting
 *        to be done in a single place, when HW is accessed outside of basic
 *        hash/cipher use-cases.
 *        For now the paramter type uses the Cipher parameter type since these
 *        functions are invoked from the ICipher interface.
 *
 *        Parameters can be passed as value (using [val]) or as an array of
 *        bytes using [ibuf, isz] arguments.
 *
 * @param h      [in] handle to the CE HAL context
 * @param type   [in] Cipher context parameter type
 * @param val    [in] Parameter can be passed by value using this argument
 * @param ibuf   [in, out] pointer to buffer in order to
 *               write/read to/from CE HAL context
 * @param isz    [in] ibuf size in bytes
 * @param osz    [out] pointer to required size on return from
 *               API
 *
 * @return UCLIB_SUCCESS if successful. UCLIB Error Code
 *         otherwise.
 */
int CE_HAL_hw_ctrl
(
  ce_hal_ctx_t *me,
  UCLIB_CIPHER_PARAM_TYPE type,
  int val,
  uint8_t *ibuf,
  size_t isz,
  size_t *osz
)
{
  int ret = UCLIB_SUCCESS;
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);

  ret = ENV_mutex_lock(me->env);
  if (UCLIB_SUCCESS == ret)
  {
    ret = ENV_ce_clk_enable(me->env);
    if (UCLIB_SUCCESS == ret)
    {
      ret = CE_HAL_hw_ctrl_helper(me, type, val, ibuf, isz, osz);

      int ret_clk = ENV_ce_clk_disable(me->env);
      if (UCLIB_SUCCESS == ret)
      {
        ret = ret_clk;
      }
    }

    int ret_unlock = ENV_mutex_release(me->env);
    if (UCLIB_SUCCESS == ret)
      ret = ret_unlock;
  }

  return ret;
}

/**
 * @brief Installs a wrapped (HW) key into the appropriate HW Key Manager slot
 *
 * @param me     [in] handle to the CE HAL context
 * @param pipe   [in] BAM pipe-set number
 *
 * @return UCLIB_SUCCESS if successful. UCLIB Error Code
 *         otherwise.
 */
int CE_HAL_install_wrapped_keys(ce_hal_ctx_t *me, uint8_t pipe)
{
  UC_PASS(!(me->auth_wrapped_key || me->encr_wrapped_key), UCLIB_SUCCESS);

  env_t* env = me->env;
  env_hwkm_key_info_t key_info = {
    .auth_key = me->auth_wrapped_key,
    .auth_key_sz = me->auth_wrapped_key_sz,
    .encr_key = me->encr_wrapped_key,
    .encr_key_sz = me->encr_wrapped_key_sz,
    .pipe = pipe,
    .odd_idx = CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_USE_ODD_KEYS),
  };

  return ENV_install_hw_key(env, &key_info);
}

/**
 * @brief Set the squeeze counter for the Extended Output Hash Algorithm, such as SHAKE and cSHAKE.
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param squeeze_cnt   [in]    squeeze counter to get the requested digest size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_squeeze_cnt(ce_hal_ctx_t *me, size_t squeeze_sz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  me->sqz_cnt = squeeze_sz;

  return UCLIB_SUCCESS;
}