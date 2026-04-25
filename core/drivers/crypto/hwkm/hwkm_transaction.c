// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc.
 */

#include <hwkm_regs.h>
#include <hwkm.h>
#include <kernel/delay.h>
#include <string.h>
#include <string_ext.h>

/*
 * Command packet format:
 * Each command starts with a CMD[0] operation-info word that encodes the
 * opcode, slot indices, flags, and total packet length. Subsequent words
 * carry the policy, BSVE, software context, and a CRC word (always 0 while
 * CRC checking is disabled).
 */

static_assert(HWKM_OP_NIST_KEYGEN == 0x0);
static_assert(HWKM_OP_SYSTEM_KDF == 0x1);
static_assert(HWKM_OP_KEY_WRAP_EXPORT == 0x3);
static_assert(HWKM_OP_KEY_UNWRAP_IMPORT == 0x4);
static_assert(HWKM_OP_KEY_SLOT_CLEAR == 0x5);
static_assert(HWKM_OP_KEY_SLOT_RDWR == 0x6);
static_assert(HWKM_OP_SET_TPKEY == 0x7);

struct hwkm_operation_info {
	unsigned op:4;		/* [3:0] opcode. */
	unsigned irq_en:1;	/* [4] 0 = polling (always used by this driver). */
	unsigned slot1_desc:8;	/* [12:5] DKS or SKS depending on opcode. */
	unsigned slot2_desc:8;	/* [20:13] KDK, KWK, or 0. */
	unsigned op_flag:1;	/* [21] flag depending on opcode. */
	unsigned context_len:5;	/* [26:22] Software context length (KDF only). */
	unsigned len:5;		/* [31:27] total cmd words. */
} __packed;

#define HWKM_OPERATION_INFO_WORDS	1

/* Command and response formats. */

/* Shared macros: */

#define HWKM_KEY_POLICY_WORDS		2
#define HWKM_BSVE_WORDS			3
#define HWKM_KEY_BLOB_WORDS		17
#define HWKM_SOFTWARE_CONTEXT_WORDS	16
#define HWKM_KEY_RDWR_WORDS		8

static_assert(HWKM_MAX_BLOB_SIZE == HWKM_KEY_BLOB_WORDS * sizeof(uint32_t));
static_assert(HWKM_MAX_CTX_SIZE ==
	HWKM_SOFTWARE_CONTEXT_WORDS * sizeof(uint32_t));
static_assert(HWKM_MAX_KEY_SIZE == HWKM_KEY_RDWR_WORDS * sizeof(uint32_t));

/*
 * HWKM_OP_NIST_KEYGEN - Generate a fresh key into a slot via hardware PRNG.
 *
 *   CMD[0] = Operation info.
 *   CMD[1:2] = Policy.
 *   CMD[3] = CRC (disabled).
 *
 *   RSP[0] = Unused.
 *   RSP[HWKM_OP_NIST_KEYGEN_RSP_ERR_IDX] = Error status.
 */
#define HWKM_OP_NIST_KEYGEN_CMD_WORDS	4	/* CMD[0] + policy + CRC. */
#define HWKM_OP_NIST_KEYGEN_RSP_WORDS	2	/* RSP[0:1]. */
#define HWKM_OP_NIST_KEYGEN_RSP_ERR_IDX	1

/*
 * HWKM_OP_SYSTEM_KDF - Derive a child key from a KDK slot via hardware KDF.
 *
 *   CMD[0] = Operation info.
 *   CMD[1:2] = Policy.
 *   CMD[3] = BSVE[0] if bsve is enabled, 0 otherwise.
 *   CMD[4:5] = BSVE[1:2] only if bsve is enabled.
 *   CMD[n:n + m] = Software Context (SC), where n = 4 or 6 depending on BSVE,
 *                  and m is SC length.
 *   CMD[n + m + 1] = CRC (disabled).
 *
 *   RSP[0] = Unused.
 *   RSP[HWKM_OP_SYSTEM_KDF_RSP_ERR_IDX] = Error status.
 */
#define HWKM_OP_SYSTEM_KDF_CMD_WORDS	4	/* CMD[0] + policy + CRC. */
#define HWKM_OP_SYSTEM_KDF_RSP_WORDS	2	/* RSP[0:1]. */
#define HWKM_OP_SYSTEM_KDF_RSP_ERR_IDX	1

/*
 * HWKM_OP_KEY_WRAP_EXPORT - Encrypt a slot under a KWK/KSK and return the blob.
 *
 *   CMD[0] = Operation info.
 *   CMD[1:3] = BSVE.
 *   CMD[4] = CRC (disabled).
 *
 *   RSP[0] = Unused.
 *   RSP[1] = Error status.
 *   RSP[HWKM_OP_KEY_WRAP_EXPORT_RSP_WRAPPED_KEY_IDX:
 *       HWKM_OP_KEY_WRAP_EXPORT_RSP_WRAPPED_KEY_IDX + HWKM_KEY_BLOB_WORDS - 1]
 *        = Wrapped Key Blob.
 */
#define HWKM_OP_KEY_WRAP_EXPORT_CMD_WORDS	5	/* CMD[0] + BSVE + CRC. */
#define HWKM_OP_KEY_WRAP_EXPORT_RSP_WORDS	19	/* RSP[0:1] + blob. */
#define HWKM_OP_KEY_WRAP_EXPORT_RSP_ERR_IDX	1
#define HWKM_OP_KEY_WRAP_EXPORT_RSP_WRAPPED_KEY_IDX 2

/*
 * HWKM_OP_KEY_UNWRAP_IMPORT - Decrypt a wrapped blob and write the key
 *                             into a slot.
 *
 *   CMD[0] = Operation info.
 *   CMD[1:17] = Wrapped Key Blob.
 *   CMD[18] = CRC (disabled).
 *
 *   RSP[0] = Unused.
 *   RSP[HWKM_OP_KEY_UNWRAP_IMPORT_RSP_ERR_IDX]    = Error status.
 */
