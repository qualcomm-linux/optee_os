// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * PAS clock sequencing for this chip has not been ported: the reset/enable
 * register layout has not been verified against the kodiak/lemans clock
 * driver it would otherwise be modelled on. All PAS subsystems on this
 * platform currently use QCOM_PAS_RESET_NONE (see platform/nord/subsys.c),
 * so none of these are reachable yet; they exist to satisfy the link.
 */

#include <drivers/clk_qcom.h>

TEE_Result qcom_clock_enable_pas(enum qcom_clk_group group __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

TEE_Result qcom_clock_enable_pas_processor(enum qcom_clk_group group __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

TEE_Result qcom_clock_pas_reset(enum qcom_clk_group group __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}
