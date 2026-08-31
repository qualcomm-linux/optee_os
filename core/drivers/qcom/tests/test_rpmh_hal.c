/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 unit tests for rpmh_hal.c - the register-encoding HAL layer.
 *
 * The two READ_TEST_CMD_ENCODING/MSG_LENGTH tests below directly cover the
 * regression this whole framework exists to catch: hal_rpmh_write_cmd()
 * previously set the MSGID_READ bit instead of MSGID_WRITE, and encoded
 * MSG_LENGTH as 1 instead of the correct 8 (a single 32-bit RPMh command
 * word is always transferred as 8 bytes per the wire protocol).
 */
#include <stdint.h>
#include <types_ext.h>

#include "harness.h"
#include "mocks/mock_regs.h"
#include "rpmh_hal.h"
#include "rpmh_hwio.h"

/* Big enough to cover one DRV_STRIDE window with headroom for TCS/CMD
 * offsets used by these tests (TCS_BASE_OFFSET + a few TCS_STRIDE slots).
 */
#define TEST_RSC_REGION_SIZE	0x4000

static vaddr_t test_rsc_base(void)
{
	vaddr_t va = mock_regs_add_region(0x1000, TEST_RSC_REGION_SIZE);

	TEST_ASSERT_MSG(va != 0, "failed to allocate fake RSC region");

	return va;
}

static vaddr_t cmd_msgid_addr(vaddr_t drv_base, uint32_t tcs_id,
			      uint32_t cmd_idx)
{
	vaddr_t tcs_base = drv_base + TCS_BASE_OFFSET + tcs_id * TCS_STRIDE;
	vaddr_t cmd_base = tcs_base + TCS_CMD_BASE_OFFSET +
			   cmd_idx * TCS_CMD_STRIDE;

	return cmd_base + TCS_CMDn_MSGID_OFFSET;
}

static vaddr_t cmd_addr_addr(vaddr_t drv_base, uint32_t tcs_id,
			     uint32_t cmd_idx)
{
	return cmd_msgid_addr(drv_base, tcs_id, cmd_idx) -
	       TCS_CMDn_MSGID_OFFSET + TCS_CMDn_ADDR_OFFSET;
}

static vaddr_t cmd_data_addr(vaddr_t drv_base, uint32_t tcs_id,
			     uint32_t cmd_idx)
{
	return cmd_msgid_addr(drv_base, tcs_id, cmd_idx) -
	       TCS_CMDn_MSGID_OFFSET + TCS_CMDn_DATA_OFFSET;
}

TEST(write_cmd_encodes_write_not_read)
{
	vaddr_t base = test_rsc_base();
	uint32_t msgid = 0;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_write_cmd(RSC_DRV_SECURE, 0, 0,
					      0x00020010, 0x1234, true),
			   HAL_STATUS_SUCCESS);

	msgid = mock_regs_read32(cmd_msgid_addr(base, 0, 0));

	/*
	 * Regression guard: bit MSGID_READ_OR_WRITE_SHIFT must read
	 * MSGID_WRITE (1). The bug this framework was built to catch left
	 * this bit at MSGID_READ (0), silently turning every RPMh write
	 * into a read.
	 */
	TEST_ASSERT_EQ_U32((msgid >> MSGID_READ_OR_WRITE_SHIFT) & 0x1,
			   MSGID_WRITE);
}

TEST(write_cmd_encodes_msg_length_as_8_bytes)
{
	vaddr_t base = test_rsc_base();
	uint32_t msgid = 0;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_write_cmd(RSC_DRV_SECURE, 0, 0,
					      0x00020010, 0x1234, true),
			   HAL_STATUS_SUCCESS);

	msgid = mock_regs_read32(cmd_msgid_addr(base, 0, 0));

	/*
	 * Regression guard: MSG_LENGTH field must encode 8 (bytes per
	 * RPMh command word), not 1. The bug this framework was built to
	 * catch encoded a bare command count instead of a byte length.
	 */
	TEST_ASSERT_EQ_U32(msgid & 0xFF, 8);
}

TEST(write_cmd_sets_res_req_when_completion_requested)
{
	vaddr_t base = test_rsc_base();
	uint32_t msgid = 0;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_write_cmd(RSC_DRV_SECURE, 0, 0,
					      0x00020010, 0x1234, true),
			   HAL_STATUS_SUCCESS);
	msgid = mock_regs_read32(cmd_msgid_addr(base, 0, 0));
	TEST_ASSERT_EQ_U32((msgid >> MSGID_RES_REQ_SHIFT) & 0x1, 1);

	TEST_ASSERT_EQ_INT(hal_rpmh_write_cmd(RSC_DRV_SECURE, 0, 1,
					      0x00020010, 0x1234, false),
			   HAL_STATUS_SUCCESS);
	msgid = mock_regs_read32(cmd_msgid_addr(base, 0, 1));
	TEST_ASSERT_EQ_U32((msgid >> MSGID_RES_REQ_SHIFT) & 0x1, 0);
}

