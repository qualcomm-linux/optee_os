/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 *
 * GPCE hash driver definitions.
 */

#ifndef QCOM_GPCE_HASH_H
#define QCOM_GPCE_HASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>
#include <tee_api_defines.h>
#include <utee_defines.h>
#include <util.h>

#include "ce_hal.h"
#include "util_blkbuf.h"

#define SHA1_DIGEST_SIZE	U(20)
#define CE_MAX_BLOCK_SIZE	U(64)
#define CE_HW_BLOCK_SIZE	U(16)

#define SHA1_INITIAL_H0	UINT32_C(0x67452301)
#define SHA1_INITIAL_H1	UINT32_C(0xefcdab89)
#define SHA1_INITIAL_H2	UINT32_C(0x98badcfe)
#define SHA1_INITIAL_H3	UINT32_C(0x10325476)
#define SHA1_INITIAL_H4	UINT32_C(0xc3d2e1f0)

#define INIT_DATA_CE_SHA1_h0	TEE_U32_TO_BIG_ENDIAN(SHA1_INITIAL_H0)
#define INIT_DATA_CE_SHA1_h1	TEE_U32_TO_BIG_ENDIAN(SHA1_INITIAL_H1)
#define INIT_DATA_CE_SHA1_h2	TEE_U32_TO_BIG_ENDIAN(SHA1_INITIAL_H2)
#define INIT_DATA_CE_SHA1_h3	TEE_U32_TO_BIG_ENDIAN(SHA1_INITIAL_H3)
#define INIT_DATA_CE_SHA1_h4	TEE_U32_TO_BIG_ENDIAN(SHA1_INITIAL_H4)

#define HASH_CTX_MAGIC		UINT32_C(0x48415348)

typedef struct hash_ctx_s hash_ctx_t;

struct ce_sha_ctx {
	struct Blk_buf_s *iblk_h;
	struct ce_ctx *ce_ctx;
};

struct hash_ctx_s {
	uint32_t magic;
	size_t md_sz;
	uint32_t flags;
	struct qce_dev *dev;
	void *ctx;
	size_t ctx_sz;
};

#define HASH_GET_DIGEST_CTX(hash, type) ((type *)((hash)->ctx))
static inline bool hash_is_ctx_valid(const hash_ctx_t *ctx)
{
	return ctx && ctx->magic == HASH_CTX_MAGIC;
}

static inline bool hash_dgst_ctx_valid(const hash_ctx_t *ctx,
				       size_t size)
{
	return ctx && ctx->ctx && ctx->ctx_sz == size;
}

static inline bool hash_is_flag_set(const hash_ctx_t *ctx,
				    uint32_t flag)
{
	return (ctx->flags & flag) == flag;
}

TEE_Result SHA1_CE_HWIO_init(hash_ctx_t *me);
TEE_Result SHA_CMN_CE_init(hash_ctx_t *me);
TEE_Result SHA_CMN_CE_update(hash_ctx_t *me, const uint8_t *msg,
			     size_t msg_sz);
TEE_Result SHA_CMN_CE_final(hash_ctx_t *me, uint8_t *md, size_t md_sz);
TEE_Result SHA_CMN_CE_reset(hash_ctx_t *me);
TEE_Result SHA_CMN_CE_cleanup(hash_ctx_t *me);
TEE_Result SHA_CMN_CE_copy(hash_ctx_t *dst, hash_ctx_t *src);
TEE_Result SHA_CMN_HWIO_process_din(void *ctx, const uint8_t *buf,
				    size_t buf_sz, bool first,
				    bool last);

#endif /* QCOM_GPCE_HASH_H */
