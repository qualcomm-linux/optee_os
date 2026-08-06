/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CLK_QCOM_VREG_H_
#define _CLK_QCOM_VREG_H_

#include <stdint.h>
#include <tee_api_types.h>

/*
 * Move the aggregate QUP rail vote from corner @old_corner to @new_corner
 * (raw rail_voltage_level; 0 means "no vote"). The walker calls this around a
 * rate change so a rate is never programmed under-volted.
 */
TEE_Result qcom_clk_vreg_vote(uint16_t old_corner, uint16_t new_corner);

#endif /* _CLK_QCOM_VREG_H_ */
