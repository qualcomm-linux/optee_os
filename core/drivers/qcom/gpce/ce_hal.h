// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * Crypto Engine HAL definitions.
 *
 * Defines the GPCE hardware abstraction layer, including
 * algorithm configuration, context management, key handling,
 * and hardware register programming interfaces.
 */
#ifndef __UCLIB_PRV_CE_HAL
#define __UCLIB_PRV_CE_HAL

#include "qce_env.h"
#include "qce_hwio.h"
#include "cipher.h"


#define CE_HW_VERSION(mj, mn, step)   ((UINT32_C(mj) << 24) + (UINT32_C(mn) << 16) + (step))

/*===========================================================================
                 Crypto Engine HAL driver Flags
 ===========================================================================*/
#define CE_HAL_MAGIC_NUMBER                 UINT32_C(0x57484543)   /* CEHW */

#define CE_HAL_SHA1_AUTH_SIZE               UINT32_C(0)
#define CE_HAL_SHA256_AUTH_SIZE             UINT32_C(1)
#define CE_HAL_SHA384_AUTH_SIZE             UINT32_C(2)
#define CE_HAL_SHA512_AUTH_SIZE             UINT32_C(3)
#define CE_HAL_SM3_AUTH_SIZE                UINT32_C(1)

#define CE_HAL_SHA3224_AUTH_SIZE            UINT32_C(0)
#define CE_HAL_SHA3256_AUTH_SIZE            UINT32_C(1)
#define CE_HAL_SHA3384_AUTH_SIZE            UINT32_C(2)
#define CE_HAL_SHA3512_AUTH_SIZE            UINT32_C(3)
#define CE_HAL_SHAKE128_AUTH_SIZE           UINT32_C(0)
#define CE_HAL_SHAKE256_AUTH_SIZE           UINT32_C(1)
#define CE_HAL_CSHAKE128_AUTH_SIZE          UINT32_C(0)
#define CE_HAL_CSHAKE256_AUTH_SIZE          UINT32_C(1)
#define CE_HAL_KMAC128_AUTH_SIZE            UINT32_C(2)
#define CE_HAL_KMAC256_AUTH_SIZE            UINT32_C(3)

#define CE_HAL_SHA3_HASH_AUTH_MODE          UINT32_C(2)
#define CE_HAL_SHA3_HMAC_AUTH_MODE          UINT32_C(3)
#define CE_HAL_SHAKE_AUTH_MODE              UINT32_C(4)
#define CE_HAL_CSHAKE_AUTH_MODE             UINT32_C(5)
#define CE_HAL_KMAC_AUTH_MODE               UINT32_C(5)

#define CE_HAL_SHAKE128_RATE_IN_BYTE        168  /* 1344bits/8 */
#define CE_HAL_SHAKE256_RATE_IN_BYTE        136  /* 1088bits/8 */

#define CE_HAL_AES128_KEY_SIZE              UINT8_C(0)
#define CE_HAL_AES256_KEY_SIZE              UINT8_C(2)
#define CE_HAL_INVALID_KEY_SIZE             UINT8_C(0xFF)
#define CE_HAL_DES_KEY_SIZE                 UINT8_C(0)
#define CE_HAL_2DES_KEY_SIZE                UINT8_C(2)
#define CE_HAL_3DES_KEY_SIZE                UINT8_C(1)
#define CE_HAL_SM4_KEY_SIZE                 UINT8_C(0)

#define CE_HAL_AUTH_ALG_SM3                 UINT32_C(7)
#define CE_HAL_ENCR_ALG_SM4                 UINT8_C(3)

#define CE_HAL_HWIO_MODE                    false
#define CE_HAL_BAM_MODE                     true

#define CE_HAL_BIG_ENDIAN                   false
#define CE_HAL_LITTLE_ENDIAN                true

