// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <inttypes.h>
#include <io.h>
#include <mm/core_memprot.h>
#include <stdbool.h>
#include <stdint.h>
#include <tee_api_types.h>
#include <trace.h>
#include <util.h>

#include "cvp.h"

/* Wrapper (TSW) registers, offsets from the CVP block base */
#define CVP_XTSS_SW_RESET		(CVP_WRAPPER_TSW_REG_BASE + 0x1000)
#define CVP_XTSS_SW_RESET_BIT		BIT(0)
#define CVP_SEC_CPA_START_ADDR		(CVP_WRAPPER_TSW_REG_BASE + 0x1020)
#define CVP_SEC_CPA_END_ADDR		(CVP_WRAPPER_TSW_REG_BASE + 0x1024)
#define CVP_SEC_FW_START_ADDR		(CVP_WRAPPER_TSW_REG_BASE + 0x1028)
#define CVP_SEC_FW_END_ADDR		(CVP_WRAPPER_TSW_REG_BASE + 0x102c)
#define CVP_SEC_NONPIX_START_ADDR	(CVP_WRAPPER_TSW_REG_BASE + 0x1030)
#define CVP_SEC_NONPIX_END_ADDR		(CVP_WRAPPER_TSW_REG_BASE + 0x1034)
#define CVP_SEC_VM1_START_ADDR		(CVP_WRAPPER_TSW_REG_BASE + 0x1038)
#define CVP_SEC_VM1_END_ADDR		(CVP_WRAPPER_TSW_REG_BASE + 0x103c)

/* TOP_TSW0 SID registers, offsets from CVP_TOP_TSW0_REG_BASE */
#define CVP_TOP_FDU_SID_VM0		0x0100
#define CVP_TOP_MPU_SID_VM0		0x0108
#define CVP_TOP_PPU_SID_VM0		0x0110

/*
 * The FW/CPA window registers are in the firmware's virtual (IOVA) space,
 * not physical: the firmware window is [0, 16MB) and the kernel maps the
 * image at IOVA 0.
 */
#define CVP_FW_IOVA_START		0x00000000
#define CVP_FW_IOVA_END			0x01000000
#define CVP_NONPIX_START		0x00900000
#define CVP_NONPIX_END			0x00a00000
#define CVP_VM1_DISABLED		0xffe00000
#define CVP_FDU_SID_EVA			0x00034300
#define CVP_FDU_SID_CAMERA		0x00074700
#define CVP_MPU_SID_DEFAULT		0x00004300
#define CVP_PPU_SID_DEFAULT		0x00034300

/* Remote states requested through the PAS set_remote_state command */
#define CVP_STATE_SUSPEND		0
#define CVP_STATE_RESUME		1
#define CVP_STATE_RESTORE_THRESHOLD	2
#define CVP_STATE_SID_EVA		0x000cede1
#define CVP_STATE_SID_CAMERA		0x000cede2

static bool cvp_resumed;

static TEE_Result cvp_get_base(struct qcom_pas_data *data, vaddr_t *base)
{
	*base = io_pa_or_va(&data->base, data->size);
	if (!*base)
		return TEE_ERROR_GENERIC;

	return TEE_SUCCESS;
}

static void cvp_program_regions(vaddr_t base)
{
	vaddr_t top = base + CVP_TOP_TSW0_REG_BASE;

	io_write32(base + CVP_SEC_FW_START_ADDR, CVP_FW_IOVA_START);
	io_write32(base + CVP_SEC_FW_END_ADDR, CVP_FW_IOVA_END);
	io_write32(base + CVP_SEC_CPA_START_ADDR, CVP_FW_IOVA_START);
	io_write32(base + CVP_SEC_CPA_END_ADDR, CVP_FW_IOVA_END);
	io_write32(base + CVP_SEC_NONPIX_START_ADDR, CVP_NONPIX_START);
	io_write32(base + CVP_SEC_NONPIX_END_ADDR, CVP_NONPIX_END);
	io_write32(base + CVP_SEC_VM1_START_ADDR, CVP_VM1_DISABLED);
	io_write32(base + CVP_SEC_VM1_END_ADDR, CVP_VM1_DISABLED);

	io_write32(top + CVP_TOP_FDU_SID_VM0, CVP_FDU_SID_CAMERA);
	io_write32(top + CVP_TOP_MPU_SID_VM0, CVP_MPU_SID_DEFAULT);
	io_write32(top + CVP_TOP_PPU_SID_VM0, CVP_PPU_SID_DEFAULT);
}

static TEE_Result cvp_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = 0;
	TEE_Result res = TEE_SUCCESS;

	if (!data->fw_base || !data->fw_size) {
		EMSG("cvp: FW not loaded");
		return TEE_ERROR_NO_DATA;
	}

	res = cvp_get_base(data, &base);
	if (res)
		return res;

	if (!(io_read32(base + CVP_XTSS_SW_RESET) & CVP_XTSS_SW_RESET_BIT)) {
		DMSG("cvp: subsystem already running, skipping bring-up");
		cvp_resumed = true;
		return TEE_SUCCESS;
	}

	cvp_program_regions(base);
	io_write32(base + CVP_XTSS_SW_RESET, 0);
	cvp_resumed = true;

	return TEE_SUCCESS;
}

static TEE_Result cvp_fw_shutdown(struct qcom_pas_data *data)
{
	vaddr_t base = 0;
	TEE_Result res = cvp_get_base(data, &base);

	if (res)
		return res;

	if (cvp_resumed)
		io_setbits32(base + CVP_XTSS_SW_RESET, CVP_XTSS_SW_RESET_BIT);

	cvp_resumed = false;

	return TEE_SUCCESS;
}

static TEE_Result cvp_set_fdu_sid(struct qcom_pas_data *data, uint32_t sid)
{
	vaddr_t base = 0;
	TEE_Result res = cvp_get_base(data, &base);

	if (res)
		return res;

	io_write32(base + CVP_TOP_TSW0_REG_BASE + CVP_TOP_FDU_SID_VM0, sid);

	return TEE_SUCCESS;
}

static TEE_Result cvp_fw_set_state(struct qcom_pas_data *data, uint32_t state)
{
	switch (state) {
	case CVP_STATE_SUSPEND:
		return cvp_fw_shutdown(data);
	case CVP_STATE_RESUME:
		return cvp_fw_start(data);
	case CVP_STATE_RESTORE_THRESHOLD:
		/* No threshold registers are defined in the CVP HWIO map */
		return TEE_SUCCESS;
	case CVP_STATE_SID_EVA:
		return cvp_set_fdu_sid(data, CVP_FDU_SID_EVA);
	case CVP_STATE_SID_CAMERA:
		return cvp_set_fdu_sid(data, CVP_FDU_SID_CAMERA);
	default:
		EMSG("cvp: unsupported remote state %#"PRIx32, state);
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

const struct qcom_pas_ops cvp_ops = {
	.fw_start = cvp_fw_start,
	.fw_shutdown = cvp_fw_shutdown,
	.fw_set_state_raw = cvp_fw_set_state,
};
