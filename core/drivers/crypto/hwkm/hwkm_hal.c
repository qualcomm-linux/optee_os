// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc.
 */

#include <io.h>
#include <kernel/delay.h>
#include <kernel/panic.h>
#include <mm/core_memprot.h>
#include <string.h>
#include <trace.h>

#include "hwkm_hal.h"

#define HWKM_MASTER_PBASE       0x0U
#define HWKM_MASTER_SIZE        0x0U

#define HWKM_GPCE0_PBASE        0x0U
#define HWKM_GPCE0_SIZE         0x0U

/* MASTER instance: */

/* Shared RG. */
#define MASTER_KM_RG_SHARED_KM_KEYTABLE_SIZE   0x0000U
#define MASTER_KM_RG_SHARED_KM_IPCAT_VERSION   0x0004U
#define MASTER_KM_RG_SHARED_KM_KEY_POLICY_VER  0x0008U

/* TZ RG - control and status. */
#define MASTER_KM_RG_TZ_KM_CTL                 0x1000U
#define MASTER_KM_RG_TZ_KM_STATUS              0x1004U
#define MASTER_KM_RG_TZ_TPKEY_RECEIVE_CTL      0x1008U
#define MASTER_KM_RG_TZ_TPKEY_RECEIVE_STATUS   0x100CU

/* Bank 0 (TZ bankU - FIFO and control. */
#define MASTER_KM_RG_BANK0_CTL                 0x2000U
#define MASTER_KM_RG_BANK0_STATUS              0x2004U
#define MASTER_KM_RG_BANK0_IRQ_STATUS          0x2008U
#define MASTER_KM_RG_BANK0_ESR                 0x200CU
#define MASTER_KM_RG_BANK0_ESYNR               0x2010U
#define MASTER_KM_RG_BANK0_CMD                 0x2100U
#define MASTER_KM_RG_BANK0_RSP                 0x2200U

/* GPCE SLAVE instance: */

/* Shared RG (at) HWKM_GPCE0_PBASE. */
#define GPCE_KM_RG_SHARED_KM_IPCAT_VERSION    0x0000U
#define GPCE_KM_RG_SHARED_KM_KEY_POLICY_VER   0x0004U
#define GPCE_KM_RG_SHARED_KM_SHARED_STATUS    0x0004U
#define GPCE_KM_RG_SHARED_KM_KEYTABLE_SIZE    0x000cU

/* TZ RG (at) HWKM_GPCE0_PBASE + 0x1000. */
#define GPCE_KM_RG_TZ_KM_CTL                  0x1000U
#define GPCE_KM_RG_TZ_KM_STATUS               0x1004U
#define GPCE_KM_RG_TZ_TPKEY_RECEIVE_CTL       0x101cU
#define GPCE_KM_RG_TZ_TPKEY_RECEIVE_STATUS    0x1020U

/* Bank 0 - FIFO and control (at) HWKM_GPCE0_PBASE + 0x2000. */
#define GPCE_KM_RG_BANK0_CTL                  0x2000U
#define GPCE_KM_RG_BANK0_STATUS               0x2004U
#define GPCE_KM_RG_BANK0_IRQ_STATUS           0x2008U
#define GPCE_KM_RG_BANK0_ESR                  0x2010U
#define GPCE_KM_RG_BANK0_ESYNR                0x2018U
#define GPCE_KM_RG_BANK0_CMD                  0x201cU
#define GPCE_KM_RG_BANK0_RSP                  0x205cU

/* Register field masks / shifts. */

/* KM_CTL - CRC_CHECK_EN field */
#define KM_CTL_CRC_CHECK_EN_SHIFT       0
#define KM_CTL_CRC_CHECK_EN_MASK        0x1U

/* KM_STATUS - per-bit status flags */
#define KM_STATUS_BIST_DONE_SHIFT                  0
#define KM_STATUS_BIST_DONE_MASK                   0x1U
#define KM_STATUS_BIST_ERROR_SHIFT                 1
#define KM_STATUS_BIST_ERROR_MASK                  0x1U
#define KM_STATUS_CRYPTO_LIB_BIST_DONE_SHIFT       2
#define KM_STATUS_CRYPTO_LIB_BIST_DONE_MASK        0x1U
#define KM_STATUS_CRYPTO_LIB_BIST_ERROR_SHIFT      3
#define KM_STATUS_CRYPTO_LIB_BIST_ERROR_MASK       0x1U
#define KM_STATUS_BOOT_CMD_LIST0_DONE_SHIFT        4
#define KM_STATUS_BOOT_CMD_LIST0_DONE_MASK         0x1U
#define KM_STATUS_BOOT_CMD_LIST1_DONE_SHIFT        5
#define KM_STATUS_BOOT_CMD_LIST1_DONE_MASK         0x1U
#define KM_STATUS_KT_CLEAR_DONE_SHIFT              6
#define KM_STATUS_KT_CLEAR_DONE_MASK               0x1U

