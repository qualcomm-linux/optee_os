/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 unit tests for rpmh_resource_commands.c - the vote reconciliation
 * (dirty-state) logic and rpmh_find_resource_command()'s binary search.
 * rpmh_resource_command_get_voter() calls cmd_db_get_priority() on the
 * first vote for a new drv, so every test here installs a fake CmdDb blob
 * (via cmd_db_fixture.h) with a matching resource entry before voting.
 */
#include <stdint.h>

#include "cmd_db_fixture.h"
#include "drivers/qcom/cmd_db/cmd_db.h"
#include "harness.h"
#include "mocks/mock_regs.h"
#include "rpmh_resource_commands.h"

#define TEST_ADDR	((FIXTURE_SLV_ID_ARC << 16) | 0x0010)

static void install_priority_entry(uint32_t addr, uint32_t priority0)
{
	struct fixture_db_header *hdr = cmd_db_fixture_install();

	cmd_db_fixture_add_entry(hdr, FIXTURE_SLV_ID_ARC, "test_res", addr,
				 priority0, 0, NULL, 0);
}

TEST(update_active_set_is_dirty_on_first_vote)
{
	struct rpmh_resource_command rc = { 0 };
	bool dirty = false;

	install_priority_entry(TEST_ADDR, 0);
	rpmh_resource_command_init(&rc, TEST_ADDR);

	dirty = rpmh_resource_command_update(&rc, RPMH_SET_ACTIVE, 0x1234,
					     true, RSC_DRV_SECURE, false);
	TEST_ASSERT_MSG(dirty, "first ACTIVE vote must be dirty");
}

TEST(update_active_set_is_clean_when_matching_vote_at_rpmh)
{
	struct rpmh_resource_command rc = { 0 };
	struct drv_votes *drv = NULL;
	bool dirty = false;

	install_priority_entry(TEST_ADDR, 0);
	rpmh_resource_command_init(&rc, TEST_ADDR);

	/* Simulate a previous send having landed at 0x1234 with completion. */
	dirty = rpmh_resource_command_update(&rc, RPMH_SET_ACTIVE, 0x1234,
					     true, RSC_DRV_SECURE, false);
	TEST_ASSERT_MSG(dirty, "first vote should be dirty");

	drv = rc.drv_votes;
	TEST_ASSERT_MSG(drv != NULL, "expected a drv_votes entry after update");
	drv->vote_at_rpmh = drv->local_votes[RPMH_SET_ACTIVE];

	/* Re-voting the identical value/completion should now read clean. */
	dirty = rpmh_resource_command_update(&rc, RPMH_SET_ACTIVE, 0x1234,
					     true, RSC_DRV_SECURE, false);
	TEST_ASSERT_MSG(!dirty, "re-voting identical value should be clean");
}

TEST(update_sleep_set_dirty_tracks_active_mismatch)
{
	struct rpmh_resource_command rc = { 0 };
	bool dirty = false;

	install_priority_entry(TEST_ADDR, 0);
	rpmh_resource_command_init(&rc, TEST_ADDR);

	rpmh_resource_command_update(&rc, RPMH_SET_ACTIVE, 0x1234, false,
				     RSC_DRV_SECURE, false);

	/* sleep vote differs from active -> dirty. */
	dirty = rpmh_resource_command_update(&rc, RPMH_SET_SLEEP, 0x5678,
					     false, RSC_DRV_SECURE, false);
	TEST_ASSERT_MSG(dirty, "sleep vote differing from active must be dirty");

	/* sleep vote matches active -> clean. */
	dirty = rpmh_resource_command_update(&rc, RPMH_SET_SLEEP, 0x1234,
					     false, RSC_DRV_SECURE, false);
	TEST_ASSERT_MSG(!dirty, "sleep vote matching active must be clean");
}

TEST(update_explicit_sleep_vote_stays_clean_explicit)
{
	struct rpmh_resource_command rc = { 0 };
	struct drv_votes *drv = NULL;
	bool dirty = false;

	install_priority_entry(TEST_ADDR, 0);
	rpmh_resource_command_init(&rc, TEST_ADDR);

	rpmh_resource_command_update(&rc, RPMH_SET_ACTIVE, 0x1234, false,
				     RSC_DRV_SECURE, false);

	/* explicit_cmd marks sleep RPMH_RC_CLEAN_EXPLICIT_VOTE, which the
	 * reconcile logic must not override even though data mismatches
	 * active.
	 */
	dirty = rpmh_resource_command_update(&rc, RPMH_SET_SLEEP, 0x9999,
					     false, RSC_DRV_SECURE, true);
	TEST_ASSERT_MSG(!dirty,
			"explicit sleep vote should report clean (not > LAST_CLEAN)");

	drv = rc.drv_votes;
	TEST_ASSERT_EQ_INT(drv->local_votes[RPMH_SET_SLEEP].dirty,
			   RPMH_RC_CLEAN_EXPLICIT_VOTE);
}

TEST(find_resource_command_returns_null_when_empty)
{
	TEST_ASSERT_MSG(rpmh_find_resource_command(TEST_ADDR) == NULL,
			"expected NULL with zero registered resources");
}

const struct harness_test rpmh_resource_commands_tests[] = {
	TEST_CASE(update_active_set_is_dirty_on_first_vote),
	TEST_CASE(update_active_set_is_clean_when_matching_vote_at_rpmh),
	TEST_CASE(update_sleep_set_dirty_tracks_active_mismatch),
	TEST_CASE(update_explicit_sleep_vote_stays_clean_explicit),
	TEST_CASE(find_resource_command_returns_null_when_empty),
};

const int rpmh_resource_commands_tests_count =
	sizeof(rpmh_resource_commands_tests) /
	sizeof(rpmh_resource_commands_tests[0]);
