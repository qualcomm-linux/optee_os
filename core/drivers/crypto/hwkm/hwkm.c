// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc.
 */

#include <platform_config.h>

#include <drivers/clk_qcom.h>
#include <initcall.h>
#include <io.h>
#include <kernel/mutex.h>
#include <kernel/panic.h>
#include <kernel/tee_common_otp.h>
#include <malloc.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <string.h>
#include <string_ext.h>
#include <trace.h>
#include <util.h>

#include <hwkm_errno.h>
#include <hwkm_regs.h>
#include <hwkm.h>

#include "clock_group_qcom.h"

static_assert(HW_UNIQUE_KEY_LENGTH <= HWKM_MAX_KEY_SIZE);

#define HWKM_HUK_CTX		"OPTEE_HUK_HWKM_V1"
#define HWKM_HUK_KDK_SLOT	HWKM_SLOT_TZ_UKDK_L2
#define HWKM_SCRATCH_SLOT_A	HWKM_SLOT_TZ_GENERAL_PURPOSE_SLOT1
#define HWKM_SCRATCH_SLOT_B	HWKM_SLOT_TZ_GENERAL_PURPOSE_SLOT2

/*
 * BANK0 BBAC bitmap for the slots managed by this driver.
 *
 * BBAC_0 covers slots 0..31. Slots not present in this bitmap remain
 * inaccessible from BANK0.
 */
#define HWKM_BBAC_BIT(_slot)	BIT((_slot) % 32)

#define HWKM_BANK0_BBAC_0 \
	(HWKM_BBAC_BIT(HWKM_SLOT_NKDK_L1)                    | \
	 HWKM_BBAC_BIT(HWKM_SLOT_PKDK_L1)                    | \
	 HWKM_BBAC_BIT(HWKM_SLOT_SKDK_L1)                    | \
	 HWKM_BBAC_BIT(HWKM_SLOT_UKDK_L1)                    | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_NKDK_L2)                 | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_PKDK_L2)                 | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_SKDK_L2)                 | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_UKDK_L2)                 | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TPKEY_SLOT)                 | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TPKEY_ODD_SLOT)             | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_SWAP_KEY_SLOT)           | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_SWAP_KEY_ODD_SLOT)       | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_WRAP_KEY_SLOT)           | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_WRAP_KEY_ODD_SLOT)       | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_GENERAL_PURPOSE_SLOT1)   | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_GENERAL_PURPOSE_SLOT2)   | \
	 HWKM_BBAC_BIT(HWKM_SLOT_PERSISTENT_SHARED_SLOT_PAIR1)     | \
	 HWKM_BBAC_BIT(HWKM_SLOT_PERSISTENT_SHARED_SLOT_PAIR1_ODD) | \
	 HWKM_BBAC_BIT(HWKM_SLOT_PERSISTENT_SHARED_SLOT_PAIR2)     | \
	 HWKM_BBAC_BIT(HWKM_SLOT_PERSISTENT_SHARED_SLOT_PAIR2_ODD) | \
	 HWKM_BBAC_BIT(HWKM_SLOT_TZ_MIXING_KEY_SLOT))

static struct hwkm_drv_ctx g_hwkm_ctx = {
	.hwkm_lock = MUTEX_INITIALIZER,
};

struct hwkm_drv_ctx *hwkm_get_context(void)
{
	return g_hwkm_ctx.initialized ? &g_hwkm_ctx : NULL;
}

/**
 * hwkm_transaction_alloc() - Allocate and zero one HWKM transaction.
 *
 * Return: zero-initialized transaction on success, or NULL on failure.
 */
struct hwkm_transaction *hwkm_transaction_alloc(void)
{
	return calloc(1, sizeof(struct hwkm_transaction));
}

/**
 * hwkm_transaction_free() - Wipe and free one HWKM transaction.
 * @t: Transaction to release.
 */
void hwkm_transaction_free(struct hwkm_transaction *t)
{
	if (t) {
		memzero_explicit(t, sizeof(*t));
		free(t);
	}
}

/**
 * hwkm_setup_kdf() - Populate a KDF transaction.
 * @t: Transaction to fill.
 * @dks: Destination key slot.
 * @kdk: Source KDK slot.
 * @policy: Key policy for the derived key.
 * @bsve: Binding state vector.
 * @ctx: Software context bytes.
 * @ctx_len: Length of @ctx in bytes.
 */