/* TPKEY_RECEIVE_CTL */
#define TPKEY_CTL_EN_SHIFT              0
#define TPKEY_CTL_EN_MASK               0x1U
#define TPKEY_CTL_DKS_SHIFT             8
#define TPKEY_CTL_DKS_MASK              0xFFU

/* TPKEY_RECEIVE_STATUS */
#define TPKEY_STATUS_DONE_SHIFT         0
#define TPKEY_STATUS_DONE_MASK          0x1U
#define TPKEY_STATUS_DKS_SHIFT          8
#define TPKEY_STATUS_DKS_MASK           0xFFU

/* BANK CTL */
#define BANK_CTL_CMD_FIFO_CLEAR_SHIFT   0
#define BANK_CTL_CMD_FIFO_CLEAR_MASK    0x1U
#define BANK_CTL_CMD_ENABLE_SHIFT       1
#define BANK_CTL_CMD_ENABLE_MASK        0x1U

/* BANK STATUS */
#define BANK_STATUS_CMD_FIFO_AVAIL_SHIFT  0
#define BANK_STATUS_CMD_FIFO_AVAIL_MASK   0xFFU
#define BANK_STATUS_RSP_FIFO_AVAIL_SHIFT  8
#define BANK_STATUS_RSP_FIFO_AVAIL_MASK   0xFFU

/* BANK IRQ_STATUS */
#define BANK_IRQ_CMD_DONE_SHIFT         0
#define BANK_IRQ_CMD_DONE_MASK          0x1U
#define BANK_IRQ_RSP_FIFO_FULL_SHIFT    1
#define BANK_IRQ_RSP_FIFO_FULL_MASK     0x1U

/* HW transaction success code (in rsp.status word) */
#define HWKM_TRANSACTION_SUCCESS        0U

/* GPCE slave default TPKEY slot */
#define CRYPTO_DEFAULT_TPKEY            UINT8_C(0x3EU

/* Command / response word counts. */
#define NIST_KEYGEN_CMD_WORDS           4
#define NIST_KEYGEN_RSP_WORDS           2
#define SYSTEM_KDF_CMD_MIN_WORDS        4
#define SYSTEM_KDF_CMD_MAX_WORDS        29
#define SYSTEM_KDF_RSP_WORDS            2
#define KEYSLOT_CLEAR_CMD_WORDS         2
#define KEYSLOT_CLEAR_RSP_WORDS         2
#define UNWRAP_IMPORT_CMD_WORDS         19
#define UNWRAP_IMPORT_RSP_WORDS         2
#define WRAP_EXPORT_CMD_WORDS           5
#define WRAP_EXPORT_RSP_WORDS           19
#define SET_TPKEY_CMD_WORDS             2
#define SET_TPKEY_RSP_WORDS             2
#define KEYSLOT_RDWR_CMD_WORDS          12
#define KEYSLOT_RDWR_RSP_WORDS          12

/* Field sizes in words. */
#define OPERATION_INFO_WORDS            1
#define KEY_POLICY_WORDS                2
#define BSVE_WORDS                      3
#define RESPONSE_KEY_WORDS              8
#define KEY_BLOB_WORDS                  17
/* Field sizes in bytes. */
#define OPERATION_INFO_LENGTH           (OPERATION_INFO_WORDS * 4)
#define KEY_POLICY_LENGTH               (KEY_POLICY_WORDS * 4)
#define MAX_BSVE_LENGTH                 (BSVE_WORDS * 4)
#define RESPONSE_KEY_LENGTH             (RESPONSE_KEY_WORDS * 4)
#define KEY_BLOB_LENGTH                 (KEY_BLOB_WORDS * 4)

/* Command packet word indices. */
#define CMD_KEY_POLICY_IDX              1
#define CMD_KEY_VALUE_IDX               3
#define CMD_WRAPPED_KEY_IDX             1
#define CMD_KEY_WRAP_BSVE_IDX           1
#define CMD_CTX_INDEX                   6

/* Response packet word indices. */
#define RSP_ERR_IDX                     1
#define RSP_KEY_POLICY_IDX              2
#define RSP_KEY_VALUE_IDX               4
#define RSP_WRAPPED_KEY_IDX             2

/* 1-word operation info field (CMD[0]) */
typedef struct {
	unsigned op : 4;          /* [3:0]   opcode */
	unsigned irq_en : 1;      /* [4]     IRQ enable (always 0 in SW) */
	unsigned slot1_desc : 8;  /* [12:5]  DKS or SKS */
	unsigned slot2_desc : 8;  /* [20:13] KDK, KWK, or 0 */
	unsigned op_flag : 1;     /* [21]    write flag / BSVE enable */
	unsigned context_len : 5; /* [26:22] SCL (KDF only) */
	unsigned len : 5;         /* [31:27] total cmd words */
} __packed hwkm_operation_info_t;

/*
 * hwkm_serialized_policy_t - 2-word (64-bit) HW wire format of a key policy.
 * Written directly into command packets and wrapped key blobs.
 */
