/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 unit tests for rpmh_tcs.c - TCS allocation, AMC mode switching,
 * and the stuck-TCS timeout logic - driven against the real
 * g_drv_config_data (drv RSC_DRV_SECURE: tcs = RPMH_TCS_MAX = 4,
 * amcs = RPMH_TCS_SLEEP - RPMH_TCS_AMC = 2, sleep_start = 2, wake_start = 3).
 */
#include <stdint.h>
#include <types_ext.h>

#include "harness.h"
#include "mocks/mock_clock.h"
#include "mocks/mock_regs.h"
#include "rpmh_drv_config.h"
#include "rpmh_hal.h"
#include "rpmh_hwio.h"
#include "rpmh_target_config.h"
#include "rpmh_tcs.h"

#define TEST_RSC_REGION_SIZE	0x4000

static vaddr_t test_rsc_base(void)
{
	vaddr_t va = mock_regs_add_region(0x1000, TEST_RSC_REGION_SIZE);

	TEST_ASSERT_MSG(va != 0, "failed to allocate fake RSC region");

	return va;
}

static void init_tcs(vaddr_t base)
{
	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(rpmh_tcs_init(), TEE_SUCCESS);
}

static vaddr_t tcs_ctrl_addr(vaddr_t base, uint32_t tcs_id)
{
	return base + TCS_BASE_OFFSET + tcs_id * TCS_STRIDE + TCS_CONTROL_OFFSET;
}

static vaddr_t tcs_status_addr(vaddr_t base, uint32_t tcs_id)
{
	return base + TCS_BASE_OFFSET + tcs_id * TCS_STRIDE + TCS_STATUS_OFFSET;
}

TEST(drv_valid_and_find_index)
{
	uint32_t idx = 0xFFFFFFFF;

	TEST_ASSERT_MSG(rpmh_tcs_drv_valid(RSC_DRV_SECURE),
			"RSC_DRV_SECURE should be valid per target config");
	TEST_ASSERT_MSG(!rpmh_tcs_drv_valid(RSC_DRV_HLOS),
			"RSC_DRV_HLOS is not in target config");

	TEST_ASSERT_EQ_INT(rpmh_tcs_find_drv_index(RSC_DRV_SECURE, &idx),
			   TEE_SUCCESS);
	TEST_ASSERT_EQ_U32(idx, 0);

	TEST_ASSERT_EQ_INT(rpmh_tcs_find_drv_index(RSC_DRV_HLOS, &idx),
			   TEE_ERROR_ITEM_NOT_FOUND);
	TEST_ASSERT_EQ_INT(rpmh_tcs_find_drv_index(RSC_DRV_SECURE, NULL),
			   TEE_ERROR_BAD_PARAMETERS);
}

TEST(size_returns_cmds_per_tcs)
{
	uint32_t size = 0;

	TEST_ASSERT_EQ_INT(rpmh_tcs_size(RSC_DRV_SECURE, &size), TEE_SUCCESS);
	TEST_ASSERT_EQ_U32(size, RPMH_MAX_CMDS_PER_TCS);

	TEST_ASSERT_EQ_INT(rpmh_tcs_size(RSC_DRV_HLOS, &size),
			   TEE_ERROR_ITEM_NOT_FOUND);
	TEST_ASSERT_EQ_INT(rpmh_tcs_size(RSC_DRV_SECURE, NULL),
			   TEE_ERROR_BAD_PARAMETERS);
}

TEST(init_splits_tcs_into_amc_and_non_amc)
{
	vaddr_t base = test_rsc_base();
	uint32_t i = 0;

	init_tcs(base);

	/* amcs = 2: tcs 0,1 converted to AMC; tcs 2,3 left/converted to TCS. */
	for (i = 0; i < 2; i++) {
		uint32_t ctrl = mock_regs_read32(tcs_ctrl_addr(base, i));

		TEST_ASSERT_MSG((ctrl & TCS_CONTROL_AMC_MODE_EN) != 0,
				"expected AMC_MODE_EN set for AMC tcs");
	}

	for (i = 2; i < 4; i++) {
		uint32_t ctrl = mock_regs_read32(tcs_ctrl_addr(base, i));

		TEST_ASSERT_MSG((ctrl & TCS_CONTROL_AMC_MODE_EN) == 0,
				"expected AMC_MODE_EN clear for non-AMC tcs");
	}

	TEST_ASSERT_MSG(rpmh_tcs_is_amc_free(RSC_DRV_SECURE),
			"expected an idle AMC tcs right after init");
}

