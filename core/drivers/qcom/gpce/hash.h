// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE BAM hash engine definitions.
 *
 * Defines internal hash contexts, SHA engine state,
 * digest constants, and BAM hash interfaces used by
 * the GPCE hash implementation.
 */
#ifndef __UCLIB_PRV_HASH
#define __UCLIB_PRV_HASH

#include <stdbool.h>
#include "qce_env.h"
#include "ce_hal.h"

/*===========================================================================
                         SHA Definitions and Macros
 ===========================================================================*/
#define INIT_DATA_SHA1_h0   UINT32_C(0x67452301)
#define INIT_DATA_SHA1_h1   UINT32_C(0xefcdab89)
#define INIT_DATA_SHA1_h2   UINT32_C(0x98badcfe)
#define INIT_DATA_SHA1_h3   UINT32_C(0x10325476)
#define INIT_DATA_SHA1_h4   UINT32_C(0xc3d2e1f0)

#define INIT_DATA_SHA256_h0 UINT32_C(0x6a09e667)
#define INIT_DATA_SHA256_h1 UINT32_C(0xbb67ae85)
#define INIT_DATA_SHA256_h2 UINT32_C(0x3c6ef372)
#define INIT_DATA_SHA256_h3 UINT32_C(0xa54ff53a)
#define INIT_DATA_SHA256_h4 UINT32_C(0x510e527f)
#define INIT_DATA_SHA256_h5 UINT32_C(0x9b05688c)
#define INIT_DATA_SHA256_h6 UINT32_C(0x1f83d9ab)
#define INIT_DATA_SHA256_h7 UINT32_C(0x5be0cd19)

#define INIT_DATA_SHA224_h0 UINT32_C(0xc1059ed8)
#define INIT_DATA_SHA224_h1 UINT32_C(0x367cd507)
#define INIT_DATA_SHA224_h2 UINT32_C(0x3070dd17)
#define INIT_DATA_SHA224_h3 UINT32_C(0xf70e5939)
#define INIT_DATA_SHA224_h4 UINT32_C(0xffc00b31)
#define INIT_DATA_SHA224_h5 UINT32_C(0x68581511)
#define INIT_DATA_SHA224_h6 UINT32_C(0x64f98fa7)
#define INIT_DATA_SHA224_h7 UINT32_C(0xbefa4fa4)

#define INIT_DATA_SM3_h0 UINT32_C(0x7380166f)
#define INIT_DATA_SM3_h1 UINT32_C(0x4914b2b9)
#define INIT_DATA_SM3_h2 UINT32_C(0x172442d7)
#define INIT_DATA_SM3_h3 UINT32_C(0xda8a0600)
#define INIT_DATA_SM3_h4 UINT32_C(0xa96f30bc)
#define INIT_DATA_SM3_h5 UINT32_C(0x163138aa)
#define INIT_DATA_SM3_h6 UINT32_C(0xe38dee4d)
#define INIT_DATA_SM3_h7 UINT32_C(0xb0fb0e4e)

#define INIT_DATA_CE_SHA1_h0   BIG2LITTLE(INIT_DATA_SHA1_h0)
#define INIT_DATA_CE_SHA1_h1   BIG2LITTLE(INIT_DATA_SHA1_h1)
#define INIT_DATA_CE_SHA1_h2   BIG2LITTLE(INIT_DATA_SHA1_h2)
#define INIT_DATA_CE_SHA1_h3   BIG2LITTLE(INIT_DATA_SHA1_h3)
#define INIT_DATA_CE_SHA1_h4   BIG2LITTLE(INIT_DATA_SHA1_h4)

#define INIT_DATA_CE_SHA224_h0 BIG2LITTLE(INIT_DATA_SHA224_h0)
#define INIT_DATA_CE_SHA224_h1 BIG2LITTLE(INIT_DATA_SHA224_h1)
#define INIT_DATA_CE_SHA224_h2 BIG2LITTLE(INIT_DATA_SHA224_h2)
#define INIT_DATA_CE_SHA224_h3 BIG2LITTLE(INIT_DATA_SHA224_h3)
#define INIT_DATA_CE_SHA224_h4 BIG2LITTLE(INIT_DATA_SHA224_h4)
#define INIT_DATA_CE_SHA224_h5 BIG2LITTLE(INIT_DATA_SHA224_h5)
#define INIT_DATA_CE_SHA224_h6 BIG2LITTLE(INIT_DATA_SHA224_h6)
#define INIT_DATA_CE_SHA224_h7 BIG2LITTLE(INIT_DATA_SHA224_h7)

