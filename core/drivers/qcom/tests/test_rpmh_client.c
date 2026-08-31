/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 unit tests for rpmh_client.c - the AOP boot-cookie gated init
 * path, client-handle creation, and the send/wait/barrier public API.
 *
 * rpmh_client_init() is static, only reachable in production via the real
 * early_init() initcall; the mocked early_init() (mocks/initcall.h)
 * exposes it as rpmh_client_init_early_init_hook() so tests can drive it
 * directly. rpmh_send_command()'s internal vote path (issue_cmd_set_internal
 * -> rpmh_resource_command_update()) calls cmd_db_get_priority(), so any
 * test that sends a command must first install a matching CmdDb fixture
 * entry for that address (see cmd_db_fixture.h) - otherwise
 * rpmh_resource_command_get_voter() returns NULL into an unchecked
 * dereference in production code.
 */
#include <stdint.h>
#include <tee_api_types.h>

#include "cmd_db_fixture.h"
#include "drivers/qcom/rpmh/rpmh_client.h"
#include "harness.h"
#include "mocks/mock_panic.h"
#include "mocks/mock_regs.h"
#include "platform_config.h"
#include "rpmh_hal.h"
#include "rpmh_hwio.h"

#define TEST_ADDR	((FIXTURE_SLV_ID_ARC << 16) | 0x0010)
#define AOP_BOOT_COOKIE		0xA0C00C1E

/*
 * rpmh_client_init_early_init_hook() is declared extern in rpmh_client.c
 * via the mocked early_init() macro.
 */
extern TEE_Result (*rpmh_client_init_early_init_hook)(void);

static vaddr_t setup_msg_ram(void)
{
	vaddr_t va = mock_regs_add_region(AOP_MSG_RAM_BASE, AOP_MSG_RAM_SIZE);

	TEST_ASSERT_MSG(va != 0, "failed to allocate fake AOP msg ram region");

	return va;
}

static vaddr_t setup_rsc(void)
{
	vaddr_t va = mock_regs_add_region(RPMH_BASE_ADDR, RPMH_RSC_SIZE);

	TEST_ASSERT_MSG(va != 0, "failed to allocate fake RSC region");

	return va;
}

static void write_boot_cookie(vaddr_t msg_ram_base)
{
	/* check_aop_init() reads cookie_addr = base + dict->boot_cookie_offset,
	 * where dict lives at base + AOP_MSG_RAM_SIZE - MSG_RAM_SECTION_SIZE.
	 * dict->boot_cookie_offset defaults to 0 in a freshly-zeroed fixture
	 * region, so the cookie itself must be written at msg_ram_base + 0,
	 * NOT at dict's address (writing there would instead corrupt the
	 * boot_cookie_offset field, sending cookie_addr out of bounds).
	 */
	mock_regs_write32(msg_ram_base, AOP_BOOT_COOKIE);
}

static void install_matching_resource(void)
{
	struct fixture_db_header *hdr = cmd_db_fixture_install();

	cmd_db_fixture_add_entry(hdr, FIXTURE_SLV_ID_ARC, "test_res",
				 TEST_ADDR, 0, 0, NULL, 0);
}

static void full_init(void)
{
	vaddr_t msg_ram = setup_msg_ram();

	setup_rsc();
	write_boot_cookie(msg_ram);

	TEST_ASSERT_EQ_INT(rpmh_client_init_early_init_hook(), TEE_SUCCESS);
}

TEST(init_succeeds_when_aop_cookie_present)
{
	full_init();
}

TEST(init_panics_on_aop_boot_timeout)
{
	setup_msg_ram();
	setup_rsc();
	/* Cookie deliberately never written -> check_aop_init() times out. */

	MOCK_PANIC_EXPECT(rpmh_client_init_early_init_hook());
	TEST_ASSERT_MSG(mock_panic_hit,
			"expected panic on AOP boot-cookie timeout");
}

TEST(create_handle_accepts_secure_drv_rejects_others)
{
	full_init();

	TEST_ASSERT_MSG(rpmh_create_handle(RSC_DRV_SECURE, "c1") != NULL,
			"expected a handle for RSC_DRV_SECURE");
	TEST_ASSERT_MSG(rpmh_create_handle(RSC_DRV_HLOS, "c1") == NULL,
			"expected NULL for a non-secure drv_id");
	TEST_ASSERT_MSG(rpmh_create_handle(RSC_DRV_SECURE, NULL) == NULL,
			"expected NULL for a NULL client name");
}

TEST(send_command_completes_when_hw_reports_finished)
{
	vaddr_t rsc_va = 0;
	struct rpmh_client *handle = NULL;
	uint32_t req_id = 0;

	rsc_va = setup_rsc();
	write_boot_cookie(setup_msg_ram());
	TEST_ASSERT_EQ_INT(rpmh_client_init_early_init_hook(), TEE_SUCCESS);

	install_matching_resource();

	handle = rpmh_create_handle(RSC_DRV_SECURE, "c1");
	TEST_ASSERT_MSG(handle != NULL, "expected a valid client handle");

	/* Simulate the RSC hardware reporting tcs 0 (the first AMC slot)
	 * finished, so wait_for_cmd()'s hal_rpmh_get_amc_status() poll
	 * succeeds instead of timing out.
	 */
	mock_regs_write32(rsc_va + RSC_DRV_IRQ_STATUS, 1u << 0);

	TEST_ASSERT_EQ_INT(rpmh_send_command(handle, RPMH_SET_ACTIVE, true,
					     TEST_ADDR, 0x1234, &req_id),
			   TEE_SUCCESS);
	TEST_ASSERT_MSG(req_id != 0, "expected a non-zero req_id");

	rpmh_barrier_single(handle, req_id);
}

TEST(send_command_reports_busy_when_hw_never_finishes)
{
	struct rpmh_client *handle = NULL;
	uint32_t req_id = 0xFFFFFFFF;

	setup_rsc();
	write_boot_cookie(setup_msg_ram());
	TEST_ASSERT_EQ_INT(rpmh_client_init_early_init_hook(), TEE_SUCCESS);

	install_matching_resource();

	handle = rpmh_create_handle(RSC_DRV_SECURE, "c1");
	TEST_ASSERT_MSG(handle != NULL, "expected a valid client handle");

	/* IRQ_STATUS deliberately left clear -> wait_for_cmd() times out. */
	TEST_ASSERT_EQ_INT(rpmh_send_command(handle, RPMH_SET_ACTIVE, true,
					     TEST_ADDR, 0x1234, &req_id),
			   TEE_ERROR_BUSY);
}

TEST(send_command_rejects_bad_parameters)
{
	uint32_t req_id = 0;

	full_init();

	TEST_ASSERT_EQ_INT(rpmh_send_command(NULL, RPMH_SET_ACTIVE, true,
					     TEST_ADDR, 0x1234, &req_id),
			   TEE_ERROR_BAD_PARAMETERS);
}

const struct harness_test rpmh_client_tests[] = {
	TEST_CASE(init_succeeds_when_aop_cookie_present),
	TEST_CASE(init_panics_on_aop_boot_timeout),
	TEST_CASE(create_handle_accepts_secure_drv_rejects_others),
	TEST_CASE(send_command_completes_when_hw_reports_finished),
	TEST_CASE(send_command_reports_busy_when_hw_never_finishes),
	TEST_CASE(send_command_rejects_bad_parameters),
};

const int rpmh_client_tests_count =
	sizeof(rpmh_client_tests) / sizeof(rpmh_client_tests[0]);