#define CE_HAL_FLAGS_IS_ENCRYPTION          UINT32_C(0x00000001)
#define CE_HAL_FLAGS_IS_HIGH_SPEED          UINT32_C(0x00000002)
#define CE_HAL_FLAGS_IS_LITTLE_ENDIAN       UINT32_C(0x00000004)
#define CE_HAL_FLAGS_IS_SW_AUTH_KEY         UINT32_C(0x00000008)
#define CE_HAL_FLAGS_IS_HW_AUTH_KEY         UINT32_C(0x00000010)
#define CE_HAL_FLAGS_IS_SW_CIPHER_KEY       UINT32_C(0x00000020)
#define CE_HAL_FLAGS_IS_HW_CIPHER_KEY       UINT32_C(0x00000040)
#define CE_HAL_FLAGS_IS_CIPHER_ENG_EN       UINT32_C(0x00000080)
#define CE_HAL_FLAGS_IS_AUTH_ENG_EN         UINT32_C(0x00000100)
#define CE_HAL_FLAGS_IS_ZERO_LEN_STR        UINT32_C(0x00000200)
#define CE_HAL_FLAGS_IS_NON_BLOCKING        UINT32_C(0x00000400)
#define CE_HAL_FLAGS_IS_CMD_DSC_UNCACHED    UINT32_C(0x00000800)
#define CE_HAL_FLAGS_IS_IN_BUF_UNCACHED     UINT32_C(0x00001000)
#define CE_HAL_FLAGS_IS_OUT_BUF_UNCACHED    UINT32_C(0x00002000)
#define CE_HAL_FLAGS_IS_RSLT_BUF_UNCACHED   UINT32_C(0x00004000)
#define CE_HAL_FLAGS_IS_SHK_EN              UINT32_C(0x00008000)
#define CE_HAL_FLAGS_IS_PHK_EN              UINT32_C(0x00010000)
#define CE_HAL_FLAGS_READ_MAC               UINT32_C(0x00020000)
#define CE_HAL_FLAGS_EMPTY_DOUT_FIFO        UINT32_C(0x00040000)
#define CE_HAL_FLAGS_HASH_BEFORE_ENCR       UINT32_C(0x00080000)
#define CE_HAL_FLAGS_IS_PIPE_TIMER_EN       UINT32_C(0x00100000)
#define CE_HAL_FLAGS_USE_PIPE_KEY           UINT32_C(0x00200000)
#define CE_HAL_FLAGS_IS_CFG_OP              UINT32_C(0x00400000)
#define CE_HAL_FLAGS_USE_ODD_KEYS           UINT32_C(0x00800000)
#define CE_HAL_FLAGS_IS_HC_KEYS             UINT32_C(0x01000000)
#define CE_HAL_FLAGS_IS_CLK_SUSPENDED       UINT32_C(0x02000000)

#define CE_HAL_SET_FLAGS(me, v)             (me)->flags |= ((v) & UINT32_C(0xffffffff))
#define CE_HAL_CLR_FLAGS(me, v)             (me)->flags &= (~((v) & UINT32_C(0xffffffff)))
#define CE_HAL_IS_FLAG_SET(me, v)           (((me)->flags & ((v) & UINT32_C(0xffffffff))) == ((v) & UINT32_C(0xffffffff)))

#define CE_HAL_IS_CTX_VALID(me)             ((NULL != (me)) && (CE_HAL_MAGIC_NUMBER == (me)->magic_number) && (me)->env)

#define MAX_AUTH_BYTE_COUNT                 4
#define MAX_AUTH_IV_COUNT                   16
#define MAX_AUTH_KEY_COUNT                  32

#define MIN_AUTH_IVM_IDX                    16
#define MAX_AUTH_IVM_IDX                    49
#define MAX_AUTH_IVM_COUNT                  (MAX_AUTH_IVM_IDX - MIN_AUTH_IVM_IDX +1)

#define MAX_CIPHER_KEY_COUNT                32
#define MAX_HMAC_KEY_SIZE                   128

#define CE_BAM_MAX_XFER_SIZE                (64*1024 - 64)

// Pattern processing defaults
#define CE_MAX_PATTERN_SIZE                 16
#define CE_MAX_PARTIAL_BLK_OFFSET           16

/* 24bits GLOBAL_TIMER_HI_MIRROR + 32 bits GLOBAL_TIMER_LO_MIRROR */
#define CE_MAX_PIPE_TIMER_VALUE             (((uint64_t) 1) << 56)

#define AUTH_WRAPPED_KEY                    (true)
#define CIPHER_WRAPPED_KEY                  (false)

typedef enum CE_HW_ALG_e
{
  CE_HAL_ALG_NONE = 0,

  /* Cipher Algorithms */
  CE_HAL_DES,
  CE_HAL_2DES,
  CE_HAL_3DES,
  CE_HAL_AES,
  CE_HAL_KASUMI,
  CE_HAL_SNOW3G,
  CE_HAL_ZUC,
  CE_HAL_SM4,

  /* Hash/Auth Algorithms */
  CE_HAL_SHA1,
  CE_HAL_SHA224,
  CE_HAL_SHA256,
  CE_HAL_SHA384,
  CE_HAL_SHA512,
  CE_HAL_SM3,
  CE_HAL_SHA3_224,
  CE_HAL_SHA3_256,
  CE_HAL_SHA3_384,
  CE_HAL_SHA3_512,
  CE_HAL_SHAKE128,
  CE_HAL_SHAKE256,
  CE_HAL_CSHAKE128,
  CE_HAL_CSHAKE256,
  CE_HAL_KMAC128,
  CE_HAL_KMAC256,
  CE_HAL_ALG_MAX,
}CE_HW_ALG_t;