typedef struct {
	unsigned dbg_qfprom_key_rd_iv_sel : 1;         /* [0] */
	unsigned reserved0 : 1;                        /* [1] */
	unsigned wrap_with_tpkey : 1;                  /* [2] */
	unsigned hw_destination : 4;                   /* [6:3] */
	unsigned reserved1 : 1;                        /* [7] */
	unsigned propagate_sec_level_to_children : 1;  /* [8] */
	unsigned security_level : 2;                   /* [10:9] */
	unsigned swap_export_allowed : 1;              /* [11] */
	unsigned wrap_export_allowed : 1;              /* [12] */
	unsigned key_type : 3;                         /* [15:13] */
	unsigned kdf_depth : 8;                        /* [23:16] */
	unsigned decrypt_allowed : 1;                  /* [24] */
	unsigned encrypt_allowed : 1;                  /* [25] */
	unsigned alg_allowed : 6;                      /* [31:26] */
	unsigned km_by_tz_allowed : 1;                 /* [32] */
	unsigned km_by_nsec_allowed : 1;               /* [33] */
	unsigned km_by_modem_allowed : 1;              /* [34] */
	unsigned km_by_spu_allowed : 1;                /* [35] */
	unsigned reserved2 : 28;                       /* [63:36] */
} __packed hwkm_serialized_policy_t;

/*
 * hwkm_kdf_bsve_t - 3-word (96-bit) HW wire format of the Binding
 * Specification Vector Extension used in SYSTEM_KDF command packets.
 */
typedef struct {
	unsigned mks : 8;                              /* [7:0] */
	unsigned key_policy_version_en : 1;            /* [8] */
	unsigned apps_secure_en : 1;                   /* [9] */
	unsigned msa_secure_en : 1;                    /* [10] */
	unsigned lcm_fuse_row_en : 1;                  /* [11] */
	unsigned boot_stage_otp_en : 1;                /* [12] */
	unsigned swc_en : 1;                           /* [13] */
	uint64_t fuse_region_sha_digest_en : 64;       /* [77:14] */
	unsigned child_key_policy_en : 1;              /* [78] */
	unsigned mks_en : 1;                           /* [79] */
	unsigned reserved : 16;                        /* [95:80] */
} __packed hwkm_kdf_bsve_t;

/*
 * hwkm_wrapping_bsve_t - 3-word (96-bit) HW wire format of the Binding
 * Specification Vector Extension used in KEY_WRAP_EXPORT command packets.
 */
typedef struct {
	unsigned key_policy_version_en : 1;            /* [0] */
	unsigned apps_secure_en : 1;                   /* [1] */
	unsigned msa_secure_en : 1;                    /* [2] */
	unsigned lcm_fuse_row_en : 1;                  /* [3] */
	unsigned boot_stage_otp_en : 1;                /* [4] */
	unsigned swc_en : 1;                           /* [5] */
	uint64_t fuse_region_sha_digest_en : 64;       /* [69:6] */
	unsigned child_key_policy_en : 1;              /* [70] */
	unsigned mks_en : 1;                           /* [71] */
	unsigned reserved : 24;                        /* [95:72] */
} __packed hwkm_wrapping_bsve_t;

/* Serialisation helpers. */

static void serialize_policy(hwkm_serialized_policy_t *out,
			     const hwkm_key_policy_t *p)
{
	memset(out, 0, sizeof(*out));
	out->wrap_with_tpkey = p->wrap_with_tpk_allowed ? 1 : 0;
	out->hw_destination = (unsigned)p->hw_destination;
	out->security_level = (unsigned)p->security_lvl;
	out->swap_export_allowed = p->swap_export_allowed ? 1 : 0;
	out->wrap_export_allowed = p->wrap_export_allowed ? 1 : 0;
	out->key_type = (unsigned)p->key_type;
	out->kdf_depth = p->kdf_depth;
	out->decrypt_allowed = p->dec_allowed ? 1 : 0;
	out->encrypt_allowed = p->enc_allowed ? 1 : 0;
	out->alg_allowed = (unsigned)p->alg_allowed;
	out->km_by_tz_allowed = p->km_by_tz_allowed ? 1 : 0;
	out->km_by_nsec_allowed = p->km_by_nsec_allowed ? 1 : 0;
	out->km_by_modem_allowed = p->km_by_modem_allowed ? 1 : 0;
	out->km_by_spu_allowed = p->km_by_spu_allowed ? 1 : 0;
}