static void hwkm_setup_kdf(struct hwkm_transaction *t,
			   uint8_t dks, uint8_t kdk,
			   const struct hwkm_key_policy *policy,
			   const struct hwkm_bsve *bsve,
			   const char *ctx, size_t ctx_len)
{
	t->cmd.op = HWKM_OP_SYSTEM_KDF;
	t->cmd.kdf.dks = dks;
	t->cmd.kdf.kdk = kdk;
	t->cmd.kdf.policy = *policy;
	t->cmd.kdf.bsve = *bsve;
	t->cmd.kdf.ctx_len = ctx_len;
	memcpy(t->cmd.kdf.ctx, ctx, ctx_len);
}

/**
 * tee_otp_get_hw_unique_key() - Return the platform hardware unique key.
 * @hwkey: Output buffer for the derived hardware unique key.
 *
 * The HUK is derived once through HWKM and then cached in the driver
 * context. Subsequent calls return the cached value.
 *
 * Slow path:
 *   Phase 1: Clear scratch slots A and B, derive the L3 KDK into slot A,
 *            then derive the L4 HUK from slot A into slot B.
 *   Phase 2: Read the L4 HUK from slot B, then clear both scratch slots.
 *
 * Return: TEE_SUCCESS on success, or a TEE_ERROR_* code on failure.
 */