#define INIT_DATA_CE_SHA256_h0 BIG2LITTLE(INIT_DATA_SHA256_h0)
#define INIT_DATA_CE_SHA256_h1 BIG2LITTLE(INIT_DATA_SHA256_h1)
#define INIT_DATA_CE_SHA256_h2 BIG2LITTLE(INIT_DATA_SHA256_h2)
#define INIT_DATA_CE_SHA256_h3 BIG2LITTLE(INIT_DATA_SHA256_h3)
#define INIT_DATA_CE_SHA256_h4 BIG2LITTLE(INIT_DATA_SHA256_h4)
#define INIT_DATA_CE_SHA256_h5 BIG2LITTLE(INIT_DATA_SHA256_h5)
#define INIT_DATA_CE_SHA256_h6 BIG2LITTLE(INIT_DATA_SHA256_h6)
#define INIT_DATA_CE_SHA256_h7 BIG2LITTLE(INIT_DATA_SHA256_h7)

#define INIT_DATA_CE_SM3_h0 BIG2LITTLE(INIT_DATA_SM3_h0)
#define INIT_DATA_CE_SM3_h1 BIG2LITTLE(INIT_DATA_SM3_h1)
#define INIT_DATA_CE_SM3_h2 BIG2LITTLE(INIT_DATA_SM3_h2)
#define INIT_DATA_CE_SM3_h3 BIG2LITTLE(INIT_DATA_SM3_h3)
#define INIT_DATA_CE_SM3_h4 BIG2LITTLE(INIT_DATA_SM3_h4)
#define INIT_DATA_CE_SM3_h5 BIG2LITTLE(INIT_DATA_SM3_h5)
#define INIT_DATA_CE_SM3_h6 BIG2LITTLE(INIT_DATA_SM3_h6)
#define INIT_DATA_CE_SM3_h7 BIG2LITTLE(INIT_DATA_SM3_h7)

/*
 * FIPS specification refers to right rotations, while our ROTATE macro
 * is left one. This is why you might notice that rotation coefficients
 * differ from those observed in FIPS document by 32-N...
 */
# define Sigma0(x)       (ROTATE32((x),30) ^ ROTATE32((x),19) ^ ROTATE32((x),10))
# define Sigma1(x)       (ROTATE32((x),26) ^ ROTATE32((x),21) ^ ROTATE32((x),7))
# define sigma0(x)       (ROTATE32((x),25) ^ ROTATE32((x),14) ^ ((x)>>3))
# define sigma1(x)       (ROTATE32((x),15) ^ ROTATE32((x),13) ^ ((x)>>10))

# define Ch(x,y,z)       (((x) & (y)) ^ ((~(x)) & (z)))
# define Maj(x,y,z)      (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/*-
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! SHA_LONG has to be at least 32 bits wide.                    !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#define MD32_REG_T        long
#define SHA_LONG          unsigned int
#define SHA_LBLOCK        16
#define SHA_CBLOCK        (SHA_LBLOCK*4)/* SHA treats input data as a
                                        * contiguous array of 32 bit wide
                                        * big-endian values. */
#define SHA_LAST_BLOCK        (SHA_CBLOCK-8)
#define SHA_DIGEST_LENGTH     20
#define SHA256_DIGEST_LENGTH  32
#define SHA224_DIGEST_LENGTH  28
#define SM3_DIGEST_LENGTH     32

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! HW CE Hash Structure.                                        !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#define CE_HW_SHA1_BLOCK       64
#define CE_HW_SHA2_BLOCK       128

#define CE_MAX_BLOCK_SIZE      64
#define CE_HW_BLOCK_SIZE       16
#define CE_BAM_BLOCK_SIZE      CE_MAX_BLOCK_SIZE

/* SHA3 Block Size(Bitrate Size for SHA3 in bytes
   The values get from the Table 3 at the page 22 in the NIST Spec:
   https://nvlpubs.nist.gov/nistpubs/fips/nist.fips.202.pdf
*/
#define CE_HW_SHA3_224_BLOCK_SIZE      144
#define CE_HW_SHA3_256_BLOCK_SIZE      136
#define CE_HW_SHA3_384_BLOCK_SIZE      104
#define CE_HW_SHA3_512_BLOCK_SIZE      72

#define CE_HW_SHAKE128_BLOCK_SIZE      168
#define CE_HW_SHAKE256_BLOCK_SIZE      136

typedef struct hash_ctx_s hash_ctx_t;

/*===========================================================================
     Engine/Hash-Algorithm Types (needed subset, inlined from
     uclib_engine.h/uclib_hash.h since those public API headers were
     removed — see cipher.h for the same treatment of cipher types)
 ===========================================================================*/