TEST(send_fills_amc_slots_then_reports_busy)
{
	vaddr_t base = test_rsc_base();
	struct rpmh_cmd_details details = {
		.address = 0x00020010,
		.data = 0x1234,
		.completion = true,
	};
	struct rpmh_cmd cmd = {
		.details = &details,
		.num_rcs = 1,
		.req_id = 7,
	};

	init_tcs(base);

	/* amcs == 2: two sends succeed, the third finds no idle AMC tcs. */
	TEST_ASSERT_EQ_INT(rpmh_tcs_send(&cmd, RSC_DRV_SECURE), TEE_SUCCESS);
	TEST_ASSERT_MSG(rpmh_tcs_is_amc_free(RSC_DRV_SECURE),
			"one AMC tcs should still be idle");

	TEST_ASSERT_EQ_INT(rpmh_tcs_send(&cmd, RSC_DRV_SECURE), TEE_SUCCESS);
	TEST_ASSERT_MSG(!rpmh_tcs_is_amc_free(RSC_DRV_SECURE),
			"no AMC tcs should be idle after filling both");

	TEST_ASSERT_EQ_INT(rpmh_tcs_send(&cmd, RSC_DRV_SECURE), TEE_ERROR_BUSY);
	TEST_ASSERT_EQ_INT(rpmh_tcs_send(NULL, RSC_DRV_SECURE),
			   TEE_ERROR_BAD_PARAMETERS);
}

TEST(finish_active_amc_reports_req_id_and_frees_slot)
{
	vaddr_t base = test_rsc_base();
	struct rpmh_cmd_details details = {
		.address = 0x00020010,
		.data = 0x1234,
		.completion = true,
	};
	struct rpmh_cmd cmd = {
		.details = &details,
		.num_rcs = 1,
		.req_id = 42,
	};
	uint32_t req_id = 0;

	init_tcs(base);

	TEST_ASSERT_EQ_INT(rpmh_tcs_send(&cmd, RSC_DRV_SECURE), TEE_SUCCESS);

	/* tcs 0 (id = 0, tcs_offset = 0) is the one just filled. Written
	 * directly (not via mock_regs_inject_read32(), which is one-shot)
	 * since both is_amc_finished() and finish_active_amc() below read
	 * IRQ_STATUS and both need to see the bit set.
	 */
	mock_regs_write32(base + RSC_DRV_IRQ_STATUS, 1u << 0);

	TEST_ASSERT_MSG(rpmh_tcs_is_amc_finished(RSC_DRV_SECURE),
			"expected amc finished after IRQ_STATUS injection");

	TEST_ASSERT_EQ_INT(rpmh_tcs_finish_active_amc(NULL, NULL,
						      RSC_DRV_SECURE, &req_id),
			   TEE_SUCCESS);
	TEST_ASSERT_EQ_U32(req_id, 42);

	TEST_ASSERT_MSG(rpmh_tcs_is_amc_free(RSC_DRV_SECURE),
			"tcs should be idle again after finish_active_amc");

	req_id = 0;
	TEST_ASSERT_EQ_INT(rpmh_tcs_finish_active_amc(NULL, NULL,
						      RSC_DRV_SECURE, &req_id),
			   TEE_ERROR_ITEM_NOT_FOUND);
	TEST_ASSERT_EQ_INT(rpmh_tcs_finish_active_amc(NULL, NULL,
						      RSC_DRV_SECURE, NULL),
			   TEE_ERROR_BAD_PARAMETERS);
}