static void deserialize_policy(hwkm_key_policy_t *out,
			       const hwkm_serialized_policy_t *p)
{
	memset(out, 0, sizeof(*out));
	out->wrap_with_tpk_allowed = p->wrap_with_tpkey ? true : false;
	out->hw_destination = (hwkm_destination_t)p->hw_destination;
	out->security_lvl = (hwkm_security_level_t)p->security_level;
	out->swap_export_allowed = p->swap_export_allowed ? true : false;
	out->wrap_export_allowed = p->wrap_export_allowed ? true : false;
	out->key_type = (hwkm_type_t)p->key_type;
	out->kdf_depth = (uint8_t)p->kdf_depth;
	out->dec_allowed = p->decrypt_allowed ? true : false;
	out->enc_allowed = p->encrypt_allowed ? true : false;
	out->alg_allowed = (hwkm_alg_t)p->alg_allowed;
	out->km_by_tz_allowed = p->km_by_tz_allowed ? true : false;
	out->km_by_nsec_allowed = p->km_by_nsec_allowed ? true : false;
	out->km_by_modem_allowed = p->km_by_modem_allowed ? true : false;
	out->km_by_spu_allowed = p->km_by_spu_allowed ? true : false;
}

static void serialize_kdf_bsve(hwkm_kdf_bsve_t *out,
			       const hwkm_bsve_t *bsve, uint8_t mks)
{
	memset(out, 0, sizeof(*out));
	out->mks = mks;
	out->key_policy_version_en = bsve->km_key_policy_ver_en ? 1 : 0;
	out->apps_secure_en = bsve->km_apps_secure_en ? 1 : 0;
	out->msa_secure_en = bsve->km_msa_secure_en ? 1 : 0;
	out->lcm_fuse_row_en = bsve->km_lcm_fuse_en ? 1 : 0;
	out->boot_stage_otp_en = bsve->km_boot_stage_otp_en ? 1 : 0;
	out->swc_en = bsve->km_swc_en ? 1 : 0;
	out->fuse_region_sha_digest_en = bsve->km_fuse_region_sha_digest_en;
	out->child_key_policy_en = bsve->km_child_key_policy_en ? 1 : 0;
	out->mks_en = bsve->km_mks_en ? 1 : 0;
}

static void serialize_wrapping_bsve(hwkm_wrapping_bsve_t *out,
				    const hwkm_bsve_t *bsve)
{
	memset(out, 0, sizeof(*out));
	out->key_policy_version_en = bsve->km_key_policy_ver_en ? 1 : 0;
	out->apps_secure_en = bsve->km_apps_secure_en ? 1 : 0;
	out->msa_secure_en = bsve->km_msa_secure_en ? 1 : 0;
	out->lcm_fuse_row_en = bsve->km_lcm_fuse_en ? 1 : 0;
	out->boot_stage_otp_en = bsve->km_boot_stage_otp_en ? 1 : 0;
	out->swc_en = bsve->km_swc_en ? 1 : 0;
	out->fuse_region_sha_digest_en = bsve->km_fuse_region_sha_digest_en;
	out->child_key_policy_en = bsve->km_child_key_policy_en ? 1 : 0;
	out->mks_en = bsve->km_mks_en ? 1 : 0;
}

/* Driver singleton. */

static hwkm_drv_ctx_t g_hwkm_ctx = {
	.lock = MUTEX_INITIALIZER,
};

hwkm_drv_ctx_t *hwkm_get_context(void)
{
	if (!g_hwkm_ctx.initialised)
		return NULL;
	return &g_hwkm_ctx;
}

/* HWKM MMIO accessors. */

static inline uint32_t hwkm_reg_read(vaddr_t base, uint32_t off)
{
	return io_read32(base + off);
}

static inline void hwkm_reg_write(vaddr_t base, uint32_t off, uint32_t val)
{
	io_write32(base + off, val);
}

/* Read-Mofdify-Write*/
static inline void hwkm_reg_rmw(vaddr_t base, uint32_t off, uint32_t mask,
				uint32_t shift, uint32_t val)
{
	uint32_t reg = hwkm_reg_read(base, off);

	reg &= ~(mask << shift);
	rreg |= (val & mask) << shift;
	hwkm_reg_write(base, off, red);
}

static inline uint32_t hwkm_reg_field(vaddr_t base, uint32_t off,
				      uint32_t mask, uint32_t shift)
{
	return (hwkm_reg_read(base, off) >> shift) & mask;
}

/* Key byte-order reversal (HW stores Key[255:224] at lowest address) */
static void reverse_bytes(uint8_t *buf, size_t len)
{
	size_t l = 0;
	size_t r = len;

	while (l < r && l < --r) {
		uint8_t tmp = buf[l];

		buf[l++] = buf[r];
		buf[r] = tmp;
	}
}

/* Context byte-swapping for NIST SP800-108 compliance */
static void reorder_kdf_ctx(uint8_t *bytes, size_t len)
{
	size_t i = 0;

	for (i = 0; i < len; i += sizeof(uint64_t)) {
		size_t chunk = len - i;

		if (chunk > sizeof(uint64_t))
			chunk = sizeof(uint64_t);
		reverse_bytes(bytes + i, chunk);
	}
}

#define MAX_RETRIES 10000

static int wait_for_field_nonzero(vaddr_t base, uint32_t off,
				  uint32_t mask, uint32_t shift,
				  int timeout_err)
{
	uint32_t retries = 0;

	while (hwkm_reg_field(base, off, mask, shift) == 0) {
		if (++retries > MAX_RETRIES)
			return timeout_err;
		udelay(10);
	}

	return HWKM_SUCCESS;
}