typedef enum {
  UCLIB_ENGINE_INV = 0,
  UCLIB_CE_SW,      /** SW Crypto Engine */
  UCLIB_CE_ARMV8,   /** Crypto Engine using ARMv8 CE extention */
  UCLIB_CE_HWIO,    /** HW Crypto Engine using HWIO Access */
  UCLIB_CE_BAM,     /** HW Crypto Engine using BAM Access */
  UCLIB_CE_PKA,     /** Public Key Acceleration Engine v1 */
  UCLIB_CE_HRNG,    /** HW PRNG Engine */
  UCLIB_CE_TME_SEQ, /** Crypto Engine using the TME Sequencer */
  UCLIB_CE_TMECOM,  /** Remote Engine */
  UCLIB_CE_ARMV8_2, /** Crypto Engine using ARMv8.2 Crypto extensions */
  UCLIB_CE_PKE,     /** Public Key Encryption Engine v1 */
  UCLIB_CE_QRNG,    /** HW QRNG Engine */

  UCLIB_ENGINE_MAX_COUNT
}UCLIB_ENGINE_TYPE;

/* Hash algorithms */
typedef enum {
  UCLIB_SHA1 = 0,

  UCLIB_SHA2_224,
  UCLIB_SHA2_256,
  UCLIB_SHA2_384,
  UCLIB_SHA2_512,

  UCLIB_SHA3_224,
  UCLIB_SHA3_256,
  UCLIB_SHA3_384,
  UCLIB_SHA3_512,

  UCLIB_MD5,
  UCLIB_SM3,
  UCLIB_RIPEMD_160,

  UCLIB_SHAKE128,
  UCLIB_SHAKE256,
  UCLIB_CSHAKE128,
  UCLIB_CSHAKE256,

  UCLIB_HASH_MAX_ALG_COUNT
}UCLIB_HASH_ALG;

/* Opaque handle to a block-buffer object (uclib_util's util_blkbuf) */
typedef struct Blk_buf_s * UTIL_BLK_BUF_HANDLE;

typedef struct ce_hash_st {
  UTIL_BLK_BUF_HANDLE iblk_h;         // Handle to block buffer object
  ce_hal_ctx_t        *ce_ctx;        // CE Context
  uint8_t             *hmac_key;      // Key to be used in HMAC mode
  uint8_t             hmac_key_buf[128]; // HMAC key storage (128 = max block size, SHA384/512)
  uint8_t             *n_str;         // pointer to cSHAKE N String
  size_t              n_str_sz;       // cSHAKE N String length in bytes, max value is UCLIB_CSHAKE_MAX_N_STR_SZ
  uint8_t             *s_str;         // pointer to cSHAKE s String
  uint8_t             first_squeeze;  // first squeeze for XOF functions
  size_t              s_str_sz;       // cSHAKE S String length in bytes, max value is UCLIB_CSHAKE_MAX_S_STR_SZ
  size_t              header_sz;      // header size to be used in CSHAKE mode
  size_t              payload_sz;     // payload size to be used in in CSHAKE mode
}CE_SHA_CTX;

/*===========================================================================
                 HASH Common DEFINITIONS AND TYPE DECLARATIONS
 ===========================================================================*/
#define HASH_CTX_MAGIC                       UINT32_C(0x48415348)   /** HASH */
#define HASH_GET_DIGEST_CTX(me, type)        CAST_PTR_TYPE(me->ctx, type)
#define HASH_IS_CTX_VALID(me)                ((NULL != (me)) && (HASH_CTX_MAGIC == me->magic) && (me)->env)
#define HASH_IS_DGST_CTX_VALID(me, dc_sz)    ((NULL != (me->ctx)) && (dc_sz == me->ctx_sz))
#define HASH_IS_FLAG_SET(me, flgs)           ((me->flags & (flgs)) == (flgs))

#define HASH_NEW_CTX(hash, typeof)                         \
  hash->ctx = ENV_zalloc(hash->env, sizeof(typeof));       \
  UC_GUARD(hash->ctx, UCLIB_ERR_OUT_OF_MEMORY, hash->env); \
  hash->ctx_sz = sizeof(typeof);                           \

typedef void DIGEST_CTX;      /** Per algorithm Digest Context */

typedef int  (*hash_init_fp)    (hash_ctx_t *);
typedef int  (*hash_update_fp)  (hash_ctx_t *, const uint8_t *, size_t);
typedef int  (*hash_final_fp)   (hash_ctx_t *, uint8_t *, size_t);
typedef int  (*hash_cleanup_fp) (hash_ctx_t *);
typedef int  (*hash_reset_fp)   (hash_ctx_t *);
typedef int  (*hash_copy_fp)    (hash_ctx_t *, hash_ctx_t *);
typedef int  (*hash_oneshot_fp) (hash_ctx_t *,  const uint8_t *, size_t, uint8_t *, size_t);
typedef int  (*hash_squeeze_fp) (hash_ctx_t *, uint8_t *, size_t);
typedef int  (*hash_squeezeblocks_fp) (hash_ctx_t *, uint8_t *, size_t);