TEE_Result tee_otp_get_hw_unique_key(struct tee_hw_unique_key *hwkey)
{
	struct hwkm_drv_ctx *drv;
	struct hwkm_transaction *t_clear_a = NULL;
	struct hwkm_transaction *t_clear_b = NULL;
	struct hwkm_transaction *t_kdf_l3 = NULL;
	struct hwkm_transaction *t_kdf_l4 = NULL;
	struct hwkm_transaction *t_read = NULL;
	/* L3: intermediate KDK derived from the root UKDK. */
	const struct hwkm_key_policy l3_policy = {
		.km_by_tz_allowed = true,
		.alg_allowed = HWKM_ALGO_AES256_CMAC,
		.enc_allowed = true,
		.dec_allowed = true,
		.key_type = HWKM_KEY_TYPE_KDK,
		.kdf_depth = 1,
		.security_lvl = HWKM_KEY_SECURITY_LVL_HW_KEY,
		.hw_destination = HWKM_KEY_DEST_KM_MASTER,
	};
	/* L4: final HUK, software-readable generic key. */
	const struct hwkm_key_policy l4_policy = {
		.km_by_tz_allowed = true,
		.alg_allowed = HWKM_ALGO_AES256_CMAC,
		.enc_allowed = true,
		.dec_allowed = true,
		.key_type = HWKM_KEY_TYPE_GENERIC_KEY,
		.kdf_depth = 0,
		.security_lvl = HWKM_KEY_SECURITY_LVL_SW_KEY,
		.hw_destination = HWKM_KEY_DEST_KM_MASTER,
	};
	const struct hwkm_bsve huk_bsve = {
		.enabled = true,
		.km_swc_en = true,
		.km_apps_secure_en = true,
	};
	TEE_Result res = TEE_SUCCESS;
	int rc = 0;

	if (!hwkey)
		return TEE_ERROR_BAD_PARAMETERS;

	drv = hwkm_get_context();
	if (!drv)
		return TEE_ERROR_NOT_SUPPORTED;

	mutex_lock(&drv->hwkm_lock);
	/* Fast path: return the cached HUK if it is already available. */
	if (drv->hwkm_huk_ready) {
		memcpy(hwkey->data, drv->hwkm_huk, HW_UNIQUE_KEY_LENGTH);
		goto out;
	}

	t_clear_a = hwkm_transaction_alloc();
	t_clear_b = hwkm_transaction_alloc();
	t_kdf_l3 = hwkm_transaction_alloc();
	t_kdf_l4 = hwkm_transaction_alloc();
	t_read = hwkm_transaction_alloc();
	if (!t_clear_a || !t_clear_b || !t_kdf_l3 || !t_kdf_l4 || !t_read) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	t_clear_a->cmd.op = HWKM_OP_KEY_SLOT_CLEAR;
	t_clear_a->cmd.clear.dks = HWKM_SCRATCH_SLOT_A;
	t_clear_a->cmd.clear.is_double_key = false;

	t_clear_b->cmd.op = HWKM_OP_KEY_SLOT_CLEAR;
	t_clear_b->cmd.clear.dks = HWKM_SCRATCH_SLOT_B;
	t_clear_b->cmd.clear.is_double_key = false;

	/* Phase 1: clear A and B, derive L3 KDK into A, derive L4 HUK into B. */

	hwkm_setup_kdf(t_kdf_l3, HWKM_SCRATCH_SLOT_A, HWKM_HUK_KDK_SLOT,
		       &l3_policy, &huk_bsve, HWKM_HUK_CTX,
		       sizeof(HWKM_HUK_CTX) - 1);
	hwkm_setup_kdf(t_kdf_l4, HWKM_SCRATCH_SLOT_B, HWKM_SCRATCH_SLOT_A,
		       &l4_policy, &huk_bsve, HWKM_HUK_CTX,
		       sizeof(HWKM_HUK_CTX) - 1);

	rc = hwkm_run_transactions(HWKM_KEY_DEST_KM_MASTER, 4,
				   (struct hwkm_transaction *const[]){
					t_clear_a, t_clear_b, t_kdf_l3, t_kdf_l4 });
	if (rc) {
		res = hwkm_to_tee(rc);
		goto out_clear;
	}

	/* Check for transactions status. */

	if (t_clear_a->rsp.status != HWKM_RSP_ERR_SUCCESS &&
	    t_clear_a->rsp.status != HWKM_CLEAR_ERR_DKS_SLOT_EMPTY) {
		res = TEE_ERROR_GENERIC;
		goto out;
	}

	if (t_clear_b->rsp.status != HWKM_RSP_ERR_SUCCESS &&
	    t_clear_b->rsp.status != HWKM_CLEAR_ERR_DKS_SLOT_EMPTY) {
		res = TEE_ERROR_GENERIC;
		goto out;
	}

	if (t_kdf_l3->rsp.status != HWKM_RSP_ERR_SUCCESS) {
		EMSG("hwkm: SYSTEM_KDF L3 failed: %#"PRIx32,
		     t_kdf_l3->rsp.status);
		res = TEE_ERROR_GENERIC;
		goto out_clear;
	}

	if (t_kdf_l4->rsp.status != HWKM_RSP_ERR_SUCCESS) {
		EMSG("hwkm: SYSTEM_KDF L4 failed: %#"PRIx32,
		     t_kdf_l4->rsp.status);
		res = TEE_ERROR_GENERIC;
		goto out_clear;
	}

	/* Phase 2: read L4 HUK from slot B, clear both scratch slots. */

	t_read->cmd.op = HWKM_OP_KEY_SLOT_RDWR;
	t_read->cmd.rdwr.slot = HWKM_SCRATCH_SLOT_B;
	t_read->cmd.rdwr.is_write = false;

	rc = hwkm_run_transactions(HWKM_KEY_DEST_KM_MASTER, 3,
				   (struct hwkm_transaction *const[]){
					t_read, t_clear_a, t_clear_b });
	if (rc) {
		res = hwkm_to_tee(rc);
		goto out_clear;
	}

	/* Check for transactions status. */

	if (t_read->rsp.status != HWKM_RSP_ERR_SUCCESS) {
		EMSG("hwkm: KEY_SLOT_RDWR read failed: %#"PRIx32,
		     t_read->rsp.status);
		res = TEE_ERROR_GENERIC;
		goto out_clear;
	}

	if (t_clear_a->rsp.status != HWKM_RSP_ERR_SUCCESS ||
	    t_clear_b->rsp.status != HWKM_RSP_ERR_SUCCESS) {
		res = TEE_ERROR_GENERIC;
		goto out;
	}

	memcpy(drv->hwkm_huk, t_read->rsp.rdwr.key, HW_UNIQUE_KEY_LENGTH);
	drv->hwkm_huk_ready = true;
	memcpy(hwkey->data, drv->hwkm_huk, HW_UNIQUE_KEY_LENGTH);

out_clear:
	if (res) {
		t_clear_a->hdl = NULL;
		t_clear_b->hdl = NULL;
		hwkm_run_transaction(HWKM_KEY_DEST_KM_MASTER, t_clear_a);
		hwkm_run_transaction(HWKM_KEY_DEST_KM_MASTER, t_clear_b);
	}

out:
	hwkm_transaction_free(t_clear_a);
	hwkm_transaction_free(t_clear_b);
	hwkm_transaction_free(t_kdf_l3);
	hwkm_transaction_free(t_kdf_l4);
	hwkm_transaction_free(t_read);

	mutex_unlock(&drv->hwkm_lock);

	return res;
}