typedef enum CE_HW_MODE_e
{
  CE_HAL_MODE_NONE = 0,

  /* Cipher Modes */
  CE_HAL_ECB,
  CE_HAL_CBC,
  CE_HAL_CTR,
  CE_HAL_CTS,
  CE_HAL_XTS,
  CE_HAL_CCM,
  CE_HAL_GCM,
  CE_HAL_IDSA,

  /* Hash/Auth Modes */
  CE_HAL_HASH,
  CE_HAL_HMAC,
  CE_HAL_CMAC,
  CE_HAL_KMAC,

  /* Hash-Cipher Modes */
  CE_HAL_HC_CBC,
  CE_HAL_HC_CTS,
  CE_HAL_HC_CTR,

  /* Cipher Modes using VA */
  CE_HAL_CBC_VA,
  CE_HAL_CTR_VA,

  CE_HAL_MODE_MAX,
}CE_HW_MODE_t;

typedef enum
{
  BLK_BUF_HEADER = 0,
  BLK_BUF_ALIGNED,
  BLK_BUF_TAILER,
  BLK_BUF_SPARE,
}align_buf_t;

typedef enum {
  VA_OP_INVALID = 0,
  VA_OP_ENCRYPT,
  VA_OP_DECRYPT,
  VA_OP_COPY,

  MAX_VA_OP_COUNT
}VA_OP_MODE_t;

/*===========================================================================
                 Crypto Engine Result Dump Structure
 ===========================================================================*/
typedef struct ce_result_shake_s {
  uint32_t CRYPTO_AUTH_IV[50];     /* from CRYPTO_AUTH_IV0 to CRYPTO_AUTH_IV49 */
  uint32_t CRYPTO_AUTH_IV_1[16];
  uint32_t CRYPTO_AUTH_BYTECNT[4];
  uint32_t CRYPTO_ENCR_CNTR0_IV0;
  uint32_t CRYPTO_ENCR_CNTR1_IV1;
  uint32_t CRYPTO_ENCR_CNTR2_IV2;
  uint32_t CRYPTO_ENCR_CNTR3_IV3;
  uint32_t CRYPTO_STATUS;
  uint32_t CRYPTO_STATUS2;
  uint32_t RSRVD[6];
}ce_result_shake_t;

typedef struct ce_result_s {
  // 1st 64 bytes
  uint32_t CRYPTO_AUTH_IV[16];

  // 2nd 64 bytes
  uint32_t CRYPTO_AUTH_BYTECNT[4];
  uint32_t CRYPTO_ENCR_CNTR0_IV0;
  uint32_t CRYPTO_ENCR_CNTR1_IV1;
  uint32_t CRYPTO_ENCR_CNTR2_IV2;
  uint32_t CRYPTO_ENCR_CNTR3_IV3;
  uint32_t CRYPTO_STATUS;
  uint32_t CRYPTO_STATUS2;
  uint32_t RSRVD[6];
}ce_result_t;

/*===========================================================================
                 Crypto Engine HAL Context Structure
 ===========================================================================*/
typedef struct ce_hal_ctx_s ce_hal_ctx_t;
typedef int  (*hal_engine_rdy_fp) (ce_hal_ctx_t *, bool);
typedef int (*hal_engine_toggle_lock_fp) (ce_hal_ctx_t *, bool);

