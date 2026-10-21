/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __DRIVERS_QCOM_TME_ARB_H
#define __DRIVERS_QCOM_TME_ARB_H

#include <stdbool.h>
#include <stdint.h>
#include <tee_api_types.h>

/*
 * Get the OEM anti-rollback (ARB) version recorded by TME for @sw_id.
 *
 * @valid is set to false when TME has no OEM ARB version for @sw_id (for
 * instance when ARB is not enabled for the image); @version is then not
 * meaningful.
 */
TEE_Result tme_get_arb_oem_version(uint32_t sw_id, uint32_t *version,
				   bool *valid);

/*
 * Ask TME to commit the recorded image versions into the anti-rollback
 * fuses.
 *
 * This blows one-time-programmable fuses: once raised, the device refuses to
 * boot an older-versioned image. There is no undo and no dry run.
 */
TEE_Result tme_update_rollback_version(void);

#endif /* __DRIVERS_QCOM_TME_ARB_H */