static int master_run_transaction(vaddr_t base,
				  const uint32_t *cmd, size_t cmd_words,
				  uint32_t *rsp, size_t rsp_words)
{
	size_t i = 0;
	int rc = 0;

	hwkm_reg_rmw(base, MASTER_KM_RG_BANK0_CTL,
		     BANK_CTL_CMD_FIFO_CLEAR_MASK,
		     BANK_CTL_CMD_FIFO_CLEAR_SHIFT, 1);
	hwkm_reg_rmw(base, MASTER_KM_RG_BANK0_CTL,
		     BANK_CTL_CMD_FIFO_CLEAR_MASK,
		     BANK_CTL_CMD_FIFO_CLEAR_SHIFT, 0);

	hwkm_reg_write(base, MASTER_KM_RG_BANK0_ESR,
		       hwkm_reg_read(base, MASTER_KM_RG_BANK0_ESR));

	hwkm_reg_rmw(base, MASTER_KM_RG_BANK0_CTL,
		     BANK_CTL_CMD_ENABLE_MASK,
		     BANK_CTL_CMD_ENABLE_SHIFT, 1);

	if (hwkm_reg_field(base, MASTER_KM_RG_BANK0_CTL,
			   BANK_CTL_CMD_FIFO_CLEAR_MASK,
			   BANK_CTL_CMD_FIFO_CLEAR_SHIFT) != 0)
		return HWKM_ERR_CMD_FIFO_NOT_EMPTY;

	for (i = 0; i < cmd_words; i++) {
		rc = wait_for_field_nonzero(base, MASTER_KM_RG_BANK0_STATUS,
					    BANK_STATUS_CMD_FIFO_AVAIL_MASK,
					    BANK_STATUS_CMD_FIFO_AVAIL_SHIFT,
					    HWKM_ERR_CMD_FIFO_TIMEOUT);
		if (rc)
			return rc;
		hwkm_reg_write(base, MASTER_KM_RG_BANK0_CMD, cmd[i]);
	}

	for (i = 0; i < rsp_words; i++) {
		rc = wait_for_field_nonzero(base, MASTER_KM_RG_BANK0_STATUS,
					    BANK_STATUS_RSP_FIFO_AVAIL_MASK,
					    BANK_STATUS_RSP_FIFO_AVAIL_SHIFT,
					    HWKM_ERR_RSP_FIFO_TIMEOUT);
		if (rc)
			return rc;
		rsp[i] = hwkm_reg_read(base, MASTER_KM_RG_BANK0_RSP);
	}

	if (hwkm_reg_field(base, MASTER_KM_RG_BANK0_IRQ_STATUS,
			   BANK_IRQ_CMD_DONE_MASK,
			   BANK_IRQ_CMD_DONE_SHIFT) == 0)
		return HWKM_ERR_RSP_OVERFLOW;

	hwkm_reg_rmw(base, MASTER_KM_RG_BANK0_IRQ_STATUS,
		     BANK_IRQ_CMD_DONE_MASK,
		     BANK_IRQ_CMD_DONE_SHIFT, 1);

	return HWKM_SUCCESS;
}

static int gpce_run_transaction(vaddr_t base,
				const uint32_t *cmd, size_t cmd_words,
				uint32_t *rsp, size_t rsp_words)
{
	/* TODO. Run GPCE Transaction. */

	return HWKM_SUCCESS;
}

/*
 * run_transaction - route a command packet to the correct per-instance
 *                   transaction function based on the destination.
 *
 * Adding a new slave type requires only a new case here and a new
 * xxx_run_transaction() function above - no existing code changes.
 */
static int run_transaction(const hwkm_handle_t *h,
			   const uint32_t *cmd, size_t cmd_words,
			   uint32_t *rsp, size_t rsp_words)
{
	switch (h->dest) {
	case KM_MASTER:
		return master_run_transaction(h->drv->master_base,
					      cmd, cmd_words, rsp, rsp_words);
	case GPCE_SLAVE:
		return gpce_run_transaction(h->drv->gpce0_base,
					    cmd, cmd_words, rsp, rsp_words);
	default:
		return HWKM_ERR_INVALID_DEST;
	}
}

/* Command handlers. */

static int submit_cmd(const hwkm_handle_t *h,
		      uint32_t *cmd, size_t cmd_words,
		      uint32_t *rsp, size_t rsp_words,
		      hwkm_rsp_t *out)
{
	int rc = run_transaction(h, cmd, cmd_words, rsp, rsp_words);

	out->status = rsp[RSP_ERR_IDX];
	return rc;
}

