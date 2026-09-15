// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE BAM cipher engine definitions.
 *
 * Defines cipher algorithms, modes, context structures,
 * and internal interfaces used by the GPCE BAM cipher
 * engine implementation.
 */
#ifndef __UCLIB_PRV_CIPHER
#define __UCLIB_PRV_CIPHER

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*===========================================================================
                 DEFINITIONS AND TYPE DECLARATIONS
 ===========================================================================*/
/* Cipher algorithms — key size is baked into the AES variant itself */
typedef enum {
  UCLIB_DES = 0,
  UCLIB_2DES,
  UCLIB_3DES,
  UCLIB_AES128,
  UCLIB_AES192,
  UCLIB_AES256,

  UCLIB_CIPHER_MAX_ALG_COUNT
}UCLIB_CIPHER_ALG;

/* Cipher modes */
typedef enum {
  UCLIB_ECB = 0,
  UCLIB_CBC,
  UCLIB_CTR,
  UCLIB_CTS,
  UCLIB_XTS,

  UCLIB_CIPHER_MAX_MODE_COUNT
}UCLIB_CIPHER_MODE;

#define CIPHER_MAX_KEY_SIZE   32  /* AES-256 */
#define CIPHER_MAX_IV_SIZE    4   /* 4 x uint32_t = 16 bytes, one cipher block */

#define CIPHER_IS_FLAG_SET(me, flgs)  ((me->flags & (flgs)) == (flgs))

/* Cipher context flag bits — only the ones cipher.c/qce_cipher.c need */
#define UCLIB_CIPHER_FLAG_DIRECTION      0x00000001  /* set = encrypt, clear = decrypt */
#define UCLIB_CIPHER_XTS_KEY_READY       0x00000002
#define UCLIB_CIPHER_XTS_DU_SIZE_READY   0x00000004

/*
 * Trimmed cipher context — only the fields the BAM engine path
 * (cipher.c) actually touches. The vendor's full cipher_ctx_t also
 * carries a block-buffer handle, padding/dispatch function pointers,
 * etc. for the generic uclib_cipher_*() API, which this port bypasses
 * (same approach hash.c/qce_hash.c took for hash_ctx_t).
 */
typedef struct cipher_ctx_s
{
  env_t             *env;                       /** Pointer to environment */

  UCLIB_CIPHER_ALG  alg;                        /** Cipher Algorithm       */
  UCLIB_CIPHER_MODE mode;                       /** Cipher Mode            */

  uint8_t           key[CIPHER_MAX_KEY_SIZE];   /** Cipher key             */
  size_t            key_sz;                     /** Key size in bytes      */

  uint8_t           xts_key[CIPHER_MAX_KEY_SIZE]; /** XTS tweak key        */
  size_t            xts_key_sz;                 /** XTS tweak key size     */

  uint32_t          iv[CIPHER_MAX_IV_SIZE];     /** IV buffer              */

  struct ce_hal_ctx_s *ce_ctx;                  /** HW CE Context          */

  uint32_t          flags;                      /** Cipher Context Flags   */
}cipher_ctx_t;

/** Bits which can be set in pipe status */
typedef enum {
  UCLIB_CIPHER_PIPE_KEY_PAUSE_DET     = 0x1,  /** Set high if a pause event occured */
  UCLIB_CIPHER_PIPE_KEY_TIMER_EXPIRED = 0x2,  /** Set high if key timer is expired  */

  UCLIB_CIPHER_MAX_PIPE_KEY_STATUS    = 0xFFFFFFFFUL
}UCLIB_CIPHER_PIPE_KEY_STATUS;

/** Bits determining which pause sources a pipe can detect */
typedef enum {
  UCLIB_CIPHER_PIPE_KEY_PAUSE_SW  = 0x1,

  UCLIB_CIPHER_MAX_PIPE_KEY_PAUSE = 0xFFFFFFFFUL
}UCLIB_CIPHER_PIPE_KEY_PAUSE_CFG;

/** Whether the pipe should be paused or unpaused */
typedef enum {
  UCLIB_CIPHER_PIPE_UNPAUSE = 0, /** Unpause the pipe */
  UCLIB_CIPHER_PIPE_PAUSE = 1,   /** Trigger pause event */

  UCLIB_CIPHER_PIPE_PAUSE_INVALID = 0xFFFFFFFFUL
}UCLIB_CIPHER_PIPE_PAUSE_VAL;

/** Stream IDs */
typedef enum {
  UCLIB_CIPHER_SID_MIN = 60,

  UCLIB_CIPHER_HLOS_KERNEL_SID = 60,  /** CSID 0 */
  UCLIB_CIPHER_HLOS_USER_SID = 61,    /** CSID 1 */
  UCLIB_CIPHER_TZ_SID = 62,           /** CSID 2 */
  UCLIB_CIPHER_CPB_SID = 63,          /** CSID 3 */

  UCLIB_CIPHER_SID_MAX,
  UCLIB_CIPHER_SID_INVALID = 0xFFFFFFFFUL
}UCLIB_CIPHER_SID_VAL;