#define HWKM_OP_KEY_UNWRAP_IMPORT_CMD_WORDS	19	/* CMD[0] + blob + CRC. */
#define HWKM_OP_KEY_UNWRAP_IMPORT_RSP_WORDS	2	/* RSP[0:1]. */
#define HWKM_OP_KEY_UNWRAP_IMPORT_RSP_ERR_IDX	1

/*
 * HWKM_OP_KEY_SLOT_CLEAR - Zeroize a slot and invalidate its policy word.
 *
 *   CMD[0] = Operation info.
 *   CMD[1] = CRC (disabled).
 *
 *   RSP[0] = Unused.
 *   RSP[HWKM_OP_KEY_SLOT_CLEAR_RSP_ERR_IDX] = Error status
 */
#define HWKM_OP_KEY_SLOT_CLEAR_CMD_WORDS	2	/* CMD[0] + CRC. */
#define HWKM_OP_KEY_SLOT_CLEAR_RSP_WORDS	2	/* RSP[0:1]. */
#define HWKM_OP_KEY_SLOT_CLEAR_RSP_ERR_IDX	1

/*
 * HWKM_OP_KEY_SLOT_RDWR - Read or write raw key material for a SW_KEY slot.
 *
 *   CMD[0] = Operation info.
 *   CMD[1:2] = Written policy (0 if read).
 *   CMD[3:10] = Written key value (0 if read).
 *   CMD[11] = CRC (disabled).
 *
 *   RSP[0] = Unused.
 *   RSP[HWKM_OP_KEY_SLOT_RDWR_RSP_ERR_IDX] = Error status.
 *   RSP[HWKM_OP_KEY_SLOT_RDWR_RSP_POLICY_IDX:
 *       HWKM_OP_KEY_SLOT_RDWR_RSP_POLICY_IDX + HWKM_KEY_POLICY_WORDS - 1]
 *        = Read policy (0 if write).
 *   RSP[HWKM_OP_KEY_SLOT_RDWR_RSP_READ_KEY_IDX:
 *       HWKM_OP_KEY_SLOT_RDWR_RSP_READ_KEY_IDX + HWKM_KEY_RDWR_WORDS -1]
 *        = Read key value (0 if write).
 */
#define HWKM_OP_KEY_SLOT_RDWR_CMD_WORDS		12	/* CMD[0] + policy + key + CRC. */
#define HWKM_OP_KEY_SLOT_RDWR_RSP_WORDS		12	/* RSP[0:1] + policy + key. */
#define HWKM_OP_KEY_SLOT_RDWR_RSP_ERR_IDX	1
#define HWKM_OP_KEY_SLOT_RDWR_RSP_POLICY_IDX	2
#define HWKM_OP_KEY_SLOT_RDWR_RSP_READ_KEY_IDX	4

/*
 * HWKM_OP_SET_TPKEY - Install a slot as the active transport protection key.
 *
 *   CMD[0] = Operation info.
 *   CMD[1] = CRC (disabled).
 *
 *   RSP[0] = Unused.
 *   RSP[HWKM_OP_SET_TPKEY_RSP_ERR_IDX] = Error status.
 */
#define HWKM_OP_SET_TPKEY_CMD_WORDS	2	/* CMD[0] + CRC. */
#define HWKM_OP_SET_TPKEY_RSP_WORDS	2	/* RSP[0:1]. */
#define HWKM_OP_SET_TPKEY_RSP_ERR_IDX	1

/* PACK and UNPACK. */

/* Size HWKM_KEY_POLICY_WORDS words. */
struct hwkm_hw_key_policy {
	unsigned dbg_qfprom_key_rd_iv_sel:1;		/* [0] */
	unsigned reserved0:1;				/* [1] */
	unsigned wrap_with_tpkey:1;			/* [2] */
	unsigned hw_destination:4;			/* [3:6] */
	unsigned reserved1:1;				/* [7] */
	unsigned propagate_sec_level_to_child_keys:1;	/* [8] */
	unsigned security_level:2;			/* [9:10] */
	unsigned swap_export_allowed:1;			/* [11] */
	unsigned wrap_export_allowed:1;			/* [12] */
	unsigned key_type:3;				/* [13:15] */
	unsigned kdf_depth:8;				/* [16:23] */
	unsigned decrypt_allowed:1;			/* [24] */
	unsigned encrypt_allowed:1;			/* [25] */
	unsigned alg_allowed:6;				/* [26:31] */
	unsigned key_management_by_tz_secure_allowed:1;	/* [32] */
	unsigned key_management_by_nonsecure_allowed:1;	/* [33] */
	unsigned key_management_by_modem_allowed:1;	/* [34] */
	unsigned key_management_by_spu_allowed:1;	/* [35] */
	unsigned reserved2:28;				/* [36:63] */
} __packed;

/* Size HWKM_BSVE_WORDS words. */
struct hwkm_hw_kdf_bsve {
	unsigned mks:8;					/* [7:0] */
	unsigned key_policy_version_en:1;		/* [8] */
	unsigned apps_secure_en:1;			/* [9] */
	unsigned msa_secure_en:1;			/* [10] */
	unsigned lcm_fuse_row_en:1;			/* [11] */
	unsigned boot_stage_otp_en:1;			/* [12] */
	unsigned swc_en:1;				/* [13] */
	uint64_t fuse_region_sha_digest_en:64;		/* [77:14] */
	unsigned child_key_policy_en:1;			/* [78] */
	unsigned mks_en:1;				/* [79] */
	unsigned reserved:16;				/* [95:80] */
} __packed;

/* Size HWKM_BSVE_WORDS words. */
struct hwkm_hw_wrapping_bsve {
	unsigned key_policy_version_en:1;	/* [0] */
	unsigned apps_secure_en:1;		/* [1] */
	unsigned msa_secure_en:1;		/* [2] */
	unsigned lcm_fuse_row_en:1;		/* [3] */
	unsigned boot_stage_otp_en:1;		/* [4] */
	unsigned reserved0:27;			/* [31:5] */
	unsigned reserved1:32;			/* [63:32] */
	unsigned reserved2:32;			/* [95:64] */
} __packed;