typedef struct hash_ctx_s
{
  uint32_t          magic;          /** Context magic number */

  env_t             *env;           /** Pointer to environment structure */

  UCLIB_HASH_ALG    alg;            /** Hash Algorithm */
  UCLIB_ENGINE_TYPE engine;         /** Acceleration Engine */

  size_t            md_sz;          /** Digest Size */
  size_t            iblk_sz;        /** Internal Block Size */
  uint32_t          flags;          /** Hash Flags */

  DIGEST_CTX        *ctx;           /** Hash Internal Context */
  size_t            ctx_sz;

  /** Hash APIs */
  hash_init_fp      init;
  hash_update_fp    update;
  hash_final_fp     final;
  hash_cleanup_fp   cleanup;
  hash_reset_fp     reset;
  hash_copy_fp      copy;
  hash_oneshot_fp   oneshot;
  hash_squeeze_fp   squeeze;
  hash_squeezeblocks_fp   squeezeblocks;
}hash_ctx_t;

#define HASH_SW_CMN_LONG    unsigned int
/* It is coincidental that all the hash-algorithms we currently support have an
 * internal block size of 64. If that changes, add a block-size member to the
 * common hash context structure
 */
#define HASH_SW_CMN_CBLOCK  64

int SHA_CMN_BAM_xfer_payload(void *ctx, const uint8_t *buf, size_t sz, bool first, bool last, bool blocking);

/*===========================================================================
                    SHA-384/512 Definitions and Macros
 ===========================================================================*/
#define INIT384_DATA_h0 0xcbbb9d5dc1059ed8ULL
#define INIT384_DATA_h1 0x629a292a367cd507ULL
#define INIT384_DATA_h2 0x9159015a3070dd17ULL
#define INIT384_DATA_h3 0x152fecd8f70e5939ULL
#define INIT384_DATA_h4 0x67332667ffc00b31ULL
#define INIT384_DATA_h5 0x8eb44a8768581511ULL
#define INIT384_DATA_h6 0xdb0c2e0d64f98fa7ULL
#define INIT384_DATA_h7 0x47b5481dbefa4fa4ULL

#define INIT512_DATA_h0 0x6a09e667f3bcc908ULL
#define INIT512_DATA_h1 0xbb67ae8584caa73bULL
#define INIT512_DATA_h2 0x3c6ef372fe94f82bULL
#define INIT512_DATA_h3 0xa54ff53a5f1d36f1ULL
#define INIT512_DATA_h4 0x510e527fade682d1ULL
#define INIT512_DATA_h5 0x9b05688c2b3e6c1fULL
#define INIT512_DATA_h6 0x1f83d9abfb41bd6bULL
#define INIT512_DATA_h7 0x5be0cd19137e2179ULL

#define SHA_LONG64 uint64_t

#define SHA2_384_DIGEST_LENGTH 48
#define SHA2_512_DIGEST_LENGTH 64

#define REVERSE64(w, x)                                                        \
  {                                                                            \
    uint64_t tmp = (w);                                                        \
    tmp = (tmp >> 32) | (tmp << 32);                                           \
    tmp = ((tmp & 0xff00ff00ff00ff00ULL) >> 8) |                               \
          ((tmp & 0x00ff00ff00ff00ffULL) << 8);                                \
    (x) = ((tmp & 0xffff0000ffff0000ULL) >> 16) |                              \
          ((tmp & 0x0000ffff0000ffffULL) << 16);                               \
  }

/*
 * SHA-512 treats input data as a
 * contiguous array of 64 bit
 * wide big-endian values.
 */
#define SHA512_CBLOCK (SHA_LBLOCK * 8)

typedef struct SHA512state_st SHA512_CTX;
typedef void (*sha512_make_string_fp)(SHA512_CTX *, unsigned char *md,
                                      size_t md_sz);
typedef void (*sha512_block_data_order_fp)(SHA512_CTX *, const void *p,
                                           size_t num);

typedef struct SHA512state_st {
  SHA_LONG64 h[8];
  SHA_LONG64 Nl, Nh;
  union {
    SHA_LONG64 d[SHA_LBLOCK];
    uint8_t p[SHA512_CBLOCK];
  };

  unsigned int num;
  sha512_block_data_order_fp block_data_order;
  sha512_make_string_fp make_string;
} SHA512_CTX;

int SHA2_512_CMN_SW_update(hash_ctx_t *ctx, const uint8_t *msg, size_t msg_sz);
int SHA2_512_CMN_SW_oneshot(hash_ctx_t *ctx, const uint8_t *msg, size_t msg_sz,
                            uint8_t *md, size_t md_sz);
int SHA2_512_CMN_SW_final(hash_ctx_t *ctx, uint8_t *md, size_t md_sz);
int SHA2_512_CMN_SW_cleanup(hash_ctx_t *ctx);

#endif /* __UCLIB_PRV_HASH */