typedef struct ce_hal_ctx_s
{
  env_t        *env;

  uint32_t     magic_number;
  uint32_t     flags;

  uint32_t     ce_hw_ver;
  int          ce_hw_status;

  uint8_t      xts_key[MAX_CIPHER_KEY_COUNT];         /** XTS Key in byte */
  size_t       xts_du_sz;                             /** XTS Data Unit(DU) Size in byte  */

  CE_HW_ALG_t  alg;
  CE_HW_MODE_t mode;
  uint8_t      auth_sz;
  uint8_t      key_sz;

  uint32_t     *cipher_iv;          /** Pointer to IV member of Cipher context */

  CE_HW_ALG_t  hc_alg;              /** Hash algorithm in Hash Cipher Modes */
  CE_HW_MODE_t hc_mode;             /** HASH or HMAC mode in hash cipher Modes */
  uint8_t      hc_hmac_key[MAX_HMAC_KEY_SIZE];         /** HMAC key for hashcipher mode  */
  size_t       hc_key_sz;           /**  HMAC key size */

  uint32_t     auth_iv[16];
  uint32_t     auth_byte_cnt[4];

  uint32_t     nonce[4];
  uint32_t     cntr[4];
  uint32_t     mask[4];

  /** Cache line align (CLA) */
  uint8_t      *container;          /** Scratch buffer to enforce CLA */
  uint8_t      *header;             /** CLA buffer pointer representing header of obuf */
  size_t       header_size;         /** Header output buffer size */
  uint8_t      *tailer;             /** CLA buffer pointer representing tailer of obuf */
  size_t       tailer_size;         /** Tailer output buffer size */
  uint8_t      *aligned_obuf;       /** pointer to CLA output buffer */
  size_t       aligned_obuf_size;   /** output buffer offset to reach CLA */
  uint8_t      *obuf;               /** pointer to output buffer */
  size_t       obuf_size;           /** output buffer size */
  uint8_t      *spare;              /** Spare CLA buffer pointer */

  /** BAM pipe set number */
  uint8_t      pipe_set_num;
  VA_OP_MODE_t va_op;

  /** Pattern processing configuration. Added in CE 5.5.0. */
  uint8_t pattern_size;
  uint8_t process_data_size;
  uint8_t pattern_offset;

  /** Partial block configuration. Added in CE 5.5.0. */
  uint8_t partial_block_offset;

  /** Used for asynchronous hash operations */
  hal_engine_rdy_fp ready;
  bool xfer_pending;

  /** Cache HW version */
  uint32_t hw_ver;

  /** Wrapped keys. Added in CE 5.6.0 */
  uint8_t *auth_wrapped_key;
  size_t  auth_wrapped_key_sz;

  uint8_t *encr_wrapped_key;
  size_t  encr_wrapped_key_sz;

  /** For setting the pipe key enable for legacy chipsets **/
  uint32_t legacy_pipe_key_enable;

  hal_engine_toggle_lock_fp engine_toggle_lock;

  /* CE 5.8.0 */
  uint32_t auth_ivm[MAX_AUTH_IVM_COUNT];

  size_t sqz_cnt;
}ce_hal_ctx_t;

/*===========================================================================
                 DEFINITIONS AND TYPE DECLARATIONS
 ===========================================================================*/
/**
 * @brief Create a new CE HAL context
 *
 * @param env            [in]    Pointer to env
 *
 * @return pointer to CE HAL context.
 */
ce_hal_ctx_t * CE_HAL_new(env_t *env);

/**
 * @brief Free CE HAL context
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_free(ce_hal_ctx_t *me);

/**
 * @brief Resets certain fields of CE HAL context which are
 *        updated by engine.
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_reset(ce_hal_ctx_t *me);

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
                bool         little_endian);

/**
 * @brief Set Counter Mask
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param msk_idx       [in]    Mask Index
 * @param mask          [in]    Mask Value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_mask(ce_hal_ctx_t *me, uint8_t msk_idx, uint32_t mask);

/**
 * @brief Set CCM init. counter buffer
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param cntr          [in]    Pointer to CCM counter buffer
 * @param cntr_sz       [in]    buffer size in byte
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_ccm_init_cntr(ce_hal_ctx_t *me, uint8_t *cntr, size_t cntr_sz);

/**
 * @brief Set nonce
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param nonce         [in]    Pointer to nonce buffer
 * @param nonce_sz      [in]    buffer size in byte
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_nonce(ce_hal_ctx_t *me, uint8_t *nonce, size_t nonce_sz);

/**
 * @brief Set expected MAC
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param mac           [in]    Pointer to mac buffer
 * @param mac_sz        [in]    buffer size in byte
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_exp_mac(ce_hal_ctx_t *me, uint8_t *mac, size_t mac_sz);

/**
 * @brief Set Auth. Algorithm in Hash Cipher Modes
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param alg           [in]    Hash Algorithm
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_alg(ce_hal_ctx_t *me, CE_HW_ALG_t alg);

/**
 * @brief Get Auth. Algorithm in Hash Cipher Modes
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param alg           [in]    Pointer to Hash Algorithm
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_auth_alg(ce_hal_ctx_t *me, CE_HW_ALG_t *alg);

/**
 * @brief Set Auth. IV
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param idx           [in]    IV array index
 * @param iv            [in]    Initial value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_iv(ce_hal_ctx_t *me, uint8_t idx, uint32_t iv);

/**
 * @brief Set Auth. IVM
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param idx           [in]    IVM array index
 * @param iv            [in]    Initial value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_ivm(ce_hal_ctx_t *me, uint8_t idx, uint32_t iv);

/**
 * @brief Set Cipher. IV
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param iv            [in]    Pointer to IV member of Cipher
 *                      Context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_cipher_iv(ce_hal_ctx_t *me, uint32_t* iv);

/**
 * @brief Get Cipher. IV
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return Pointer to IV member.
 */
