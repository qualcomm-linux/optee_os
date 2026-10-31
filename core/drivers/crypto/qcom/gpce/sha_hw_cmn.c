// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 *
 * Implementation of common GPCE hash operations.
 */

#include <stdlib.h>
#include <string.h>

#include "ce_hwio.h"
#include "hash.h"
#include <tee_api_types.h>
#include <tee_api_defines.h>

TEE_Result SHA_CMN_CE_update(hash_ctx_t *me, const uint8_t *msg,
			     size_t msg_sz)
{
	struct ce_sha_ctx *c = NULL;
	TEE_Result ret = TEE_SUCCESS;

	if (!hash_dgst_ctx_valid(me, sizeof(struct ce_sha_ctx)))
		return TEE_ERROR_BAD_STATE;
	c = HASH_GET_DIGEST_CTX(me, struct ce_sha_ctx);
	ret = util_blk_buf_out(c->iblk_h, msg, msg_sz);
	if (ret != TEE_SUCCESS)
		return ret;

	return TEE_SUCCESS;
}

TEE_Result SHA_CMN_CE_final(hash_ctx_t *me, uint8_t *md, size_t md_sz)
{
	struct ce_sha_ctx *c = NULL;
	TEE_Result ret = TEE_SUCCESS;

	if (!hash_dgst_ctx_valid(me, sizeof(struct ce_sha_ctx)))
		return TEE_ERROR_BAD_STATE;
	c = HASH_GET_DIGEST_CTX(me, struct ce_sha_ctx);
	ret = util_blk_buf_out(c->iblk_h, NULL, 0);
	if (ret != TEE_SUCCESS)
		return ret;

	if (md_sz > me->md_sz)
		md_sz = me->md_sz;
	ret = CE_HAL_get_auth_iv(c->ce_ctx, md, md_sz);
	if (ret != TEE_SUCCESS)
		return ret;

	return TEE_SUCCESS;
}

TEE_Result SHA_CMN_CE_reset(hash_ctx_t *me)
{
	if (hash_dgst_ctx_valid(me, sizeof(struct ce_sha_ctx))) {
		struct ce_sha_ctx *c =
			HASH_GET_DIGEST_CTX(me, struct ce_sha_ctx);
		TEE_Result ret = TEE_SUCCESS;

		if (c->iblk_h) {
			ret = util_blk_buf_reset(c->iblk_h);
			if (ret != TEE_SUCCESS)
				return ret;
		}
		if (c->ce_ctx) {
			ret = CE_HAL_reset(c->ce_ctx);
			if (ret != TEE_SUCCESS)
				return ret;
		}
	}

	return TEE_SUCCESS;
}

TEE_Result SHA_CMN_CE_cleanup(hash_ctx_t *me)
{
	struct ce_sha_ctx *c = NULL;
	TEE_Result ret = TEE_SUCCESS;

	/*
	 * TODO: Unvote the CE clock through ICB once OP-TEE provides an ICB
	 * driver.
	 *
	 * icb_unvote_ce_clock();
	 */
	if (!hash_dgst_ctx_valid(me, sizeof(struct ce_sha_ctx)))
		return TEE_SUCCESS;

	c = HASH_GET_DIGEST_CTX(me, struct ce_sha_ctx);
	if (c->iblk_h) {
		ret = util_blk_buf_free(c->iblk_h);
		if (ret != TEE_SUCCESS)
			return ret;
	}
	if (c->ce_ctx) {
		ret = CE_HAL_free(c->ce_ctx);
		if (ret != TEE_SUCCESS)
			return ret;
	}
	memset(me->ctx, 0, sizeof(struct ce_sha_ctx));
	free(me->ctx);
	me->ctx_sz = 0;

	return TEE_SUCCESS;
}

TEE_Result SHA_CMN_CE_init(hash_ctx_t *me)
{
	/*
	 * TODO: Vote for the CE clock through ICB once OP-TEE provides an ICB
	 * driver. The vote must complete before accessing CE registers.
	 *
	 * icb_vote_ce_clock();
	 */
	if (!hash_is_ctx_valid(me) || !me->dev)
		return TEE_ERROR_BAD_STATE;

	return TEE_SUCCESS;
}

TEE_Result SHA_CMN_CE_copy(hash_ctx_t *dest, hash_ctx_t *src)
{
	struct ce_sha_ctx *src_inner_ctx =
		HASH_GET_DIGEST_CTX(src, struct ce_sha_ctx);
	struct ce_sha_ctx *dest_inner_ctx =
		HASH_GET_DIGEST_CTX(dest, struct ce_sha_ctx);
	TEE_Result ret = TEE_SUCCESS;

	dest_inner_ctx->iblk_h = NULL;
	dest_inner_ctx->ce_ctx = NULL;

	if (src_inner_ctx->iblk_h) {
		dest_inner_ctx->iblk_h =
			calloc(1, sizeof(*src_inner_ctx->iblk_h));
		if (!dest_inner_ctx->iblk_h)
			return TEE_ERROR_OUT_OF_MEMORY;
		ret = util_blk_buf_copy(dest_inner_ctx->iblk_h,
					src_inner_ctx->iblk_h, dest);
		if (ret != TEE_SUCCESS)
			return ret;
	}
	if (src_inner_ctx->ce_ctx) {
		dest_inner_ctx->ce_ctx =
			calloc(1, sizeof(*src_inner_ctx->ce_ctx));
		if (!dest_inner_ctx->ce_ctx)
			return TEE_ERROR_OUT_OF_MEMORY;
		ret = CE_HAL_copy(dest_inner_ctx->ce_ctx,
				  src_inner_ctx->ce_ctx);
		if (ret != TEE_SUCCESS)
			return ret;
	}
	return TEE_SUCCESS;
}
