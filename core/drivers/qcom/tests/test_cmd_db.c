/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 unit tests for cmd_db.c - init magic/version validation and the
 * cmd_db_get_addr()/cmd_db_get_priority() lookup paths, driven against a
 * fake CmdDb blob built via cmd_db_fixture.h.
 */
#include <stdint.h>
#include <tee_api_types.h>

#include "cmd_db_fixture.h"
#include "drivers/qcom/cmd_db/cmd_db.h"
#include "harness.h"
#include "mocks/mock_panic.h"
#include "mocks/mock_regs.h"
#include "platform_config.h"

#define TEST_ADDR	((FIXTURE_SLV_ID_ARC << 16) | 0x0010)

/* cmd_db_init_early_init_hook() is declared extern in cmd_db_fixture.c;
 * redeclared here to drive the bad-magic panic path directly, bypassing
 * cmd_db_fixture_install() (which always writes a valid magic/version).
 */
extern TEE_Result (*cmd_db_init_early_init_hook)(void);

TEST(init_valid_magic_and_version_returns_success)
{
	struct fixture_db_header *hdr = cmd_db_fixture_install();

	TEST_ASSERT_MSG(hdr != NULL, "expected a fake CmdDb header");
}

TEST(init_bad_magic_panics)
{
	vaddr_t va = mock_regs_add_region(AOP_CMD_DB_BASE, AOP_CMD_DB_SIZE);
	struct fixture_db_header *hdr = (struct fixture_db_header *)va;

	TEST_ASSERT_MSG(va != 0, "failed to allocate fake CmdDb region");

	hdr->version = FIXTURE_CMD_DB_VER;
	hdr->magic_num = 0xdeadbeef;

	MOCK_PANIC_EXPECT(cmd_db_init_early_init_hook());
	TEST_ASSERT_MSG(mock_panic_hit, "expected panic on magic mismatch");
}

TEST(get_addr_finds_registered_resource)
{
	struct fixture_db_header *hdr = cmd_db_fixture_install();
	uint32_t addr = 0;

	cmd_db_fixture_add_entry(hdr, FIXTURE_SLV_ID_ARC, "test_res",
				 TEST_ADDR, 0, 0, NULL, 0);

	TEST_ASSERT_EQ_INT(cmd_db_get_addr("test_res", &addr), TEE_SUCCESS);
	TEST_ASSERT_EQ_U32(addr, TEST_ADDR);
}

TEST(get_addr_returns_not_found_for_unknown_res_id)
{
	struct fixture_db_header *hdr = cmd_db_fixture_install();
	uint32_t addr = 0;

	cmd_db_fixture_add_entry(hdr, FIXTURE_SLV_ID_ARC, "test_res",
				 TEST_ADDR, 0, 0, NULL, 0);

	TEST_ASSERT_EQ_INT(cmd_db_get_addr("no_such", &addr),
			   TEE_ERROR_ITEM_NOT_FOUND);
}

TEST(get_addr_rejects_bad_parameters)
{
	uint32_t addr = 0;

	cmd_db_fixture_install();

	TEST_ASSERT_EQ_INT(cmd_db_get_addr(NULL, &addr),
			   TEE_ERROR_BAD_PARAMETERS);
	TEST_ASSERT_EQ_INT(cmd_db_get_addr("test_res", NULL),
			   TEE_ERROR_BAD_PARAMETERS);
}

TEST(get_priority_extracts_low_drv_bits)
{
	struct fixture_db_header *hdr = cmd_db_fixture_install();
	uint32_t priority = 0;

	/* drv_id 0 occupies bits [1:0] of priority[0]; value 3 == 0b11. */
	cmd_db_fixture_add_entry(hdr, FIXTURE_SLV_ID_ARC, "test_res",
				 TEST_ADDR, 0x3, 0, NULL, 0);

	TEST_ASSERT_EQ_INT(cmd_db_get_priority(TEST_ADDR, 0, &priority),
			   TEE_SUCCESS);
	TEST_ASSERT_EQ_U32(priority, 0x3);
}

TEST(get_priority_returns_bad_parameters_for_invalid_slv_id)
{
	uint32_t priority = 0;
	uint32_t bad_addr = (0x1 << 16) | 0x0010; /* slv_id 1 is invalid */

	cmd_db_fixture_install();

	TEST_ASSERT_EQ_INT(cmd_db_get_priority(bad_addr, 0, &priority),
			   TEE_ERROR_BAD_PARAMETERS);
}

const struct harness_test cmd_db_tests[] = {
	TEST_CASE(init_valid_magic_and_version_returns_success),
	TEST_CASE(init_bad_magic_panics),
	TEST_CASE(get_addr_finds_registered_resource),
	TEST_CASE(get_addr_returns_not_found_for_unknown_res_id),
	TEST_CASE(get_addr_rejects_bad_parameters),
	TEST_CASE(get_priority_extracts_low_drv_bits),
	TEST_CASE(get_priority_returns_bad_parameters_for_invalid_slv_id),
};

const int cmd_db_tests_count =
	sizeof(cmd_db_tests) / sizeof(cmd_db_tests[0]);