TEST(slots_available_for_sleep_and_wake_sets)
{
	vaddr_t base = test_rsc_base();
	uint32_t tcs_index = 0xFFFFFFFF;

	init_tcs(base);

	/* mode: sleep_start = RPMH_TCS_SLEEP (2), wake_start = RPMH_TCS_WAKE (3). */
	TEST_ASSERT_EQ_INT(rpmh_tcs_slots_available(RSC_DRV_SECURE,
						    RPMH_SET_SLEEP, 1,
						    &tcs_index),
			   TEE_SUCCESS);
	TEST_ASSERT_EQ_U32(tcs_index, 2);

	TEST_ASSERT_EQ_INT(rpmh_tcs_slots_available(RSC_DRV_SECURE,
						    RPMH_SET_WAKE, 1,
						    &tcs_index),
			   TEE_SUCCESS);
	TEST_ASSERT_EQ_U32(tcs_index, 3);

	TEST_ASSERT_EQ_INT(rpmh_tcs_slots_available(RSC_DRV_SECURE,
						    RPMH_SET_ACTIVE, 1,
						    &tcs_index),
			   TEE_ERROR_BAD_PARAMETERS);

	TEST_ASSERT_EQ_INT(rpmh_tcs_slots_available(RSC_DRV_SECURE,
						    RPMH_SET_SLEEP,
						    RPMH_MAX_CMDS_PER_TCS + 1,
						    &tcs_index),
			   TEE_ERROR_BAD_PARAMETERS);
}

TEST(is_stuck_reflects_timeout_and_idle_status)
{
	vaddr_t base = test_rsc_base();
	struct rpmh_cmd_details details = {
		.address = 0x00020010,
		.data = 0x1234,
		.completion = true,
	};
	struct rpmh_cmd cmd = {
		.details = &details,
		.num_rcs = 1,
	};

	init_tcs(base);

	/* Non-zero send time so is_stuck()'s "!tcs->sent_at" guard doesn't
	 * mistake this send for "never sent".
	 */
	mock_clock_advance_us(1000);
	TEST_ASSERT_EQ_INT(rpmh_tcs_send(&cmd, RSC_DRV_SECURE), TEE_SUCCESS);

	TEST_ASSERT_MSG(!rpmh_tcs_is_stuck(RSC_DRV_SECURE),
			"should not be stuck immediately after send");

	mock_clock_advance_us(TCS_TIMEOUT_THRESHOLD + 10000);
	TEST_ASSERT_MSG(rpmh_tcs_is_stuck(RSC_DRV_SECURE),
			"expected stuck once past TCS_TIMEOUT_THRESHOLD");

	/* Marking the busy tcs (id 0) idle in hardware should clear stuck,
	 * even past the timeout - is_stuck() only flags non-idle tcs.
	 */
	mock_regs_write32(tcs_status_addr(base, 0), TCS_STATUS_CONTROLLER_IDLE);
	TEST_ASSERT_MSG(!rpmh_tcs_is_stuck(RSC_DRV_SECURE),
			"idle tcs should not be reported stuck");
}

TEST(get_finished_drv_passes_through_real_drvs)
{
	enum rsc_drv_id finished = RSC_DRV_VIRTUAL_MAX;

	TEST_ASSERT_EQ_INT(rpmh_tcs_get_finished_drv(RSC_DRV_SECURE, &finished),
			   TEE_SUCCESS);
	TEST_ASSERT_EQ_INT(finished, RSC_DRV_SECURE);

	TEST_ASSERT_EQ_INT(rpmh_tcs_get_finished_drv(RSC_DRV_SECURE, NULL),
			   TEE_ERROR_BAD_PARAMETERS);
}

const struct harness_test rpmh_tcs_tests[] = {
	TEST_CASE(drv_valid_and_find_index),
	TEST_CASE(size_returns_cmds_per_tcs),
	TEST_CASE(init_splits_tcs_into_amc_and_non_amc),
	TEST_CASE(send_fills_amc_slots_then_reports_busy),
	TEST_CASE(finish_active_amc_reports_req_id_and_frees_slot),
	TEST_CASE(slots_available_for_sleep_and_wake_sets),
	TEST_CASE(is_stuck_reflects_timeout_and_idle_status),
	TEST_CASE(get_finished_drv_passes_through_real_drvs),
};

const int rpmh_tcs_tests_count =
	sizeof(rpmh_tcs_tests) / sizeof(rpmh_tcs_tests[0]);