/**
 * hwkm_pack_key_policy() - Pack a software key policy into hardware format.
 * @dst: Destination hardware policy structure.
 * @src: Source software policy.
 *
 * Fill @dst from @src using the HWKM hardware policy encoding.
 * Fields not represented in struct hwkm_key_policy are cleared.
 */
static void hwkm_pack_key_policy(struct hwkm_hw_key_policy *dst,
				 const struct hwkm_key_policy *src)
{
	memset(dst, 0, sizeof(*dst));

	dst->wrap_with_tpkey = src->wrap_with_tpkey_allowed ? 1:0;
	dst->hw_destination = (unsigned)src->hw_destination;
	dst->security_level = (unsigned)src->security_lvl;
	dst->swap_export_allowed = src->swap_export_allowed ? 1:0;
	dst->wrap_export_allowed = src->wrap_export_allowed ? 1:0;
	dst->key_type = (unsigned)src->key_type;
	dst->kdf_depth = src->kdf_depth;
	dst->decrypt_allowed = src->dec_allowed ? 1:0;
	dst->encrypt_allowed = src->enc_allowed ? 1:0;
	dst->alg_allowed = (unsigned)src->alg_allowed;
	dst->key_management_by_tz_secure_allowed = src->km_by_tz_allowed ? 1:0;
	dst->key_management_by_nonsecure_allowed = src->km_by_nsec_allowed ? 1:0;
	dst->key_management_by_modem_allowed = src->km_by_modem_allowed ? 1:0;
	dst->key_management_by_spu_allowed = src->km_by_spu_allowed ? 1:0;
}

/**
 * hwkm_unpack_key_policy() - Unpack a hardware key policy into software form.
 * @dst: Destination software policy.
 * @src: Source hardware policy structure.
 *
 * Decode @src from the HWKM hardware policy format into @dst.
 * Fields not represented in struct hwkm_key_policy are ignored.
 */
static void hwkm_unpack_key_policy(struct hwkm_key_policy *dst,
				   const struct hwkm_hw_key_policy *src)
{
	memset(dst, 0, sizeof(*dst));

	dst->wrap_with_tpkey_allowed = !!src->wrap_with_tpkey;
	dst->hw_destination = (enum hwkm_key_destination)src->hw_destination;
	dst->security_lvl = (enum hwkm_key_security_lvl)src->security_level;
	dst->swap_export_allowed = !!src->swap_export_allowed;
	dst->wrap_export_allowed = !!src->wrap_export_allowed;
	dst->key_type = (enum hwkm_key_type)src->key_type;
	dst->kdf_depth = (uint8_t)src->kdf_depth;
	dst->dec_allowed = !!src->decrypt_allowed;
	dst->enc_allowed = !!src->encrypt_allowed;
	dst->alg_allowed = (enum hwkm_algo)src->alg_allowed;
	dst->km_by_tz_allowed = !!src->key_management_by_tz_secure_allowed;
	dst->km_by_nsec_allowed = !!src->key_management_by_nonsecure_allowed;
	dst->km_by_modem_allowed = !!src->key_management_by_modem_allowed;
	dst->km_by_spu_allowed = !!src->key_management_by_spu_allowed;
}

/**
 * hwkm_pack_wrapping_bsve() - Pack wrapping BSVE settings into hardware format.
 * @dst: Destination hardware BSVE structure.
 * @src: Source BSVE settings.
 *
 * Fill @dst with the wrapping-BSVE encoding derived from @src. If BSVE is
 * disabled, @dst is cleared and no fields are set.
 */
static void hwkm_pack_wrapping_bsve(struct hwkm_hw_wrapping_bsve *dst,
				    const struct hwkm_bsve *src)
{
	memset(dst, 0, sizeof(*dst));

	if (!src->enabled)
		return;

	dst->key_policy_version_en = src->km_key_policy_ver_en ? 1:0;
	dst->apps_secure_en = src->km_apps_secure_en ? 1:0;
	dst->msa_secure_en = src->km_msa_secure_en ? 1:0;
	dst->lcm_fuse_row_en = src->km_lcm_fuse_en ? 1:0;
	dst->boot_stage_otp_en = src->km_boot_stage_otp_en ? 1:0;
}

/**
 * hwkm_pack_kdf_bsve() - Pack KDF BSVE settings into hardware format.
 * @dst: Destination hardware BSVE structure.
 * @src: Source BSVE settings.
 * @mks: Mixing key selector value.
 *
 * Fill @dst with the KDF-BSVE encoding derived from @src and @mks. If BSVE is
 * disabled, @dst is cleared and no fields are set.
 */
static void hwkm_pack_kdf_bsve(struct hwkm_hw_kdf_bsve *dst,
			       const struct hwkm_bsve *src,
			       uint8_t mks)
{
	memset(dst, 0, sizeof(*dst));

	if (!src->enabled)
		return;

	dst->mks = mks;
	dst->key_policy_version_en = src->km_key_policy_ver_en ? 1:0;
	dst->apps_secure_en = src->km_apps_secure_en ? 1:0;
	dst->msa_secure_en = src->km_msa_secure_en ? 1:0;
	dst->lcm_fuse_row_en = src->km_lcm_fuse_en ? 1:0;
	dst->boot_stage_otp_en = src->km_boot_stage_otp_en ? 1:0;
	dst->swc_en = src->km_swc_en ? 1:0;
	dst->fuse_region_sha_digest_en = src->km_fuse_region_sha_digest_en;
	dst->child_key_policy_en = src->km_child_key_policy_en ? 1:0;
	dst->mks_en = src->km_mks_en ? 1:0;
}

/* RUN TRANSACTION. */

#define HWKM_MASTER_MAX_RETRIES	100000U

/**
 * hwkm_fifo_wait() - Poll a FIFO status field until it is non-zero.
 * @base: HWKM MMIO base.
 * @reg: Status register offset.
 * @mask: Field mask.
 * @shift: Field shift.
 *
 * Return: HWKM_SUCCESS when the field becomes non-zero, or
 *         HWKM_ERR_FIFO_TIMEOUT if the field does not become non-zero within
 *         HWKM_MASTER_MAX_RETRIES attempts.
 */