uint32_t *CE_HAL_get_cipher_iv(ce_hal_ctx_t *me);

/**
 * @brief Set Cipher XTS key
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param xts_key       [in]    Pointer to xts_key buffer
 * @param xts_key_sz    [in]    xts_key buffer size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_xts_key(ce_hal_ctx_t *me, uint8_t *xts_key, size_t xts_key_sz);

/**
 * @brief Set Cipher XTS Data Unit size
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param xts_du_sz     [in]    xts_du_sz value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_xts_du_size(ce_hal_ctx_t *me, size_t xts_du_sz);

/**
 * @brief Get Cipher XTS Data Unit size
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param xts_du_sz     [in]    pointer to xts_du_size value
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_xts_du_size(ce_hal_ctx_t *me, size_t *xts_du_sz);

/**
 * @brief Get Auth. IV
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param buf           [in]    Pointer to IV buffer
 * @param sz            [in]    IV buffer size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_auth_iv(ce_hal_ctx_t *me, uint8_t *buf, size_t sz);

/**
 * @brief Get Auth. IVM
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param buf           [in]    Pointer to IV buffer
 * @param sz            [in]    IV buffer size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_auth_ivm(ce_hal_ctx_t *me, uint8_t *buf, size_t sz);

/**
 * @brief Set Cipher Direction
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param encryption    [in]    true if encryption is enable,
 *                      false otherwise.
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_cipher_dir(ce_hal_ctx_t *me, bool encryption);

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
int CE_HAL_get_ce_state(ce_hal_ctx_t *me, bool live);

/**
 * @brief Set Auth Size
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param auth_sz       [in]    Auth size in byte
 *                      false otherwise.
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_size(ce_hal_ctx_t *me, size_t auth_sz);

/**
 * @brief Clear HW Key flags
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_clear_hw_key_flags(ce_hal_ctx_t *me);

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
int CE_HAL_set_hw_key_type(ce_hal_ctx_t *me, bool is_cipher_hw_key, bool is_auth_hw_key, bool is_SHK);

/**
 * @brief Set Crypto Operation Mode using Virtual Address
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param op            [in]    Crypto Operation Mode using VA
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_va_op(ce_hal_ctx_t *me, VA_OP_MODE_t op);

/**
 * @brief Get Crypto Operation Mode using Virtual Address
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param op            [in]    Pointer Crypto Operation Mode
 *                      using VA
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_va_op(ce_hal_ctx_t *me, VA_OP_MODE_t *op);

/**
 * @brief Set pipe set number for BAM use cases
 *
 * @param me             [in]  Pointer to CE HAL context
 * @param pipe_set_num   [in]  pipe set number
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_pipe_set_num(ce_hal_ctx_t *me, uint8_t pipe_set_num );

/**
 * @brief Get pipe set number for BAM use cases
 *
 * @param me              [in]  Pointer to CE HAL context
 * @param *pipe_set_num   [in]  pointer to pipe set number
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_get_pipe_set_num(ce_hal_ctx_t *me, uint8_t *pipe_set_num );

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
int CE_HAL_set_auth_cipher_order(ce_hal_ctx_t *me, bool is_before_cipher);

/**
 * @brief Copies the CE HAL context from one handle to another
 *
 * @param dest             [in]    Pointer to the destination CE HAL context
 * @param src              [in]    Pointer to the source CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_copy(ce_hal_ctx_t *dest, ce_hal_ctx_t *src);

/**
 * Check if HW supports pattern processing
 * @param me [in]  Pointer to the CE HAL context
 *
 * @return true if pattern processing is supported, false otherwise.
 */
bool CE_HAL_are_CE_550_features_supported(ce_hal_ctx_t *me);

