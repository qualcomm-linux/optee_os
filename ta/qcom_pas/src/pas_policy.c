// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <pas_policy.h>
#include <tee_api_types.h>
#include <util.h>

/*
 * PAS remote-processor identifiers (the pas_id passed by the REE). These are
 * the architectural Qualcomm PIL processor IDs.
 */
#define PAS_ID_QDSP6		1	/* ADSP / LPASS */
#define PAS_ID_WPSS		6	/* WLAN/BT */
#define PAS_ID_VENUS		9	/* video: VENUS/IRIS */
#define PAS_ID_TURING		18	/* CDSP / CDSP0 */
#define PAS_ID_TURING1		30	/* CDSP1 */
#define PAS_ID_GPDSP0		39
#define PAS_ID_GPDSP1		40

/*
 * Image software types (SW_ID). The image's signed metadata carries one of
 * these and it must equal the value bound to the peripheral being brought
 * up.
 */
#define SECBOOT_ADSP_SW_TYPE	0x04
#define SECBOOT_WCNSS_SW_TYPE	0x0D
#define SECBOOT_VIDEO_SW_TYPE	0x0E
#define SECBOOT_CDSP_SW_TYPE	0x17
#define SECBOOT_CDSP1_SW_TYPE	0x44
#define SECBOOT_GPDSP0_SW_TYPE	0x58
#define SECBOOT_GPDSP1_SW_TYPE	0x5A

/* pas_id -> expected SW_ID for these targets. */
static const struct {
	uint32_t pas_id;
	uint32_t swid;
} pas_swid_map[] = {
	{ PAS_ID_QDSP6, SECBOOT_ADSP_SW_TYPE },
	{ PAS_ID_WPSS, SECBOOT_WCNSS_SW_TYPE },
	{ PAS_ID_VENUS, SECBOOT_VIDEO_SW_TYPE },
	{ PAS_ID_TURING, SECBOOT_CDSP_SW_TYPE },
	{ PAS_ID_TURING1, SECBOOT_CDSP1_SW_TYPE },
	{ PAS_ID_GPDSP0, SECBOOT_GPDSP0_SW_TYPE },
	{ PAS_ID_GPDSP1, SECBOOT_GPDSP1_SW_TYPE },
};

TEE_Result pas_policy_expected_swid(uint32_t pas_id, uint32_t *swid)
{
	size_t i = 0;

	if (!swid)
		return TEE_ERROR_BAD_PARAMETERS;

	for (i = 0; i < ARRAY_SIZE(pas_swid_map); i++) {
		if (pas_swid_map[i].pas_id == pas_id) {
			*swid = pas_swid_map[i].swid;
			return TEE_SUCCESS;
		}
	}

	return TEE_ERROR_NOT_SUPPORTED;
}

enum pas_sign_authority pas_policy_signer(uint32_t swid __unused)
{
	/* Every SW_ID this port supports is OEM-signed only. */
	return PAS_OEM_SIGNED;
}
