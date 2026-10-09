/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __IPCC_REGS_H__
#define __IPCC_REGS_H__

#include <util.h>

/* Per-target addresses, strides, signal counts and interrupt numbers */
#include <target_config.h>

/*
 * Register offsets and field encodings of the IPC_CORE, IPC_CONFIG and
 * IPC_TRACE blocks. They are identical on every part that has an IPCC, so
 * this half is chipset independent and every chipset shares it; only the
 * values it is applied to differ, and those come from target_config.h.
 *
 * An IPC_CORE register sits at
 *	IPCC_BASE + IPCC_PROTO_STRIDE * protocol_id +
 *	IPCC_CLIENT_STRIDE * client_page + offset
 * and every client page has the full set: a client reads its own page to
 * receive and writes its own page to send.
 */

#define IPCC_VERSION_OFF			0x0
#define IPCC_VERSION_MASK			0xffffffU
#define IPCC_VERSION_MAJOR_SHIFT		16
#define IPCC_VERSION_MINOR_SHIFT		8
#define IPCC_VERSION(maj, min) \
	(SHIFT_U32(maj, IPCC_VERSION_MAJOR_SHIFT) | \
	 SHIFT_U32(min, IPCC_VERSION_MINOR_SHIFT))

/* Reads back the (CLIENT_ID, PROTOCOL_ID) the page belongs to */
#define IPCC_ID_OFF				0x4

#define IPCC_CONFIG_OFF				0x8
#define IPCC_CONFIG_CLEAR_ON_RECV_RD		BIT32(0)
#define IPCC_CONFIG_CLIENT_DISABLE		BIT32(31)

/* SEND is write-only: SIGNAL_ID 15:0, CLIENT_ID 30:16, BROADCAST bit 31 */
#define IPCC_SEND_OFF				0xC
#define IPCC_SEND_SIGNAL_ID_SHIFT		0
#define IPCC_SEND_SIGNAL_ID_MASK		0xffffU
#define IPCC_SEND_CLIENT_ID_SHIFT		16
#define IPCC_SEND_CLIENT_ID_MASK		0x7fffU
#define IPCC_SEND_BROADCAST			BIT32(31)

/*
 * RECV_ID reports the pending signal as (CLIENT_ID, SIGNAL_ID) and reads back
 * IPCC_NO_DATA once drained. From v2.0 RECV_ID_PHYS reports the sender as a
 * physical client page index instead of a client ID.
 */
#define IPCC_RECV_ID_OFF			0x10
#define IPCC_RECV_ID_PHYS_OFF			0x48
#define IPCC_RECV_ID_SIGNAL_ID_SHIFT		0
#define IPCC_RECV_ID_SIGNAL_ID_MASK		0xffffU
#define IPCC_RECV_ID_CLIENT_ID_SHIFT		16
#define IPCC_RECV_ID_CLIENT_ID_MASK		0xffffU

/* Value read back from RECV_ID / RECV_ID_PHYS when no signal is pending */
#define IPCC_NO_DATA				0xffffffffU

#define IPCC_RECV_SIGNAL_ENABLE_OFF		0x14
#define IPCC_RECV_SIGNAL_DISABLE_OFF		0x18
#define IPCC_RECV_SIGNAL_CLEAR_OFF		0x1C
#define IPCC_RECV_SIGNAL_ID_SHIFT		0
#define IPCC_RECV_SIGNAL_ID_MASK		0xffffU
#define IPCC_RECV_CLIENT_ID_SHIFT		16
#define IPCC_RECV_CLIENT_ID_MASK		0xffffU

/* Drops every signal pending on the page */
#define IPCC_CLIENT_CLEAR_OFF			0x38
#define IPCC_CLIENT_CLEAR			BIT32(0)

/*
 * IPC_CONFIG.TOP_MODE selects the mechanism the controller runs for the whole
 * block, 0 = direct and 1 = router. It and IPC_TRACE.ENABLE sit at a block
 * offset from the controller base rather than in a client page, are one
 * register each, and are programmed once.
 */
#define IPCC_TOP_MODE_BIT			BIT32(0)
#define IPCC_TRACE_ENABLE_BIT			BIT32(0)

#endif /* __IPCC_REGS_H__ */