static int hwkm_fifo_wait(vaddr_t base, uint32_t reg,
			  uint32_t mask, uint32_t shift)
{
	uint32_t retries = 0;

	while (!hwkm_reg_get_field(base, reg, mask, shift)) {
		if (++retries > HWKM_MASTER_MAX_RETRIES)
			return HWKM_ERR_FIFO_TIMEOUT;
		udelay(10);
	}

	return HWKM_SUCCESS;
}

/**
 * master_run_transaction() - Submit one command packet to the master HWKM.
 * @cmd: Command packet words to write into the command FIFO.
 * @cmd_words: Number of 32-bit words in @cmd.
 * @rsp: Response buffer filled from the response FIFO.
 * @rsp_words: Number of 32-bit words expected in @rsp.
 *
 * Step by step:
 *   1. Fetch the driver context and resolve the HWKM MMIO base.
 *   2. Pulse CMD_FIFO_CLEAR to flush any stale command FIFO contents.
 *   3. Clear stale ESR bits from the previous transaction.
 *   4. Enable command processing in BANK0_KM_CTL.
 *   5. Verify that CMD_FIFO_CLEAR has deasserted, otherwise the FIFO did not
 *      drain correctly.
 *   6. For each command word, poll CMD_FIFO_AVAIL_SPACE until space is
 *      available or a timeout is reached, then write the word to
 *      HWKM_BANK0_KM_CMD_FIFO.
 *   7. For each response word, poll RSP_FIFO_AVAIL_DATA until data is
 *      available or a timeout is reached, then read the word from
 *      HWKM_BANK0_KM_RSP_FIFO.
 *   8. Verify that CMD_DONE is set, which indicates the command completed and
 *      the full response was produced.
 *   9. Clear CMD_DONE before returning.
 *
 * Return: HWKM_SUCCESS on success, or a HWKM_ERR_* code on failure.
 */
static int master_run_transaction(const uint32_t *cmd, size_t cmd_words,
				  uint32_t *rsp, size_t rsp_words)
{
	struct hwkm_drv_ctx *ctx;
	vaddr_t base;

	if (!cmd || !cmd_words || !rsp || !rsp_words)
		return HWKM_ERR_INVALID_ARG;

	ctx = hwkm_get_context();
	if (!ctx)
		return HWKM_ERR_INVALID_ARG;

	base = ctx->base;

	/* Flush any stale command FIFO contents. */
	hwkm_reg_set_field(base, HWKM_BANK0_KM_CTL,
			   HWKM_BANK0_KM_CTL_CMD_FIFO_CLEAR,
			   HWKM_BANK0_KM_CTL_CMD_FIFO_CLEAR_SHIFT, 1);
	hwkm_reg_set_field(base, HWKM_BANK0_KM_CTL,
			   HWKM_BANK0_KM_CTL_CMD_FIFO_CLEAR,
			   HWKM_BANK0_KM_CTL_CMD_FIFO_CLEAR_SHIFT, 0);

	/* Clear stale error state from the previous transaction. */
	HWKM_REG_WRITE(base, HWKM_BANK0_KM_ESR,
		       HWKM_REG_READ(base, HWKM_BANK0_KM_ESR));

	/* Enable command processing. */
	hwkm_reg_set_field(base, HWKM_BANK0_KM_CTL,
			   HWKM_BANK0_KM_CTL_CMD_ENABLE,
			   HWKM_BANK0_KM_CTL_CMD_ENABLE_SHIFT, 1);

	/* Confirm the FIFO clear bit has deasserted. */
	if (hwkm_reg_get_field(base, HWKM_BANK0_KM_CTL,
			       HWKM_BANK0_KM_CTL_CMD_FIFO_CLEAR,
			       HWKM_BANK0_KM_CTL_CMD_FIFO_CLEAR_SHIFT))
		return HWKM_ERR_FIFO_NOT_EMPTY;

	/* Push the command packet one word at a time. */
	for (size_t i = 0; i < cmd_words; i++) {
		int rc = hwkm_fifo_wait(base, HWKM_BANK0_KM_STATUS,
					HWKM_BANK0_KM_STATUS_CMD_FIFO_AVAIL_SPACE,
					HWKM_BANK0_KM_STATUS_CMD_FIFO_AVAIL_SPACE_SHIFT);
		if (rc)
			return rc;

		HWKM_REG_WRITE(base, HWKM_BANK0_KM_CMD_FIFO, cmd[i]);
	}

	/* Pull the response packet one word at a time. */
	for (size_t i = 0; i < rsp_words; i++) {
		int rc = hwkm_fifo_wait(base, HWKM_BANK0_KM_STATUS,
					HWKM_BANK0_KM_STATUS_RSP_FIFO_AVAIL_DATA,
					HWKM_BANK0_KM_STATUS_RSP_FIFO_AVAIL_DATA_SHIFT);
		if (rc)
			return rc;

		rsp[i] = HWKM_REG_READ(base, HWKM_BANK0_KM_RSP_FIFO);
	}

	/* The hardware must report completion after the response is read. */
	if (!hwkm_reg_get_field(base, HWKM_BANK0_KM_IRQ_STATUS,
				HWKM_BANK0_KM_IRQ_STATUS_CMD_DONE,
				HWKM_BANK0_KM_IRQ_STATUS_CMD_DONE_SHIFT))
		return HWKM_ERR_RSP_OVERFLOW;

	/* Acknowledge completion. */
	HWKM_REG_WRITE(base, HWKM_BANK0_KM_IRQ_STATUS,
		       HWKM_BANK0_KM_IRQ_STATUS_CMD_DONE);

	return HWKM_SUCCESS;
}

static int run_transaction(const struct hwkm_transaction *t,
			   const uint32_t *cmd, size_t cmd_words,
			   uint32_t *rsp, size_t rsp_words)
{
	switch (t->hdl->dest) {
	case HWKM_KEY_DEST_KM_MASTER:
		return master_run_transaction(cmd, cmd_words, rsp, rsp_words);
	default:
		return HWKM_ERR_INVALID_DEST;
	}
}