static int handle_nist_keygen(const hwkm_handle_t *h, hwkm_transaction_t *t)
{
	uint32_t cmd[NIST_KEYGEN_CMD_WORDS] = { 0 };
	uint32_t rsp[NIST_KEYGEN_RSP_WORDS] = { 0 };
	hwkm_serialized_policy_t policy = { };
	hwkm_operation_info_t op = {
		.op         = NIST_KEYGEN,
		.irq_en     = 0,
		.slot1_desc = t->cmd->keygen.dks,
		.len        = NIST_KEYGEN_CMD_WORDS,
	};

	serialize_policy(&policy, &t->cmd->keygen.policy);
	memcpy(cmd, &op, OPERATION_INFO_LENGTH);
	memcpy(cmd + CMD_KEY_POLICY_IDX, &policy, KEY_POLICY_LENGTH);

	return submit_cmd(h, cmd, NIST_KEYGEN_CMD_WORDS, rsp,
			  NIST_KEYGEN_RSP_WORDS, t->rsp);
}

static int handle_keyslot_clear(const hwkm_handle_t *h, hwkm_transaction_t *t)
{
	uint32_t cmd[KEYSLOT_CLEAR_CMD_WORDS] = { 0 };
	uint32_t rsp[KEYSLOT_CLEAR_RSP_WORDS] = { 0 };
	hwkm_operation_info_t op = {
		.op         = KEY_SLOT_CLEAR,
		.irq_en     = 0,
		.slot1_desc = t->cmd->clear.dks,
		.op_flag    = t->cmd->clear.is_double_key ? 1 : 0,
		.len        = KEYSLOT_CLEAR_CMD_WORDS,
	};
	int rc = 0;

	memcpy(cmd, &op, OPERATION_INFO_LENGTH);

	return submit_cmd(h, cmd, KEYSLOT_CLEAR_CMD_WORDS, rsp,
			  KEYSLOT_CLEAR_RSP_WORDS, , t->rsp);
}

static int handle_keyslot_rdwr(const hwkm_handle_t *h, hwkm_transaction_t *t)
{
	uint32_t cmd[KEYSLOT_RDWR_CMD_WORDS] = { 0 };
	uint32_t rsp[KEYSLOT_RDWR_RSP_WORDS] = { 0 };
	hwkm_serialized_policy_t policy = { };
	hwkm_operation_info_t op = {
		.op         = KEY_SLOT_RDWR,
		.irq_en     = 0,
		.slot1_desc = t->cmd->rdwr.slot,
		.op_flag    = t->cmd->rdwr.is_write ? 1 : 0,
		.len        = KEYSLOT_RDWR_CMD_WORDS,
	};
	int rc = 0;

	memcpy(cmd, &op, OPERATION_INFO_LENGTH);

	if (t->cmd->rdwr.is_write) {
		serialize_policy(&policy, &t->cmd->rdwr.policy);
		memcpy(cmd + CMD_KEY_POLICY_IDX, &policy, KEY_POLICY_LENGTH);
		memcpy(cmd + CMD_KEY_VALUE_IDX, t->cmd->rdwr.key, t->cmd->rdwr.sz);
		reverse_bytes((uint8_t *)(cmd + CMD_KEY_VALUE_IDX),
			      HWKM_MAX_KEY_SIZE);
	}

	rc = run_transaction(h, cmd, KEYSLOT_RDWR_CMD_WORDS,
			     rsp, KEYSLOT_RDWR_RSP_WORDS);
	t->rsp->status = rsp[RSP_ERR_IDX];

	if (!t->cmd->rdwr.is_write &&
	    t->rsp->status == HWKM_TRANSACTION_SUCCESS) {
		hwkm_serialized_policy_t rpol = { };

		memcpy(&rpol, rsp + RSP_KEY_POLICY_IDX, KEY_POLICY_LENGTH);
		deserialize_policy(&t->rsp->rdwr.policy, &rpol);
		t->rsp->rdwr.sz = HWKM_MAX_KEY_SIZE;
		memcpy(t->rsp->rdwr.key, rsp + RSP_KEY_VALUE_IDX,
		       RESPONSE_KEY_LENGTH);
		reverse_bytes(t->rsp->rdwr.key, HWKM_MAX_KEY_SIZE);
	}

	memset(cmd, 0, sizeof(cmd));
	memset(rsp, 0, sizeof(rsp));
	return rc;
}

static int handle_set_tpkey(const hwkm_handle_t *h, hwkm_transaction_t *t)
{
	uint32_t cmd[SET_TPKEY_CMD_WORDS] = { 0 };
	uint32_t rsp[SET_TPKEY_RSP_WORDS] = { 0 };
	hwkm_operation_info_t op = {
		.op         = SET_TPKEY,
		.irq_en     = 0,
		.slot1_desc = t->cmd->set_tpkey.sks,
		.len        = SET_TPKEY_CMD_WORDS,
	};
	int rc = 0;

	/* SET_TPKEY is a Master-only operation */
	if (h->dest != KM_MASTER)
		return HWKM_ERR_INVALID_DEST;

	memcpy(cmd, &op, OPERATION_INFO_LENGTH);

	return submit_cmd(h, cmd, SET_TPKEY_CMD_WORDS, rsp,
			  SET_TPKEY_RSP_WORDS, t->rsp);
}