/**
 * hwkm_map() - Map the KM_MASTER MMIO window.
 *
 * Return: mapped virtual base on success, or 0 on failure.
 */
static vaddr_t hwkm_map(void)
{
	void *va;

	va = core_mmu_add_mapping(MEM_AREA_IO_SEC, HWKM_MASTER_BASE,
				  HWKM_MASTER_SIZE);
	if (!va)
		return 0;

	return (vaddr_t)va;
}

/**
 * hwkm_init() - Initialize the HWKM instance.
 *
 * Initialization sequence:
 *   1. Enable the four GCC clock branches required by the HWKM Master.
 *   2. Map the master MMIO window.
 *   3. Check the hardware self-test status.
 *   4. Disable packet CRC checking.
 *   5. Program BANK0 BBAC for the slots managed by this driver.
 *   6. Clear the documented spurious RSP_FIFO_FULL sticky bit.
 *   7. Publish the mapped base and mark the driver initialized.
 *
 * Return: TEE_SUCCESS on success, or a TEE_ERROR_* code on failure.
 */
TEE_Result hwkm_init(void)
{
	uint32_t status;
	vaddr_t base;

	if (g_hwkm_ctx.initialized)
		return TEE_SUCCESS;

	base = hwkm_map();
	if (!base)
		return TEE_ERROR_GENERIC;

	/* Check the hardware self-test status. */
	status = HWKM_REG_READ(base, HWKM_TZ_KM_STATUS);
	if (status & (HWKM_TZ_KM_STATUS_BIST_ERROR |
		      HWKM_TZ_KM_STATUS_CRYPTO_LIB_BIST_ERROR))
		return TEE_ERROR_GENERIC;

	/* Disable CRC checking on command packets. */
	hwkm_reg_set_field(base, HWKM_TZ_KM_CTL,
			   HWKM_TZ_KM_CTL_CRC_CHECK_EN,
			   HWKM_TZ_KM_CTL_CRC_CHECK_EN_SHIFT, 0);

	HWKM_REG_WRITE(base, HWKM_BANK0_AC + HWKM_BANKn_AC_BBAC_0,
		       HWKM_BANK0_BBAC_0);
	HWKM_REG_WRITE(base, HWKM_BANK0_AC + HWKM_BANKn_AC_BBAC_1, 0);
	HWKM_REG_WRITE(base, HWKM_BANK0_AC + HWKM_BANKn_AC_BBAC_2, 0);
	HWKM_REG_WRITE(base, HWKM_BANK0_AC + HWKM_BANKn_AC_BBAC_3, 0);
	HWKM_REG_WRITE(base, HWKM_BANK0_AC + HWKM_BANKn_AC_BBAC_4, 0);

	/*
	 * Clear the spurious RSP_FIFO_FULL sticky bit.
	 * HW errata QCTDD06252768: RSP_FIFO_FULL may be set after reset even
	 * though the FIFO is empty. Write 1 to clear it unconditionally so
	 * it does not interfere with CMD_DONE polling in
	 * master_run_transaction().
	 */
	HWKM_REG_WRITE(base, HWKM_BANK0_KM_IRQ_STATUS,
		       HWKM_BANK0_KM_IRQ_STATUS_RSP_FIFO_FULL);

	g_hwkm_ctx.base = base;
	g_hwkm_ctx.initialized = true;

	return TEE_SUCCESS;
}

static TEE_Result hwkm_driver_init(void)
{
	struct tee_hw_unique_key huk = { };
	TEE_Result res;

	res = hwkm_init();
	if (res) {
		EMSG("hwkm: init failed: 0x%08"PRIx32, res);
		return res;
	}

	res = tee_otp_get_hw_unique_key(&huk);
	if (res) {
		EMSG("hwkm: HUK derivation failed: 0x%08"PRIx32, res);
		panic("HWKM HUK derivation failure");
	}

	memset(&huk, 0, sizeof(huk));

	return TEE_SUCCESS;
}

driver_init(hwkm_driver_init);