/* Command handlers. */

/**
 * hwkm_nist_keygen_handle() - Execute HWKM_OP_NIST_KEYGEN.
 * @t: Transaction carrying the keygen request and response.
 *
 * Command format:
 *   CMD[0] = Operation info.
 *   CMD[1:2] = Key policy.
 *   CMD[3] = CRC word, left as 0 when CRC checking is disabled.
 *
 * Response format:
 *   RSP[0] = Unused.
 *   RSP[1] = Error status.
 *
 * Return: HWKM_SUCCESS on transport success, or a HWKM_ERR_* code.
 * Hardware command failure is reported in t->rsp.status.
 */
static int hwkm_nist_keygen_handle(struct hwkm_transaction *t)
{
	uint32_t cmd[HWKM_OP_NIST_KEYGEN_CMD_WORDS] = { 0 };
	uint32_t rsp[HWKM_OP_NIST_KEYGEN_RSP_WORDS] = { 0 };
	struct hwkm_hw_key_policy policy = { };
	struct hwkm_operation_info op = { };
	int rc;

	if (!t || !t->hdl)
		return HWKM_ERR_INVALID_ARG;

	op = (struct hwkm_operation_info){
		.op = HWKM_OP_NIST_KEYGEN,
		.irq_en = 0,
		.slot1_desc = t->cmd.keygen.dks,
		.slot2_desc = 0,
		.op_flag = 0,
		.context_len = 0,
		.len = ARRAY_SIZE(cmd),
	};

	hwkm_pack_key_policy(&policy, &t->cmd.keygen.policy);
	/* CMD[0]: */
	memcpy(&cmd[0], &op, sizeof(op));
	/* CMD[1:2]: */
	memcpy(&cmd[HWKM_OPERATION_INFO_WORDS], &policy, sizeof(policy));

	rc = run_transaction(t, cmd, ARRAY_SIZE(cmd), rsp, ARRAY_SIZE(rsp));
	/* On success, RSP[1]: */
	if (rc == HWKM_SUCCESS)
		t->rsp.status = rsp[HWKM_OP_NIST_KEYGEN_RSP_ERR_IDX];

	memset(cmd, 0, sizeof(cmd));
	memset(rsp, 0, sizeof(rsp));

	return rc;
}

/**
 * hwkm_system_kdf_handle() - Execute HWKM_OP_SYSTEM_KDF.
 * @t: Transaction carrying the KDF request and response.
 *
 * Command format:
 *   CMD[0] = Operation info.
 *   CMD[1:2] = Child key policy.
 *   CMD[3] = BSVE[0] if bsve is enabled, 0 otherwise.
 *   CMD[4:5] = BSVE[1:2] only if bsve is enabled.
 *   CMD[n:] = Software context, padded to 32-bit words.
 *   CMD[last] = CRC word, left as 0 when CRC checking is disabled.
 *
 * Response format:
 *   RSP[0] = Unused.
 *   RSP[1] = Error status.
 *
 * Return: HWKM_SUCCESS on transport success, or a HWKM_ERR_* code.
 * Hardware command failure is reported in t->rsp.status.
 */
static int hwkm_system_kdf_handle(struct hwkm_transaction *t)
{
	uint32_t cmd[HWKM_OP_SYSTEM_KDF_CMD_WORDS +
		     HWKM_BSVE_WORDS + HWKM_SOFTWARE_CONTEXT_WORDS] = { 0 };
	uint32_t rsp[HWKM_OP_SYSTEM_KDF_RSP_WORDS] = { 0 };
	struct hwkm_hw_key_policy policy = { };
	struct hwkm_operation_info op = { };
	size_t ctx_words, ctx_idx, cmd_words;
	int rc;

	if (!t || !t->hdl)
		return HWKM_ERR_INVALID_ARG;

	if (t->cmd.kdf.ctx_len > sizeof(t->cmd.kdf.ctx))
		return HWKM_ERR_INVALID_ARG;

	ctx_words = ROUNDUP_DIV(t->cmd.kdf.ctx_len, sizeof(uint32_t));
	ctx_idx = HWKM_OPERATION_INFO_WORDS + HWKM_KEY_POLICY_WORDS +
		  (t->cmd.kdf.bsve.enabled ? HWKM_BSVE_WORDS : 1);

	/* Total command length. */
	cmd_words = ctx_idx + ctx_words + 1 /* CRC */;
	if (cmd_words > ARRAY_SIZE(cmd))
		return HWKM_ERR_INVALID_ARG;

	op = (struct hwkm_operation_info){
		.op = HWKM_OP_SYSTEM_KDF,
		.irq_en = 0,
		.slot1_desc = t->cmd.kdf.dks,
		.slot2_desc = t->cmd.kdf.kdk,
		.op_flag = t->cmd.kdf.bsve.enabled ? 1U : 0U,
		.context_len = ctx_words,
		.len = ctx_idx + 1, /* Up to and including CRC, excluding context. */
	};

	hwkm_pack_key_policy(&policy, &t->cmd.kdf.policy);
	/* CMD[0]: */
	memcpy(&cmd[0], &op, sizeof(op));
	/* CMD[1:2]: */
	memcpy(&cmd[HWKM_OPERATION_INFO_WORDS], &policy, sizeof(policy));

	if (t->cmd.kdf.bsve.enabled) {
		struct hwkm_hw_kdf_bsve bsve = { };

		hwkm_pack_kdf_bsve(&bsve, &t->cmd.kdf.bsve, t->cmd.kdf.mks);
		/* CMD[3:5]: */
		memcpy(&cmd[HWKM_OPERATION_INFO_WORDS + HWKM_KEY_POLICY_WORDS],
		       &bsve, sizeof(bsve));
	}

	if (t->cmd.kdf.ctx_len) {
		/* CMD[n:], if software context exists: */
		memcpy(&cmd[ctx_idx], t->cmd.kdf.ctx, t->cmd.kdf.ctx_len);
	}

	rc = run_transaction(t, cmd, cmd_words, rsp, ARRAY_SIZE(rsp));
	/* On success, RSP[1]: */
	if (rc == HWKM_SUCCESS)
		t->rsp.status = rsp[HWKM_OP_SYSTEM_KDF_RSP_ERR_IDX];

	memset(cmd, 0, sizeof(cmd));
	memset(rsp, 0, sizeof(rsp));

	return rc;
}

