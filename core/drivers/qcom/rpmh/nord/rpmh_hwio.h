/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __RPMH_HWIO_H__
#define __RPMH_HWIO_H__

#include <util.h>

/*
 * TODO: pending AoP confirmation for Nord IQ10 (see to_ask.md #2.5 and
 * policy_decisions.md). Offsets below are copied from hoya and are NOT
 * verified against Nord's RPMH IP revision - do not trust for real hardware
 * until cross-checked against the Nord HPG / downstream driver source.
 * This file is not currently compiled - CFG_QCOM_RPMH_CLIENT is not set for
 * wildcat/nord.
 */

#define DRV_STRIDE			0x10000
#define RSC_DRV_IRQ_ENABLE		0x0d00
#define RSC_DRV_IRQ_STATUS		0x0d04
#define RSC_DRV_IRQ_CLEAR		0x0d08

#define RSC_DRV_TCS_CONFIG		0x0C
#define TCS_BASE_OFFSET			0x0D10  /* CMD_WAIT_FOR_CMPL base */
#define TCS_STRIDE			0x2A0

#define TCS_CONTROL_OFFSET		0x04
#define TCS_STATUS_OFFSET		0x08
#define TCS_CMD_ENABLE_OFFSET		0x0C

#define TCS_CMD_BASE_OFFSET		0x20
#define TCS_CMDn_MSGID_OFFSET		0x00
#define TCS_CMDn_ADDR_OFFSET		0x04
#define TCS_CMDn_DATA_OFFSET		0x08
#define TCS_CMD_STRIDE			0x14

#define TCS_CONTROL_AMC_MODE_TRIGGER	BIT(24)
#define TCS_CONTROL_AMC_MODE_EN		BIT(16)

#define TCS_STATUS_CONTROLLER_IDLE	BIT(0)

#define RSC_DRV_ERROR_IRQ_STATUS	0xD0
#define RSC_DRV_ERROR_IRQ_ENABLE	0xD8
#define RSC_DRV_ERROR_IRQ_CLEAR		0xD4

#define EPCB_TIMEOUT_IRQ_EN_MASK	BIT(20)
#define EPCB_TIMEOUT_THRESHOLD_SHIFT	0x0
#define EPCB_TIMEOUT_THRESHOLD_MASK	0xFFFF

#define MSGID_READ_OR_WRITE_SHIFT	0x10
#define MSGID_RES_REQ_SHIFT		0x8
#define MSGID_MSG_LENGTH_SHIFT		0x0

#define MSGID_READ			0x0
#define MSGID_WRITE			0x1

/* Each RPMh command transfers a single 32-bit word (encoded as 8 bytes). */
#define MSGID_MSG_LENGTH_VALUE		0x8

#define ADDR_SLV_ID_SHIFT		0x10
#define ADDR_OFFSET_SHIFT		0x0

#endif /* __RPMH_HWIO_H__ */