/** Operation Types */
typedef enum {
  UCLIB_CIPHER_OP_GENERAL = 0,              /** General TZ use cases  */
  UCLIB_CIPHER_OP_DRM_MULTI_OFFLOAD,        /** Multi offload DRM     */
  UCLIB_CIPHER_OP_DRM_TZ,                   /** TZ data path DRM      */
  UCLIB_CIPHER_OP_DRM_LEGACY_OFFLOAD_PIPE1, /** Legacy offload DRM    */
  UCLIB_CIPHER_OP_DRM_LEGACY_OFFLOAD_PIPE3, /** Legacy offload DRM    */
  UCLIB_CIPHER_OP_DRM_LEGACY_OFFLOAD_PIPE4, /** Legacy offload DRM    */
  UCLIB_CIPHER_OP_DRM_LEGACY_OFFLOAD_PIPE8, /** Legacy offload DRM    */
  UCLIB_CIPHER_OP_DRM_LEGACY_OFFLOAD_PIPE9, /** Legacy offload DRM    */

  UCLIB_CIPHER_OP_MAX,
  UCLIB_CIPHER_OP_INVALID = 0xFFFFFFFFUL
}UCLIB_CIPHER_OP_VAL;

/* Cipher Context Parameter Types */
typedef enum {
  UCLIB_CIPHER_SET_DIR = 0,                 /** Possible Values: UCLIB_ENCRYPT, UCLIB_DECRYPT                                           */
  UCLIB_CIPHER_GET_DIR,                     /** Possible Values: UCLIB_ENCRYPT, UCLIB_DECRYPT                                           */

  UCLIB_CIPHER_GET_ALG,                     /** Possible Values: values enumerated in UCLIB_CIPHER_ALG                                  */
  UCLIB_CIPHER_GET_MODE,                    /** Possible Values: values enumerated in UCLIB_CIPHER_MODE                                 */
  UCLIB_CIPHER_GET_ENG,                     /** Possible Values: values enumerated in UCLIB_ENGINE_TYPE                                 */

  UCLIB_CIPHER_SET_HW_KEY_TYPE,             /** Possible Values: values enumerated in UCLIB_CIPHER_HW_KEY_TYPE                          */

  UCLIB_CIPHER_SET_KEY,                     /** Data buffer and size                                                                    */
  UCLIB_CIPHER_GET_KEY,                     /** Data buffer and size                                                                    */
  UCLIB_CIPHER_SET_IV,                      /** Data buffer and size                                                                    */
  UCLIB_CIPHER_GET_IV,                      /** Data buffer and size                                                                    */
  UCLIB_CIPHER_SET_NONCE,                   /** Data buffer and size                                                                    */

  UCLIB_CIPHER_SET_XCM_PAYLOAD_LEN,         /** CCM/GCM Params: Possible Values: 0 to MAX_UINT32                                        */
  UCLIB_CIPHER_SET_XCM_TAG_LEN,             /** CCM/GCM Params: Possible Values: 0 to MAX_UINT32                                        */
  UCLIB_CIPHER_SET_XCM_HDR_LEN,             /** CCM/GCM Params: Possible Values: 0 to MAX_UINT32                                        */
  UCLIB_CIPHER_SET_XCM_TAG,                 /** CCM/GCM Params: Data buffer and size                                                    */
  UCLIB_CIPHER_GET_XCM_PAYLOAD_LEN,         /** CCM/GCM Params: Possible Values: 0 to MAX_UINT32                                        */
  UCLIB_CIPHER_GET_XCM_TAG_LEN,             /** CCM/GCM Params: Possible Values: 0 to MAX_UINT32                                        */
  UCLIB_CIPHER_GET_XCM_HDR_LEN,             /** CCM/GCM Params: Possible Values: 0 to MAX_UINT32                                        */
  UCLIB_CIPHER_GET_XCM_TAG,                 /** CCM/GCM Params: Data buffer and size                                                    */

  UCLIB_CIPHER_SET_XTS_DU_SIZE,             /** Possible Values: cipher block size to 2^20 bytes                                        */
  UCLIB_CIPHER_GET_XTS_DU_SIZE,             /** Possible Values: cipher block size to 2^20 bytes                                        */
  UCLIB_CIPHER_SET_XTS_KEY,                 /** Data buffer and size                                                                    */

  UCLIB_CIPHER_SET_COUNTER_SZ,              /** Possible Values: 32 to 128 bits                                                         */

  UCLIB_CIPHER_SET_PAD_TYPE,                /** Possible Values: all modes in UCLIB_CIPHER_PAD                                          */
  UCLIB_CIPHER_GET_PAD_TYPE,                /** Possible Values: all modes in UCLIB_CIPHER_PAD                                          */

  UCLIB_CIPHER_SET_HASH_ALG,                /** Possible Values: values enumerated in UCLIB_HASH_ALG                                    */
  UCLIB_CIPHER_SET_HASH_ORDER,              /** Possible Values: UCLIB_HASH_BEFORE_ENCR, UCLIB_HASH_AFTER_ENCR                          */
  UCLIB_CIPHER_SET_HC_HMAC_KEY,             /** HMAC key for hashcipher mode                                                            */

  UCLIB_CIPHER_SET_BAM_PIPE,                /** Set the bam pipe set for CE                                                             */
  UCLIB_CIPHER_GET_BAM_PIPE,                /** Get the bam pipe set for CE                                                             */

  UCLIB_CIPHER_SET_VA_OP_MODE,              /** Set Operation Mode using VA - UCLIB_CIPHER_VA_OP_MODE                                   */

  UCLIB_CIPHER_SET_COPY,                    /** Copies the cipher context passed as an argument into the current cipher context handle  */

  UCLIB_CIPHER_SET_PIPE_TIMER,              /** Set the timeout for the pipe which will be configured by this context                   */

  UCLIB_CIPHER_SET_PIPE_KEY,                /** Configures the selected BAM pipe with the key from the current context                  */

  UCLIB_CIPHER_SET_PATT_OFFSET,             /** Set pattern offset                                                                      */
  UCLIB_CIPHER_SET_PATT_DATA_SIZE,          /** Set pattern processing data size                                                        */
  UCLIB_CIPHER_SET_PATT_SIZE,               /** Set pattern size                                                                        */

  UCLIB_CIPHER_SET_BLOCK_OFFSET,            /** Set the partial block offset                                                            */

  UCLIB_CIPHER_USE_ODD_KEYS,                /** Set a flag to use odd keys. Pipe keys set and used by this context will be odd keys.    */
  UCLIB_CIPHER_USE_EVEN_KEYS,               /** Clear the flag to use odd keys. Pipe keys set and used by this context will be even.    */

  UCLIB_CIPHER_SET_WRAPPED_KEY,             /** Accepts data buffer and size. Used to set a wrapped key                                 */
  UCLIB_HASH_CIPHER_SET_AUTH_WRAPPED_KEY,   /** Accepts data buffer and size. Sets the auth wrapped key                                 */

  UCLIB_CIPHER_SET_BLK_CNTR,                /** Integer argument represents a counter value                                             */

  UCLIB_CIPHER_SET_SEGMENT_SIZE,            /** Set segment size for CFB mode, supported values: 128 (bits)                             */

  UCLIB_CIPHER_GET_PIPE_FIRST_USE,          /** Gets the first-use timer value (first use since chip reset) for the indicated pipe      */
  UCLIB_CIPHER_GET_PIPE_LAST_USE,           /** Gets the last-use timer value (last use since chip reset) for the indicated pipe        */
  UCLIB_CIPHER_GET_PIPE_TIME_LEFT,          /** Gets the time remaining in the pipe timer for the indicated pipe                        */

  UCLIB_CIPHER_GET_PIPE_KEY_STATUS,         /** Gets Pipe status defined as bit pattern in UCLIB_CIPHER_PIPE_KEY_STATUS                 */
  UCLIB_CIPHER_SET_KEY_PAUSE_CFG,           /** Sets Pipe Key Pause Configuration as bit pattern in UCLIB_CIPHER_PIPE_KEY_PAUSE_CFG     */
  UCLIB_CIPHER_SET_PIPE_PAUSE,              /** Set whether pipe should be paused or unpaused                                           */

  UCLIB_CIPHER_SET_VOTE_CLK,                /** Set vote CE Clock whether Suspended or Resumed                                          */
  UCLIB_CIPHER_GET_VOTE_CLK,                /** Get vote CE Clock whether Suspended or Resumed                                          */

  UCLIB_CIPHER_GET_KEYSLOT_ID,              /** Get the keyslot id assigned to cipher ctx                                               */
  UCLIB_CIPHER_GET_NUM_DRM_KEYSLOTS_AVAIL,  /** Get the total num of DRM keyslots that are currently free                               */
  UCLIB_CIPHER_CLEAR_KEY,                   /** Clear the key in keyslor for this ctx                                                   */

  UCLIB_CIPHER_SET_IN_OUT_SID,              /** Set input/output SIDs for a key (DRM key and with index based key sel enabled)          */

  UCLIB_CIPHER_SET_OP_TYPE,                 /** Set Operation type - as defined in UCLIB_CIPHER_OP_VAL                                  */

  UCLIB_CIPHER_MAX_PARAM_TYPE_COUNT
}UCLIB_CIPHER_PARAM_TYPE;

#endif /* __UCLIB_PRV_CIPHER */