/**
 * hwkm_key_wrap_export_handle() - Execute HWKM_OP_KEY_WRAP_EXPORT.
 * @t: Transaction carrying the wrap-export request and response.
 *
 * Command format:
 *   CMD[0] = Operation info.
 *   CMD[1:3] = BSVE, or 0 if BSVE is disabled.
 *   CMD[4] = CRC word, left as 0 when CRC checking is disabled.
 *
 * Response format:
 *   RSP[0] = Unused.
 *   RSP[1] = Error status.
 *   RSP[2:18] = Wrapped key blob on success.
 *
 * Return: HWKM_SUCCESS on transport success, or a HWKM_ERR_* code.
 * Hardware command failure is reported in t->rsp.status.
 */
static int hwkm_key_wrap_export_handle(struct hwkm_transaction *t)
{
	uint32_t cmd[HWKM_OP_KEY_WRAP_EXPORT_CMD_WORDS] = { 0 };
	uint32_t rsp[HWKM_OP_KEY_WRAP_EXPORT_RSP_WORDS] = { 0 };
	struct hwkm_hw_wrapping_bsve bsve = { };
	struct hwkm_operation_info op = { };
	int rc;

	if (!t || !t->hdl)
		return HWKM_ERR_INVALID_ARG;

	op = (struct hwkm_operation_info){
		.op = HWKM_OP_KEY_WRAP_EXPORT,
		.irq_en = 0,
		.slot1_desc = t->cmd.wrap.sks,
		.slot2_desc = t->cmd.wrap.kwk,
		.op_flag = 0,
		.context_len = 0,
		.len = ARRAY_SIZE(cmd),
	};

	hwkm_pack_wrapping_bsve(&bsve, &t->cmd.wrap.bsve);
	/* CMD[0]: */
	memcpy(&cmd[0], &op, sizeof(op));
	/* CMD[1:3]: */
	memcpy(&cmd[HWKM_OPERATION_INFO_WORDS], &bsve, sizeof(bsve));

	rc = run_transaction(t, cmd, ARRAY_SIZE(cmd), rsp, ARRAY_SIZE(rsp));
	/* On success, RSP[1]: */
	if (rc == HWKM_SUCCESS) {
		t->rsp.status = rsp[HWKM_OP_KEY_WRAP_EXPORT_RSP_ERR_IDX];
		/* On operation success, RSP[2:18]: */
		if (!t->rsp.status) {
			memcpy(t->rsp.wrap.wkb,
			       &rsp[HWKM_OP_KEY_WRAP_EXPORT_RSP_WRAPPED_KEY_IDX],
			       HWKM_MAX_BLOB_SIZE);
		}
	}

	memset(cmd, 0, sizeof(cmd));
	memset(rsp, 0, sizeof(rsp));

	return rc;
}

/**
 * hwkm_key_unwrap_import_handle() - Execute HWKM_OP_KEY_UNWRAP_IMPORT.
 * @t: Transaction carrying the unwrap-import request and response.
 *
 * Command format:
 *   CMD[0] = Operation info.
 *   CMD[1:17] = Wrapped key blob.
 *   CMD[18] = CRC word, left as 0 when CRC checking is disabled.
 *
 * Response format:
 *   RSP[0] = Unused.
 *   RSP[1] = Error status.
 *
 * Return: HWKM_SUCCESS on transport success, or a HWKM_ERR_* code.
 * Hardware command failure is reported in t->rsp.status.
 */
static int hwkm_key_unwrap_import_handle(struct hwkm_transaction *t)
{
	uint32_t cmd[HWKM_OP_KEY_UNWRAP_IMPORT_CMD_WORDS] = { 0 };
	uint32_t rsp[HWKM_OP_KEY_UNWRAP_IMPORT_RSP_WORDS] = { 0 };
	struct hwkm_operation_info op = { };
	int rc;

	if (!t || !t->hdl)
		return HWKM_ERR_INVALID_ARG;

	op = (struct hwkm_operation_info){
		.op = HWKM_OP_KEY_UNWRAP_IMPORT,
		.irq_en = 0,
		.slot1_desc = t->cmd.unwrap.dks,
		.slot2_desc = t->cmd.unwrap.kwk,
		.op_flag = 0,
		.context_len = 0,
		.len = ARRAY_SIZE(cmd),
	};

	/* CMD[0]: */
	memcpy(&cmd[0], &op, sizeof(op));
	/* CMD[1:17]: */
	memcpy(&cmd[HWKM_OPERATION_INFO_WORDS], t->cmd.unwrap.wkb,
	       HWKM_MAX_BLOB_SIZE);

	rc = run_transaction(t, cmd, ARRAY_SIZE(cmd), rsp, ARRAY_SIZE(rsp));
	/* On success, RSP[1]: */
	if (rc == HWKM_SUCCESS)
		t->rsp.status = rsp[HWKM_OP_KEY_UNWRAP_IMPORT_RSP_ERR_IDX];

	memset(cmd, 0, sizeof(cmd));
	memset(rsp, 0, sizeof(rsp));

	return rc;
}

/**
 * hwkm_key_slot_clear_handle() - Execute HWKM_OP_KEY_SLOT_CLEAR.
 * @t: Transaction carrying the clear request and response.
 *
 * Command format:
 *   CMD[0] = Operation info.
 *   CMD[1] = CRC word, left as 0 when CRC checking is disabled.
 *
 * Response format:
 *   RSP[0] = Unused.
 *   RSP[1] = Error status.
 *
 * Return: HWKM_SUCCESS on transport success, or a HWKM_ERR_* code.
 * Hardware command failure is reported in t->rsp.status.
 */