static int handle_key_unwrap_import(const hwkm_handle_t *h,
				    hwkm_transaction_t *t)
{
	uint32_t cmd[UNWRAP_IMPORT_CMD_WORDS] = { 0 };
	uint32_t rsp[UNWRAP_IMPORT_RSP_WORDS] = { 0 };
	hwkm_operation_info_t op = {
		.op         = KEY_UNWRAP_IMPORT,
		.irq_en     = 0,
		.slot1_desc = t->cmd->unwrap.dks,
		.slot2_desc = t->cmd->unwrap.kwk,
		.len        = UNWRAP_IMPORT_CMD_WORDS,
	};
	int rc = 0;

	memcpy(cmd, &op, OPERATION_INFO_LENGTH);
	memcpy(cmd + CMD_WRAPPED_KEY_IDX, t->cmd->unwrap.wkb, t->cmd->unwrap.sz);

	rc = submit_cmd(h, cmd, UNWRAP_IMPORT_CMD_WORDS, rsp,
			UNWRAP_IMPORT_RSP_WORDS, t->rsp);

	memset(cmd, 0, sizeof(cmd));
	return rc;
}

static int handle_key_wrap_export(const hwkm_handle_t *h,
				  hwkm_transaction_t *t)
{
	uint32_t cmd[WRAP_EXPORT_CMD_WORDS] = { 0 };
	uint32_t rsp[WRAP_EXPORT_RSP_WORDS] = { 0 };
	hwkm_wrapping_bsve_t bsve = { };
	hwkm_operation_info_t op = {
		.op         = KEY_WRAP_EXPORT,
		.irq_en     = 0,
		.slot1_desc = t->cmd->wrap.sks,
		.slot2_desc = t->cmd->wrap.kwk,
		.len        = WRAP_EXPORT_CMD_WORDS,
	};
	int rc = 0;

	serialize_wrapping_bsve(&bsve, &t->cmd->wrap.bsve);
	memcpy(cmd, &op, OPERATION_INFO_LENGTH);
	if (t->cmd->wrap.bsve.enabled)
		memcpy(cmd + CMD_KEY_WRAP_BSVE_IDX, &bsve, MAX_BSVE_LENGTH);

	rc = submit_cmd(h, cmd, WRAP_EXPORT_CMD_WORDS, rsp,
			WRAP_EXPORT_RSP_WORDS, t->rsp);

	if (t->rsp->status == HWKM_TRANSACTION_SUCCESS) {
		t->rsp->wrap.sz = KEY_BLOB_LENGTH;
		memcpy(t->rsp->wrap.wkb, rsp + RSP_WRAPPED_KEY_IDX,
		       KEY_BLOB_LENGTH);
	}

	memset(rsp, 0, sizeof(rsp));
	return rc;
}

static int handle_system_kdf(const hwkm_handle_t *h, hwkm_transaction_t *t)
{
	uint32_t cmd[SYSTEM_KDF_CMD_MAX_WORDS] = { 0 };
	uint32_t rsp[SYSTEM_KDF_RSP_WORDS] = { 0 };
	hwkm_serialized_policy_t policy = { };
	size_t scl = (t->cmd->kdf.sz + 3) / 4;
	size_t base_len = SYSTEM_KDF_CMD_MIN_WORDS +
			  (t->cmd->kdf.bsve.enabled ? BSVE_WORDS : 1);
	hwkm_operation_info_t op = {
		.op          = SYSTEM_KDF,
		.irq_en      = 0,
		.slot1_desc  = t->cmd->kdf.dks,
		.slot2_desc  = t->cmd->kdf.kdk,
		.op_flag     = t->cmd->kdf.bsve.enabled ? 1 : 0,
		.context_len = (unsigned)scl,
		.len         = (unsigned)base_len,
	};
	int rc = 0;

	serialize_policy(&policy, &t->cmd->kdf.policy);

	memcpy(cmd, &op, OPERATION_INFO_LENGTH);
	memcpy(cmd + CMD_KEY_POLICY_IDX, &policy, KEY_POLICY_LENGTH);

	if (t->cmd->kdf.bsve.enabled) {
		hwkm_kdf_bsve_t bsve = { };

		serialize_kdf_bsve(&bsve, &t->cmd->kdf.bsve, t->cmd->kdf.mks);
		memcpy(cmd + 3, &bsve, MAX_BSVE_LENGTH);
	}

	memcpy(cmd + CMD_CTX_INDEX, t->cmd->kdf.ctx, t->cmd->kdf.sz);
	reorder_kdf_ctx((uint8_t *)(cmd + CMD_CTX_INDEX), t->cmd->kdf.sz);

	rc = submit_cmd(h, cmd, base_len + scl, rsp, SYSTEM_KDF_RSP_WORDS,
			t->rsp);

	memset(cmd, 0, sizeof(cmd));
	return rc;
}