TEST(write_cmd_encodes_addr_slave_id_and_offset)
{
	vaddr_t base = test_rsc_base();
	uint32_t addr_reg = 0;
	/* slave_id = 3, offset = 0x0040 */
	uint32_t addr = (3u << 16) | 0x0040;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_write_cmd(RSC_DRV_SECURE, 0, 0,
					      addr, 0xAABBCCDD, true),
			   HAL_STATUS_SUCCESS);

	addr_reg = mock_regs_read32(cmd_addr_addr(base, 0, 0));

	TEST_ASSERT_EQ_U32((addr_reg >> ADDR_SLV_ID_SHIFT) & 0x7, 3);
	TEST_ASSERT_EQ_U32((addr_reg >> ADDR_OFFSET_SHIFT) & 0xFFFF, 0x0040);
}

TEST(write_cmd_writes_data_verbatim)
{
	vaddr_t base = test_rsc_base();
	uint32_t data_reg = 0;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_write_cmd(RSC_DRV_SECURE, 0, 0,
					      0x00020010, 0xDEADBEEF, true),
			   HAL_STATUS_SUCCESS);

	data_reg = mock_regs_read32(cmd_data_addr(base, 0, 0));
	TEST_ASSERT_EQ_U32(data_reg, 0xDEADBEEF);
}

TEST(write_cmd_rejects_unregistered_drv)
{
	TEST_ASSERT_EQ_INT(hal_rpmh_init(test_rsc_base()), HAL_STATUS_SUCCESS);

	/* RSC_DRV_HLOS was never registered via hal_rpmh_register_drv(). */
	TEST_ASSERT_EQ_INT(hal_rpmh_write_cmd(RSC_DRV_HLOS, 0, 0,
					      0x00020010, 0x1234, true),
			   HAL_STATUS_INVALID_PARAM);
}

TEST(send_tcs_triggers_amc_mode)
{
	vaddr_t base = test_rsc_base();
	vaddr_t tcs_base = base + TCS_BASE_OFFSET;
	uint32_t control = 0;
	uint32_t enable = 0;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_send_tcs(RSC_DRV_SECURE, 0, 0x3),
			   HAL_STATUS_SUCCESS);

	enable = mock_regs_read32(tcs_base + TCS_CMD_ENABLE_OFFSET);
	TEST_ASSERT_EQ_U32(enable, 0x3);

	control = mock_regs_read32(tcs_base + TCS_CONTROL_OFFSET);
	TEST_ASSERT_MSG((control & TCS_CONTROL_AMC_MODE_EN) != 0,
			"AMC_MODE_EN not set after send_tcs");
	TEST_ASSERT_MSG((control & TCS_CONTROL_AMC_MODE_TRIGGER) != 0,
			"AMC_MODE_TRIGGER not set after send_tcs");
}

TEST(convert_to_amc_and_back_toggles_amc_mode_en)
{
	vaddr_t base = test_rsc_base();
	vaddr_t tcs_base = base + TCS_BASE_OFFSET;
	uint32_t control = 0;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_convert_to_amc(RSC_DRV_SECURE, 0),
			   HAL_STATUS_SUCCESS);
	control = mock_regs_read32(tcs_base + TCS_CONTROL_OFFSET);
	TEST_ASSERT_MSG((control & TCS_CONTROL_AMC_MODE_EN) != 0,
			"AMC_MODE_EN not set by convert_to_amc");

	TEST_ASSERT_EQ_INT(hal_rpmh_convert_to_tcs(RSC_DRV_SECURE, 0),
			   HAL_STATUS_SUCCESS);
	control = mock_regs_read32(tcs_base + TCS_CONTROL_OFFSET);
	TEST_ASSERT_MSG((control & TCS_CONTROL_AMC_MODE_EN) == 0,
			"AMC_MODE_EN still set after convert_to_tcs");
}

TEST(is_tcs_idle_reflects_status_register)
{
	vaddr_t base = test_rsc_base();
	vaddr_t tcs_base = base + TCS_BASE_OFFSET;
	bool idle = false;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	mock_regs_write32(tcs_base + TCS_STATUS_OFFSET, 0);
	TEST_ASSERT_EQ_INT(hal_rpmh_is_tcs_idle(RSC_DRV_SECURE, 0, &idle),
			   HAL_STATUS_SUCCESS);
	TEST_ASSERT_MSG(!idle, "expected not idle when status bit clear");

	mock_regs_write32(tcs_base + TCS_STATUS_OFFSET,
			  TCS_STATUS_CONTROLLER_IDLE);
	TEST_ASSERT_EQ_INT(hal_rpmh_is_tcs_idle(RSC_DRV_SECURE, 0, &idle),
			   HAL_STATUS_SUCCESS);
	TEST_ASSERT_MSG(idle, "expected idle when status bit set");
}