static int hwkm_key_slot_clear_handle(struct hwkm_transaction *t)
{
	uint32_t cmd[HWKM_OP_KEY_SLOT_CLEAR_CMD_WORDS] = { 0 };
	uint32_t rsp[HWKM_OP_KEY_SLOT_CLEAR_RSP_WORDS] = { 0 };
	struct hwkm_operation_info op = { };
	int rc;

	if (!t || !t->hdl)
		return HWKM_ERR_INVALID_ARG;

	op = (struct hwkm_operation_info){
		.op = HWKM_OP_KEY_SLOT_CLEAR,
		.irq_en = 0,
		.slot1_desc = t->cmd.clear.dks,
		.slot2_desc = 0,
		.op_flag = t->cmd.clear.is_double_key ? 1U : 0U,
		.context_len = 0,
		.len = ARRAY_SIZE(cmd),
	};

	/* CMD[0]: */
	memcpy(&cmd[0], &op, sizeof(op));

	rc = run_transaction(t, cmd, ARRAY_SIZE(cmd), rsp, ARRAY_SIZE(rsp));
	/* On success, RSP[1]: */
	if (rc == HWKM_SUCCESS)
		t->rsp.status = rsp[HWKM_OP_KEY_SLOT_CLEAR_RSP_ERR_IDX];

	memset(cmd, 0, sizeof(cmd));
	memset(rsp, 0, sizeof(rsp));

	return rc;
}

/**
 * hwkm_key_slot_rdwr_handle() - Execute HWKM_OP_KEY_SLOT_RDWR.
 * @t: Transaction carrying the read/write request and response.
 *
 * Command format:
 *   CMD[0] = Operation info.
 *   CMD[1:2] = Written policy, or 0 for read.
 *   CMD[3:10] = Written key value, or 0 for read.
 *   CMD[11] = CRC word, left as 0 when CRC checking is disabled.
 *
 * Response format:
 *   RSP[0] = Unused.
 *   RSP[1] = Error status.
 *   RSP[2:3] = Read policy, or 0 for write.
 *   RSP[4:11] = Read key value, or 0 for write.
 *
 * Return: HWKM_SUCCESS on transport success, or a HWKM_ERR_* code.
 * Hardware command failure is reported in t->rsp.status.
 */
static int hwkm_key_slot_rdwr_handle(struct hwkm_transaction *t)
{
	uint32_t cmd[HWKM_OP_KEY_SLOT_RDWR_CMD_WORDS] = { 0 };
	uint32_t rsp[HWKM_OP_KEY_SLOT_RDWR_RSP_WORDS] = { 0 };
	struct hwkm_operation_info op = { };
	int rc;

	if (!t || !t->hdl)
		return HWKM_ERR_INVALID_ARG;

	op = (struct hwkm_operation_info){
		.op = HWKM_OP_KEY_SLOT_RDWR,
		.irq_en = 0,
		.slot1_desc = t->cmd.rdwr.slot,
		.slot2_desc = 0,
		.op_flag = t->cmd.rdwr.is_write ? 1U : 0U,
		.context_len = 0,
		.len = ARRAY_SIZE(cmd),
	};

	/* CMD[0]: */
	memcpy(&cmd[0], &op, sizeof(op));

	if (t->cmd.rdwr.is_write) {
		struct hwkm_hw_key_policy policy = { };

		hwkm_pack_key_policy(&policy, &t->cmd.rdwr.policy);
		/* CMD[1:2]: */
		memcpy(&cmd[HWKM_OPERATION_INFO_WORDS], &policy,
		       sizeof(policy));
		/* CMD[3:10]: */
		memcpy(&cmd[HWKM_OPERATION_INFO_WORDS + HWKM_KEY_POLICY_WORDS],
		       t->cmd.rdwr.key, HWKM_MAX_KEY_SIZE);

	}

	rc = run_transaction(t, cmd, ARRAY_SIZE(cmd), rsp, ARRAY_SIZE(rsp));
	/* On success, RSP[1]: */
	if (rc == HWKM_SUCCESS) {
		t->rsp.status = rsp[HWKM_OP_KEY_SLOT_RDWR_RSP_ERR_IDX];
		if (!t->cmd.rdwr.is_write && !t->rsp.status) {
			struct hwkm_hw_key_policy policy = { };

			/* RSP[2:3]: */
			memcpy(&policy,
			       &rsp[HWKM_OP_KEY_SLOT_RDWR_RSP_POLICY_IDX],
			       sizeof(policy));
			hwkm_unpack_key_policy(&t->rsp.rdwr.policy, &policy);
			/* RSP[4:11]: */
			memcpy(t->rsp.rdwr.key,
			       &rsp[HWKM_OP_KEY_SLOT_RDWR_RSP_READ_KEY_IDX],
			       HWKM_MAX_KEY_SIZE);
		}
	}

	memset(cmd, 0, sizeof(cmd));
	memset(rsp, 0, sizeof(rsp));

	return rc;
}

/**
 * hwkm_set_tpkey_handle() - Execute HWKM_OP_SET_TPKEY.
 * @t: Transaction carrying the set-TPKEY request and response.
 *
 * Command format:
 *   CMD[0] = Operation info.
 *   CMD[1] = CRC word, left as 0 when CRC checking is disabled.
 *
 * Response format:
 *   RSP[0] = Unused.
 *   RSP[1] = Error status.
 *
 * Return: HWKM_SUCCESS on transport success, or a HWKM_ERR_* code.
 * Hardware command failure is reported in t->rsp.status.
 */