/* Transaction dispatcher. */
static int dispatch(const hwkm_handle_t *h, hwkm_transaction_t *t)
{
	switch (t->cmd->op) {
	case NIST_KEYGEN:
		return handle_nist_keygen(h, t);
	case SYSTEM_KDF:
		return handle_system_kdf(h, t);
	case KEY_WRAP_EXPORT:
		return handle_key_wrap_export(h, t);
	case KEY_UNWRAP_IMPORT:
		return handle_key_unwrap_import(h, t);
	case KEY_SLOT_CLEAR:
		return handle_keyslot_clear(h, t);
	case KEY_SLOT_RDWR:
		return handle_keyslot_rdwr(h, t);
	case SET_TPKEY:
		return handle_set_tpkey(h, t);
	default:
		return HWKM_ERR_NOT_SUPPORTED;
	}
}

/* Public API. */

 /**
 * hwkm_handle_init() - Initialise a transaction handle.
 * @h:     handle to initialise
 * @drv:   driver context from hwkm_get_context()
 * @dest:  KM_MASTER or GPCE_SLAVE
 */
int hwkm_handle_init(hwkm_handle_t *h, hwkm_drv_ctx_t *drv,
		     hwkm_destination_t dest)
{
	if (!h || !drv || !drv->initialised)
		return HWKM_ERR_INVALID_ARG;

	if (dest != KM_MASTER && dest != GPCE_SLAVE)
		return HWKM_ERR_INVALID_DEST;

	h->drv   = drv;
	h->dest  = dest;
	h->front = NULL;
	h->end   = NULL;
	return HWKM_SUCCESS;
}

/**
 * hwkm_add_req() - Enqueue one command/response pair onto a handle.
 * @h:   Handle previously initialised by hwkm_handle_init().
 * @t:   Caller-allocated transaction node.
 * @cmd: Command to issue.
 * @rsp: Buffer to receive the hardware response.
 *
 * Appends @t to the tail of @h's pending queue. No hardware access occurs
 * here; the command is not submitted until hwkm_run_cmds() is called.
 * @t, @cmd, and @rsp must remain valid until hwkm_run_cmds() returns.
 *
 * Return: HWKM_SUCCESS or HWKM_ERR_INVALID_ARG if any pointer is NULL.
 */
int hwkm_add_req(hwkm_handle_t *h, hwkm_transaction_t *t,
		 hwkm_cmd_t *cmd, hwkm_rsp_t *rsp)
{
	if (!h || !t || !cmd || !rsp)
		return HWKM_ERR_INVALID_ARG;

	t->cmd  = cmd;
	t->rsp  = rsp;
	t->next = NULL;

	if (!h->end) {
		h->front = t;
		h->end   = t;
	} else {
		h->end->next = t;
		h->end       = t;
	}
	return HWKM_SUCCESS;
}

/**
 * hwkm_run_cmds() - Submit all queued commands to hardware in order.
 * @h: Handle whose pending queue is to be executed.
 *
 * Acquires the driver mutex, then iterates the transaction queue, packing
 * each hwkm_cmd_t into the hardware command FIFO and reading the response
 * back into the corresponding hwkm_rsp_t. The mutex is released and the
 * queue is cleared before returning, regardless of whether an error occurs.
 *
 * The handle must not be reused after this function returns. Re-initialise
 * it with hwkm_handle_init() if further commands are needed.
 *
 * Return: HWKM_SUCCESS on success, or one of the HWKM_ERR_* codes if a
 *         FIFO timeout, response overflow, or hardware error is detected.
 */
int hwkm_run_cmds(hwkm_handle_t *h)
{
	hwkm_transaction_t *t = NULL;
	int rc = HWKM_SUCCESS;

	if (!h || !h->drv)
		return HWKM_ERR_INVALID_ARG;

	for (t = h->front; t; t = t->next) {
		rc = dispatch(h, t);
		if (rc != HWKM_SUCCESS)
			break;
	}

	/* Clear the queue (nodes are caller-owned, no free needed) */
	h->front = NULL;
	h->end   = NULL;
	return rc;
}

TEE_Result hwkm_hal_init(void)
{
	hwkm_drv_ctx_t *drv = &g_hwkm_ctx;

	if (drv->initialised)
		return TEE_SUCCESS;

	IMSG("hwkm: mapping MMIO regions");

	/* Map all MMIO regions once. */
	drv->master_base = (vaddr_t)phys_to_virt(HWKM_MASTER_PBASE,
						 MEM_AREA_IO_SEC,
						 HWKM_MASTER_SIZE);
	drv->gpce0_base  = (vaddr_t)phys_to_virt(HWKM_GPCE0_PBASE,
						 MEM_AREA_IO_SEC,
						 HWKM_GPCE0_SIZE);

	if (!drv->master_base || !drv->gpce0_base) {
		EMSG("hwkm: MMIO mapping failed");
		return TEE_ERROR_GENERIC;
	}

	/* TODO. Do the rest of initialization, including TPKEY genration. */

	drv->initialised = true;
	IMSG("hwkm: HAL initialised");
	return TEE_SUCCESS;
}