TEST(amc_status_enable_clear_and_get)
{
	vaddr_t base = test_rsc_base();
	bool finished = false;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_enable_amc_status(RSC_DRV_SECURE, 2),
			   HAL_STATUS_SUCCESS);

	mock_regs_inject_read32(base + RSC_DRV_IRQ_STATUS, 1u << 2);
	TEST_ASSERT_EQ_INT(hal_rpmh_get_amc_status(RSC_DRV_SECURE, 2,
						   &finished),
			   HAL_STATUS_SUCCESS);
	TEST_ASSERT_MSG(finished, "expected tcs 2 finished per IRQ_STATUS");

	TEST_ASSERT_EQ_INT(hal_rpmh_clear_amc_status(RSC_DRV_SECURE, 2),
			   HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_U32(mock_regs_read32(base + RSC_DRV_IRQ_CLEAR),
			   1u << 2);
}

TEST(read_config_splits_tcs_and_cmds_fields)
{
	vaddr_t base = test_rsc_base();
	uint32_t tcs = 0;
	uint32_t cmds = 0;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);

	mock_regs_write32(base + RSC_DRV_TCS_CONFIG, (6u << 8) | 3u);

	TEST_ASSERT_EQ_INT(hal_rpmh_read_config(RSC_DRV_SECURE, &tcs, &cmds),
			   HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_U32(tcs, 3);
	TEST_ASSERT_EQ_U32(cmds, 6);
}

TEST(epcb_timeout_update_and_toggle_only_for_secure_drv)
{
	vaddr_t base = test_rsc_base();
	uint32_t val = 0;

	TEST_ASSERT_EQ_INT(hal_rpmh_init(base), HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_SECURE),
			   HAL_STATUS_SUCCESS);
	TEST_ASSERT_EQ_INT(hal_rpmh_register_drv(RSC_DRV_HLOS),
			   HAL_STATUS_SUCCESS);

	TEST_ASSERT_EQ_INT(hal_rpmh_update_epcb_timeout(RSC_DRV_HLOS, 0x100),
			   HAL_STATUS_INVALID_PARAM);
	TEST_ASSERT_EQ_INT(hal_rpmh_toggle_epcb_timeout(RSC_DRV_HLOS, true),
			   HAL_STATUS_INVALID_PARAM);

	TEST_ASSERT_EQ_INT(hal_rpmh_update_epcb_timeout(RSC_DRV_SECURE, 0x1234),
			   HAL_STATUS_SUCCESS);
	val = mock_regs_read32(base + RSC_DRV_ERROR_IRQ_ENABLE);
	TEST_ASSERT_EQ_U32(val & EPCB_TIMEOUT_THRESHOLD_MASK, 0x1234);

	TEST_ASSERT_EQ_INT(hal_rpmh_toggle_epcb_timeout(RSC_DRV_SECURE, true),
			   HAL_STATUS_SUCCESS);
	val = mock_regs_read32(base + RSC_DRV_ERROR_IRQ_ENABLE);
	TEST_ASSERT_MSG((val & EPCB_TIMEOUT_IRQ_EN_MASK) != 0,
			"EPCB_TIMEOUT_IRQ_EN_MASK not set after toggle(true)");

	TEST_ASSERT_EQ_INT(hal_rpmh_toggle_epcb_timeout(RSC_DRV_SECURE, false),
			   HAL_STATUS_SUCCESS);
	val = mock_regs_read32(base + RSC_DRV_ERROR_IRQ_ENABLE);
	TEST_ASSERT_MSG((val & EPCB_TIMEOUT_IRQ_EN_MASK) == 0,
			"EPCB_TIMEOUT_IRQ_EN_MASK still set after toggle(false)");
}

const struct harness_test rpmh_hal_tests[] = {
	TEST_CASE(write_cmd_encodes_write_not_read),
	TEST_CASE(write_cmd_encodes_msg_length_as_8_bytes),
	TEST_CASE(write_cmd_sets_res_req_when_completion_requested),
	TEST_CASE(write_cmd_encodes_addr_slave_id_and_offset),
	TEST_CASE(write_cmd_writes_data_verbatim),
	TEST_CASE(write_cmd_rejects_unregistered_drv),
	TEST_CASE(send_tcs_triggers_amc_mode),
	TEST_CASE(convert_to_amc_and_back_toggles_amc_mode_en),
	TEST_CASE(is_tcs_idle_reflects_status_register),
	TEST_CASE(amc_status_enable_clear_and_get),
	TEST_CASE(read_config_splits_tcs_and_cmds_fields),
	TEST_CASE(epcb_timeout_update_and_toggle_only_for_secure_drv),
};

const int rpmh_hal_tests_count =
	sizeof(rpmh_hal_tests) / sizeof(rpmh_hal_tests[0]);