/**
 * @brief Set pattern offset
 *
 * @param me               [in]    Pointer to the destination CE HAL context
 * @param offset           [in]    Offset for patterns to be processed
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_patt_proc_offset(ce_hal_ctx_t *me, uint32_t offset);

/**
 * @brief Set pattern processing data size
 *
 * @param me               [in]    Pointer to the destination CE HAL context
 * @param data_size        [in]    Data size for patterns to be processed
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_patt_proc_data_size(ce_hal_ctx_t *me, uint32_t data_size);

/**
 * @brief Set pattern size
 *
 * @param me               [in]    Pointer to the destination CE HAL context
 * @param pattern_size     [in]    Pattern size for patterns to be processed
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_patt_proc_size(ce_hal_ctx_t *me, uint32_t pattern_size);

/**
 * @brief Set partial block offset
 *
 * @param me               [in]    Pointer to the destination CE HAL context
 * @param offset           [in]    Partial block offset
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_partial_block_offset(ce_hal_ctx_t *me, uint32_t offset);

/**
 * Determine whether crpyto operations will use even or odd keys
 * This is intended to be used to provision odd keys into pipes
 *
 * @param me               [in]      Pointer to the CE HAL context
 * @param odd              [in]     Whether this context is using odd or even keys
 *
 * @return UCLIB_SUCCESS if successful
 */
int CE_HAL_set_odd_key_usage(ce_hal_ctx_t *me, bool odd);

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
int CE_HAL_ready_for_xfer(ce_hal_ctx_t *me, bool blocking);

/**
 * @brief Checks if current operation is a Virtual Address mode operation.
 *
 * @param me        [in]    Pointer to CE HAL context
 *
 * @return 'true' if VA operation is ongoing else 'false'.
 */
bool CE_HAL_is_va_op(ce_hal_ctx_t *me);

/**
 * Configure a BAM pipe with a key, key usage rules
 * This API configures the legacy pipe key registers not HWKM key slots.
 *
 * @param me         [in] Pointer to CE HAL context. Should have all configuration options desired for the pipe.
 * @param pipe_id    [in] The pipe to configure. Should be in range [0,7] since there are only 8 pipes.
 * @param key        [in] Pointer to the key to configure the pipe to use.
 * @param key_sz     [in] Size of the key in bytes.
 *
 * @return UCLIB_SUCCESS if successful.
 */
int CE_HAL_set_pipe_key_legacy
(
  ce_hal_ctx_t* me,
  uint32_t pipe,
  const uint8_t* key,
  size_t key_sz
);

/**
 * @brief Sets the mode for hc-hmac operation.
 *
 * @param me               [in]    Pointer to CE HAL context
 * @param mode             [in]    Mode for hc operation
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_auth_hc_mode(ce_hal_ctx_t *me, CE_HW_MODE_t mode);

/**
 * @brief Gets the CE HW version.
 *
 * @param me        [in]    Pointer to environment
 *
 * @return Crypto Engine HW version.
 */
uint32_t CE_HAL_get_ce_version(env_t *env);

/**
 * @brief Checks if Hardware Key Manager functionality is supported
 *
 * @param me        [in]    Pointer to CE HAL context
 *
 * @return true if HW Key Manager functionality is supported, false otherwise
 */
bool CE_HAL_is_hwkm_supported(ce_hal_ctx_t *me);

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
);

/**
 * Returns whether a wrapped key has been provided or not
 *
 * @param me          [in]  Pointer to the CE HAL context
 * @param is_auth_key [in]  Is auth or cipher key
 *
 * @return true if a wrapped key was provided, false otherwise.
 */
bool CE_HAL_wrapped_key_present(ce_hal_ctx_t *me, bool is_auth_key);

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
);

/**
 * @brief Installs a wrapped (HW) key into the appropriate HW Key Manager slot
 *
 * @param me     [in] handle to the CE HAL context
 * @param pipe   [in] BAM pipe-set number
 *
 * @return UCLIB_SUCCESS if successful. UCLIB Error Code
 *         otherwise.
 */
int CE_HAL_install_wrapped_keys(ce_hal_ctx_t *me, uint8_t pipe);

/**
 * @brief Set the squeeze counter for the Extended Output Hash Algorithm, such as SHAKE and cSHAKE.
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param squeeze_cnt   [in]    squeeze counter to get the requested digest size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_HAL_set_squeeze_cnt(ce_hal_ctx_t *me, size_t squeeze_sz);

#endif /* __UCLIB_PRV_CE_HAL */
