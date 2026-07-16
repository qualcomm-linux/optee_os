/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __QCOM_PAS_PRIV_H
#define __QCOM_PAS_PRIV_H

#include <pas_mbn_parser.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Per-session context shared between the command dispatch (qcom_pas.c) and
 * the authentication backend (pas_auth.c).
 *
 * The firmware metadata is saved at INIT_IMAGE inside a TEE-private copy so
 * the REE cannot alter it between INIT_IMAGE and AUTH_AND_RESET. Keeping it
 * per-session (rather than as a TA global) ensures two concurrent sessions
 * cannot observe each other's metadata.
 *
 * The remoteproc driver opens one TEE session shared by every peripheral,
 * and DSPs load concurrently. Metadata must therefore be keyed by pas_id to
 * prevent one DSP's INIT_IMAGE overwriting another's slot before its
 * AUTH_AND_RESET runs. The table is sized per platform to cover the number
 * of PAS subsystems a target exposes: a target overrides CFG_PAS_MD_SLOTS
 * to size it up, and it defaults to a single slot otherwise.
 */
#ifndef CFG_PAS_MD_SLOTS
#define CFG_PAS_MD_SLOTS	1U
#endif
#define PAS_MD_SLOTS		CFG_PAS_MD_SLOTS

struct pas_md_slot {
	void *meta_data;
	size_t meta_data_size;
	uint32_t pas_id;
	bool used;
	struct pas_mbn mbn;
	/*
	 * True once pas_mbn_parse() has located a table for this slot and,
	 * on secure-boot devices, every signature-authentication check has
	 * passed. Gates AUTH_AND_RESET.
	 */
	bool hash_table_valid;
};

struct qcom_pas_session {
	struct pas_md_slot slots[PAS_MD_SLOTS];
};

#endif /* __QCOM_PAS_PRIV_H */