static int hwkm_set_tpkey_handle(struct hwkm_transaction *t)
{
	uint32_t cmd[HWKM_OP_SET_TPKEY_CMD_WORDS] = { 0 };
	uint32_t rsp[HWKM_OP_SET_TPKEY_RSP_WORDS] = { 0 };
	struct hwkm_operation_info op = { };
	int rc;

	if (!t || !t->hdl)
		return HWKM_ERR_INVALID_ARG;

	if (t->hdl->dest != HWKM_KEY_DEST_KM_MASTER)
		return HWKM_ERR_INVALID_DEST;

	op = (struct hwkm_operation_info){
		.op = HWKM_OP_SET_TPKEY,
		.irq_en = 0,
		.slot1_desc = t->cmd.set_tpkey.sks,
		.slot2_desc = 0,
		.op_flag = 0,
		.context_len = 0,
		.len = ARRAY_SIZE(cmd),
	};

	/* CMD[0]: */
	memcpy(&cmd[0], &op, sizeof(op));

	rc = run_transaction(t, cmd, ARRAY_SIZE(cmd), rsp, ARRAY_SIZE(rsp));
	/* On success, RSP[1]: */
	if (rc == HWKM_SUCCESS)
		t->rsp.status = rsp[HWKM_OP_SET_TPKEY_RSP_ERR_IDX];

	memset(cmd, 0, sizeof(cmd));
	memset(rsp, 0, sizeof(rsp));

	return rc;
}

/* Command dispatcher: */
static int hwkm_transaction_dispatch(struct hwkm_transaction *t)
{
	switch (t->cmd.op) {
	case HWKM_OP_NIST_KEYGEN:
		return hwkm_nist_keygen_handle(t);
	case HWKM_OP_SYSTEM_KDF:
		return hwkm_system_kdf_handle(t);
	case HWKM_OP_KEY_WRAP_EXPORT:
		return hwkm_key_wrap_export_handle(t);
	case HWKM_OP_KEY_UNWRAP_IMPORT:
		return hwkm_key_unwrap_import_handle(t);
	case HWKM_OP_KEY_SLOT_CLEAR:
		return hwkm_key_slot_clear_handle(t);
	case HWKM_OP_KEY_SLOT_RDWR:
		return hwkm_key_slot_rdwr_handle(t);
	case HWKM_OP_SET_TPKEY:
		return hwkm_set_tpkey_handle(t);
	default:
		return HWKM_ERR_NOT_SUPPORTED;
	}
}

/* Command queue. */

/**
 * hwkm_handle_init() - Initialize an HWKM handle.
 * @hdl: Handle to initialize.
 * @dest: Target hardware engine.
 *
 * Initializes @hdl for queuing commands to @dest.
 *
 * Return: HWKM_SUCCESS on success, or a HWKM_ERR_* code on failure.
 */
int hwkm_handle_init(struct hwkm_handle *hdl, enum hwkm_key_destination dest)
{
	switch (dest) {
	case HWKM_KEY_DEST_KM_MASTER:
		break;
	default:
		return HWKM_ERR_INVALID_DEST;
	}

	hdl->dest = dest;
	STAILQ_INIT(&hdl->queue);

	return HWKM_SUCCESS;
}

/**
 * hwkm_enqueue() - Enqueue a transaction on a handle.
 * @hdl: Handle that owns the queue.
 * @t: Transaction to enqueue.
 *
 * Associates @t with @hdl and appends it to the tail of the queue.
 *
 * Return: HWKM_SUCCESS on success, or a HWKM_ERR_* code on failure.
 */
int hwkm_enqueue(struct hwkm_handle *hdl, struct hwkm_transaction *t)
{
	if (!t)
		return HWKM_ERR_INVALID_ARG;

	if (t->hdl)
		return HWKM_ERR_INVALID_ARG;

	t->hdl = hdl;
	STAILQ_INSERT_TAIL(&hdl->queue, t, link);

	return HWKM_SUCCESS;
}

/**
 * hwkm_enqueue_many() - Enqueue multiple transactions on one handle.
 * @hdl: Handle that owns the queue.
 * @num_t: Number of transactions in @trans.
 * @trans: Array of transaction pointers to enqueue in FIFO order.
 *
 * Enqueues all transactions in @trans onto @hdl using hwkm_enqueue().
 * If enqueueing any transaction fails, all transactions already queued by
 * this call are removed again and their ownership is cleared, so the caller
 * sees all-or-nothing behavior.
 *
 * Return: HWKM_SUCCESS on success, or a HWKM_ERR_* code on failure.
 */
int hwkm_enqueue_many(struct hwkm_handle *hdl, size_t num_t,
		      struct hwkm_transaction *const trans[])
{
	size_t i = 0;
	int rc;

	if (!hdl || (num_t && !trans))
		return HWKM_ERR_INVALID_ARG;

	for (i = 0; i < num_t; i++) {
		rc = hwkm_enqueue(hdl, trans[i]);
		if (rc)
			goto rollback;
	}

	return HWKM_SUCCESS;

rollback:
	while (i > 0) {
		struct hwkm_transaction *t = trans[--i];
		/* Release the ownership. */
		STAILQ_REMOVE(&hdl->queue, t, hwkm_transaction, link);
		t->hdl = NULL;
	}

	return rc;
}

/**
 * hwkm_run_cmd_queue() - Execute queued transactions.
 * @hdl: Handle containing queued transactions
 *
 * Executes all transactions queued on @hdl in FIFO order by calling
 * hwkm_transaction_dispatch() for each transaction. Execution stops at the
 * first error and that error is returned to the caller.
 *
 * On return, transactions that were processed are removed from the queue and
 * their ownership is cleared (to be reused).
 *
 * Return: HWKM_SUCCESS on success, or a HWKM_ERR_* code on failure.
 */
int hwkm_run_cmd_queue(struct hwkm_handle *hdl)
{
	struct hwkm_transaction *t = NULL;
	int rc = HWKM_SUCCESS;

	while (!STAILQ_EMPTY(&hdl->queue)) {
		t = STAILQ_FIRST(&hdl->queue);
		rc = hwkm_transaction_dispatch(t);
		/* Release the ownership. */
		STAILQ_REMOVE_HEAD(&hdl->queue, link);
		t->hdl = NULL;

		if (rc != HWKM_SUCCESS)
			break;
	}

	return rc;
}

/* Run one transaction. */
int hwkm_run_transaction(enum hwkm_key_destination dest,
			 struct hwkm_transaction *t)
{
	struct hwkm_handle hdl;
	int rc;

	rc = hwkm_handle_init(&hdl, dest);
	if (rc)
		return rc;

	rc = hwkm_enqueue(&hdl, t);
	if (rc)
		return rc;

	return hwkm_run_cmd_queue(&hdl);
}
