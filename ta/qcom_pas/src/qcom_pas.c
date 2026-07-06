// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <config.h>
#include <pas_auth.h>
#include <pas_hashseg.h>
#include <pas_policy.h>
#include <pta_qcom_fuse.h>
#include <pta_qcom_pas.h>
#include <string.h>
#include <string_ext.h>
#include <ta_qcom_pas.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <types_ext.h>
#include <utee_defines.h>
#include <util.h>

/*
 * Per-session context. The firmware metadata is saved at INIT_IMAGE inside a
 * TEE-private copy so the REE cannot alter it between INIT_IMAGE and
 * AUTH_AND_RESET. Keeping it here (rather than as a TA global) ensures two
 * concurrent sessions cannot observe each other's metadata.
 *
 * The kernel qcom_pas_tee driver opens one TEE session shared by every
 * peripheral, and DSPs load concurrently. Metadata must therefore be keyed
 * by pas_id to prevent one DSP's INIT_IMAGE overwriting another's slot
 * before its AUTH_AND_RESET runs. lemans/kodiak expose at most 7 PAS
 * subsystems; size the table with headroom.
 */
#define PAS_MD_SLOTS	8U

struct pas_md_slot {
	void *md;
	size_t md_size;
	uint32_t pas_id;
	bool used;
	struct pas_hashseg hs;
	bool authenticated;
};

struct qcom_pas_session {
	struct pas_md_slot md[PAS_MD_SLOTS];
};

static struct pas_md_slot *find_md_slot(struct qcom_pas_session *s,
					uint32_t pas_id)
{
	size_t i = 0;

	for (i = 0; i < PAS_MD_SLOTS; i++) {
		if (s->md[i].used && s->md[i].pas_id == pas_id)
			return &s->md[i];
	}

	return NULL;
}

/*
 * The PTA session is shared across all TA sessions: it wraps the core-side
 * PAS driver and does not carry per-image state. refcount tracks when to open
 * and close it.
 */
static size_t session_refcount;
static TEE_TASessionHandle pta_session;

/*
 * Save a private copy of the INIT_IMAGE metadata so it cannot be modified
 * by the REE before AUTH_AND_RESET. Keyed by pas_id so concurrent loads of
 * different peripherals on the shared session do not clobber each other.
 */
static TEE_Result save_metadata(struct qcom_pas_session *s, uint32_t pt,
				TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	struct pas_md_slot *slot = NULL;
	uint32_t pas_id = 0;
	void *copy = NULL;
	size_t size = 0;
	size_t i = 0;

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	pas_id = params[0].value.a;

	/* Reuse the existing slot for this pas_id, else take a free one. */
	slot = find_md_slot(s, pas_id);
	if (!slot) {
		for (i = 0; i < PAS_MD_SLOTS; i++) {
			if (!s->md[i].used) {
				slot = &s->md[i];
				break;
			}
		}
	}
	if (!slot) {
		EMSG("PAS auth: no free md slot (pas_id=%"PRIu32")", pas_id);
		return TEE_ERROR_OUT_OF_MEMORY;
	}

	size = params[1].memref.size;
	if (size) {
		copy = TEE_Malloc(size, TEE_MALLOC_FILL_ZERO);
		if (!copy)
			return TEE_ERROR_OUT_OF_MEMORY;
		memcpy(copy, params[1].memref.buffer, size);
	}

	TEE_Free(slot->md);
	slot->md = copy;
	slot->md_size = size;
	slot->pas_id = pas_id;
	slot->used = true;
	slot->authenticated = false;
	memset(&slot->hs, 0, sizeof(slot->hs));

	return TEE_SUCCESS;
}

#ifdef CFG_QCOM_PAS_SECURE_BOOT
/*
 * SHA-384 digest of the QTI QSEE production root certificate (DER-encoded),
 * taken verbatim from the platform's SHA-384 root-of-trust table. Used to
 * bind the QTI cert chain to the device-side anchor for double-signed
 * images.
 */
static const uint8_t qti_root_of_trust[TEE_SHA384_HASH_SIZE] = {
	0x46, 0x7f, 0x30, 0x20, 0xc4, 0xcc, 0x78, 0x8d,
	0x2a, 0x27, 0xa6, 0xe0, 0x97, 0xff, 0xa7, 0xbf,
	0xc2, 0x4e, 0x82, 0xc2, 0xd5, 0x69, 0x53, 0xd3,
	0xb4, 0x5b, 0x49, 0x4c, 0xdb, 0xa5, 0xc2, 0x42,
	0x2a, 0x94, 0x7d, 0x2c, 0x81, 0xb5, 0x75, 0x2b,
	0x8a, 0x2a, 0xc9, 0xea, 0xc8, 0xac, 0xdf, 0x34,
};

static TEE_Result get_root_anchor(uint8_t *anchor, bool *secboot_on)
{
	static const TEE_UUID fuse_uuid = PTA_QCOM_FUSE_UUID;
	TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	TEE_Param params[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t pt = 0;

	*secboot_on = false;

	res = TEE_OpenTASession(&fuse_uuid, TEE_TIMEOUT_INFINITE, 0, NULL,
				&sess, NULL);
	if (res) {
		EMSG("PAS auth: cannot open fuse PTA: %#"PRIx32, res);
		return res;
	}

	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_GET_SECBOOT_STATE, pt, params,
				  NULL);
	if (res)
		goto out;
	*secboot_on = params[0].value.a != 0;

	memset(params, 0, sizeof(params));
	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	params[0].memref.buffer = anchor;
	params[0].memref.size = PTA_QCOM_FUSE_ROOT_OF_TRUST_SIZE;
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_GET_ROOT_OF_TRUST, pt, params,
				  NULL);
out:
	TEE_CloseTASession(sess);

	return res;
}

/*
 * Accepted metadata major versions: V0 (0) and V1 (1); minor must be 0.
 */
#define SECBOOT_METADATA_MAJOR_V0	0U
#define SECBOOT_METADATA_MAJOR_V1	1U
#define SECBOOT_METADATA_MINOR		0U

/*
 * Reject metadata whose major/minor version falls outside the accepted
 * set. Must be called after signature verification so the metadata is
 * authenticated.
 */
static TEE_Result check_metadata_version(const struct pas_hashseg *hs,
					 bool secboot_on)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	struct pas_meta meta = { };

	res = pas_hashseg_get_meta(hs, &meta);
	if (res == TEE_ERROR_NO_DATA)
		return TEE_SUCCESS; /* v5 / no metadata: nothing to check */
	if (res) {
		EMSG("PAS auth: bad OEM metadata in version check");
		return secboot_on ? res : TEE_SUCCESS;
	}

	if ((meta.major == SECBOOT_METADATA_MAJOR_V0 ||
	     meta.major == SECBOOT_METADATA_MAJOR_V1) &&
	    meta.minor == SECBOOT_METADATA_MINOR)
		return TEE_SUCCESS;

	EMSG("PAS auth: unsupported metadata version %"PRIu32".%"PRIu32,
	     meta.major, meta.minor);
	if (secboot_on)
		return TEE_ERROR_SECURITY;
	IMSG("PAS auth: metadata version mismatch tolerated");
	return TEE_SUCCESS;
}

/*
 * Bind the signed image to the peripheral being brought up: the metadata
 * SW_ID must match the value the platform assigns to this pas_id, so a
 * validly-signed image for one subsystem cannot be loaded onto another.
 * The reference implementation also compares the metadata secondary_sw_id,
 * but only against the image's own secondary_sw_id fed back from the same
 * metadata - a self-comparison that never constrains, so it is not
 * replicated here. The metadata lives inside the already
 * signature-verified region. Enforced when secure boot is on; logged
 * otherwise so unsigned-boot boards still come up.
 */
static TEE_Result check_sw_binding(const struct pas_hashseg *hs,
				   uint32_t pas_id, bool secboot_on)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	struct pas_meta meta = { };
	uint32_t expected = 0;

	res = pas_hashseg_get_meta(hs, &meta);
	if (res == TEE_ERROR_NO_DATA) {
		/* No OEM metadata (e.g. MBN v5): nothing to bind. */
		return TEE_SUCCESS;
	}
	if (res) {
		EMSG("PAS auth: bad OEM metadata");
		return secboot_on ? res : TEE_SUCCESS;
	}

	res = pas_policy_expected_swid(pas_id, &expected);
	if (res) {
		EMSG("PAS auth: no SW_ID binding for pas_id %"PRIu32, pas_id);
		return secboot_on ? res : TEE_SUCCESS;
	}

	if (meta.sw_id != expected) {
		EMSG("PAS auth: SW_ID got %#"PRIx32" want %#"PRIx32,
		     meta.sw_id, expected);
		if (secboot_on)
			return TEE_ERROR_SECURITY;
		IMSG("PAS auth: SW_ID mismatch tolerated");
		return TEE_SUCCESS;
	}

	DMSG("PAS auth: SW_ID %#"PRIx32" bound to pas_id %"PRIu32, meta.sw_id,
	     pas_id);

	return TEE_SUCCESS;
}

/*
 * 2-bit option field value at @shift within @flags. Valid values are 0-2
 * (disable/enable/enable-with-serial-match) for DEBUG, root revoke/activate
 * and UIE key switch; 3 is reserved and never valid.
 */
static uint32_t pas_meta_option(uint32_t flags, uint32_t shift)
{
	return (flags >> shift) & PAS_META_OPTION_MASK;
}

/*
 * Reject metadata whose 2-bit option fields carry the reserved value 3.
 */
static TEE_Result check_metadata_options(const struct pas_meta *meta,
					 bool secboot_on)
{
	if (pas_meta_option(meta->flags,
			    PAS_META_FLAG_ROOT_REVOKE_ACTIVATE_SHIFT) <=
	    PAS_META_OPTION_MAX &&
	    pas_meta_option(meta->flags, PAS_META_FLAG_UIE_KEY_SWITCH_SHIFT) <=
	    PAS_META_OPTION_MAX &&
	    pas_meta_option(meta->flags, PAS_META_FLAG_DEBUG_SHIFT) <=
	    PAS_META_OPTION_MAX)
		return TEE_SUCCESS;

	EMSG("PAS auth: reserved metadata option value, flags=%#"PRIx32,
	     meta->flags);
	if (secboot_on)
		return TEE_ERROR_SECURITY;
	IMSG("PAS auth: reserved option value tolerated");
	return TEE_SUCCESS;
}

/*
 * Device-identity fields read from the fuse PTA (PTA_QCOM_FUSE_GET_DEVICE_IDS
 * / _GET_SOC_HW_VERSION), used for the HW binding checks below. The PTA's own
 * struct qcom_secboot_device_ids lives in a core-only header not visible to
 * user TAs.
 */
struct pas_device_ids {
	uint32_t oem_id;
	uint32_t model_id;
	uint32_t jtag_id;
	uint32_t serial_num;
};

/*
 * Bind OEM_ID and MODEL_ID to the device fuses: each field is checked
 * unless its *_INDEPENDENT flag exempts it.
 */
static TEE_Result check_oem_model_binding(const struct pas_meta *meta,
					  const struct pas_device_ids *ids,
					  bool secboot_on)
{
	bool oem_independent = meta->flags &
				BIT32(PAS_META_FLAG_OEM_ID_INDEPENDENT);
	bool model_independent = meta->flags &
				 BIT32(PAS_META_FLAG_MODEL_ID_INDEPENDENT);

	if (!oem_independent && meta->oem_id != ids->oem_id) {
		EMSG("PAS auth: OEM_ID got %#"PRIx32" want %#"PRIx32,
		     meta->oem_id, ids->oem_id);
		if (secboot_on)
			return TEE_ERROR_SECURITY;
		IMSG("PAS auth: OEM_ID mismatch tolerated");
	}

	if (!model_independent && meta->model_id != ids->model_id) {
		EMSG("PAS auth: MODEL_ID got %#"PRIx32" want %#"PRIx32,
		     meta->model_id, ids->model_id);
		if (secboot_on)
			return TEE_ERROR_SECURITY;
		IMSG("PAS auth: MODEL_ID mismatch tolerated");
	}

	return TEE_SUCCESS;
}

/*
 * Bind HW_ID (JTAG authentication bits) to the device fuse; checked only
 * when IN_USE_JTAG_ID is set.
 */
static TEE_Result check_jtag_binding(const struct pas_meta *meta,
				     const struct pas_device_ids *ids,
				     bool secboot_on)
{
	if (!(meta->flags & BIT32(PAS_META_FLAG_IN_USE_JTAG_ID)))
		return TEE_SUCCESS;

	if (meta->hw_id == ids->jtag_id)
		return TEE_SUCCESS;

	EMSG("PAS auth: HW_ID got %#"PRIx32" want %#"PRIx32, meta->hw_id,
	     ids->jtag_id);
	if (secboot_on)
		return TEE_ERROR_SECURITY;
	IMSG("PAS auth: HW_ID mismatch tolerated");
	return TEE_SUCCESS;
}

/*
 * Bind the device serial number against the metadata allow-list; checked
 * when either the metadata's USE_SERIAL_NUMBER flag is set, or the APPS
 * SECURE_BOOTn USE_SERIAL_NUM fuse override forces it, and the device has
 * a nonzero fused serial.
 */
static TEE_Result check_serial_binding(const struct pas_meta *meta,
				       const struct pas_device_ids *ids,
				       bool use_serial_num_override,
				       bool secboot_on)
{
	size_t i = 0;

	if ((!(meta->flags & BIT32(PAS_META_FLAG_USE_SERIAL_NUMBER)) &&
	     !use_serial_num_override) || !ids->serial_num)
		return TEE_SUCCESS;

	for (i = 0; i < ARRAY_SIZE(meta->serial_num); i++) {
		if (meta->serial_num[i] &&
		    meta->serial_num[i] == ids->serial_num)
			return TEE_SUCCESS;
	}

	EMSG("PAS auth: serial number %#"PRIx32" not in metadata allow-list",
	     ids->serial_num);
	if (secboot_on)
		return TEE_ERROR_SECURITY;
	IMSG("PAS auth: serial number mismatch tolerated");
	return TEE_SUCCESS;
}

/*
 * Bind the SoC family|device version against the metadata allow-list;
 * checked only when IN_USE_SOC_HW_VERSION is set.
 */
static TEE_Result check_soc_vers_binding(const struct pas_meta *meta,
					 uint32_t fam_dev, bool secboot_on)
{
	size_t i = 0;

	if (!(meta->flags & BIT32(PAS_META_FLAG_IN_USE_SOC_HW_VERSION)))
		return TEE_SUCCESS;

	for (i = 0; i < ARRAY_SIZE(meta->soc_vers); i++) {
		if (meta->soc_vers[i] == fam_dev)
			return TEE_SUCCESS;
	}

	EMSG("PAS auth: SOC_HW_VERSION %#"PRIx32" not in metadata allow-list",
	     fam_dev);
	if (secboot_on)
		return TEE_ERROR_SECURITY;
	IMSG("PAS auth: SOC_HW_VERSION mismatch tolerated");
	return TEE_SUCCESS;
}

/*
 * Bind the signed image to this device's fuses and reject malformed option
 * fields (HW/OEM/MODEL/serial/SoC binding checks; UIE key-switch handling
 * excluded, out of scope for PAS peripheral images). Opens its own session
 * to the fuse PTA, matching check_anti_rollback(). Tolerated when secure
 * boot is off.
 */
static TEE_Result check_hw_binding(const struct pas_hashseg *hs,
				   bool secboot_on)
{
	static const TEE_UUID fuse_uuid = PTA_QCOM_FUSE_UUID;
	TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	TEE_Param params[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	struct pas_device_ids ids = { };
	struct pas_meta meta = { };
	bool use_serial_num_override = false;
	uint32_t fam_dev = 0;
	uint32_t pt = 0;

	res = pas_hashseg_get_meta(hs, &meta);
	if (res == TEE_ERROR_NO_DATA)
		return TEE_SUCCESS; /* v5 / no metadata: nothing to bind */
	if (res) {
		EMSG("PAS auth: bad OEM metadata in HW binding check");
		return secboot_on ? res : TEE_SUCCESS;
	}

	res = check_metadata_options(&meta, secboot_on);
	if (res)
		return res;

	res = TEE_OpenTASession(&fuse_uuid, TEE_TIMEOUT_INFINITE, 0, NULL,
				&sess, NULL);
	if (res) {
		EMSG("PAS auth: cannot open fuse PTA: %#"PRIx32, res);
		return secboot_on ? res : TEE_SUCCESS;
	}

	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
			     TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE);
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_GET_DEVICE_IDS, pt, params,
				  NULL);
	if (res) {
		EMSG("PAS auth: cannot read device ids: %#"PRIx32, res);
		TEE_CloseTASession(sess);
		return secboot_on ? res : TEE_SUCCESS;
	}
	ids.oem_id = params[0].value.a;
	ids.model_id = params[0].value.b;
	ids.jtag_id = params[1].value.a;
	ids.serial_num = params[1].value.b;

	memset(params, 0, sizeof(params));
	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_GET_USE_SERIAL_NUM, pt, params,
				  NULL);
	if (res) {
		EMSG("PAS auth: cannot read USE_SERIAL_NUM fuse: %#"PRIx32,
		     res);
		TEE_CloseTASession(sess);
		return secboot_on ? res : TEE_SUCCESS;
	}
	use_serial_num_override = params[0].value.a;

	if (meta.flags & BIT32(PAS_META_FLAG_IN_USE_SOC_HW_VERSION)) {
		memset(params, 0, sizeof(params));
		pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
				     TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
				     TEE_PARAM_TYPE_NONE);
		res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
					  PTA_QCOM_FUSE_GET_SOC_HW_VERSION, pt,
					  params, NULL);
		if (res) {
			EMSG("PAS auth: cannot read SOC_HW_VERSION: %#"PRIx32,
			     res);
			TEE_CloseTASession(sess);
			return secboot_on ? res : TEE_SUCCESS;
		}
		fam_dev = params[0].value.a;
	}

	TEE_CloseTASession(sess);

	res = check_oem_model_binding(&meta, &ids, secboot_on);
	if (res)
		return res;

	res = check_jtag_binding(&meta, &ids, secboot_on);
	if (res)
		return res;

	res = check_serial_binding(&meta, &ids, use_serial_num_override,
				   secboot_on);
	if (res)
		return res;

	res = check_soc_vers_binding(&meta, fam_dev, secboot_on);
	if (res)
		return res;

	DMSG("PAS auth: HW binding ok (oem=%#"PRIx32" model=%#"PRIx32")",
	     ids.oem_id, ids.model_id);

	return TEE_SUCCESS;
}

/*
 * Query whether Extended Key Usage enforcement is fused on, mirroring the
 * reference secboot OEM-config EKU-enforcement check. Failure to reach the
 * fuse PTA
 * is non-fatal: EKU enforcement is an additional restriction on top of
 * secure boot, not a substitute for it, so a board without an accessible
 * fuse PTA session simply does not enforce it (matching the fail-open
 * pattern used by the other fuse-backed checks in this TA).
 */
static bool eku_enforcement_enabled(void)
{
	static const TEE_UUID fuse_uuid = PTA_QCOM_FUSE_UUID;
	TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	TEE_Param params[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t pt = 0;

	res = TEE_OpenTASession(&fuse_uuid, TEE_TIMEOUT_INFINITE, 0, NULL,
				&sess, NULL);
	if (res) {
		EMSG("PAS auth: cannot open fuse PTA: %#"PRIx32, res);
		return false;
	}

	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_GET_EKU_ENFORCEMENT_EN, pt,
				  params, NULL);
	TEE_CloseTASession(sess);
	if (res) {
		EMSG("PAS auth: cannot read EKU enforcement fuse: %#"PRIx32,
		     res);
		return false;
	}

	return params[0].value.a;
}

/*
 * Multiple-root-certificate (MRC) provisioning state, read once per
 * authentication and passed to the cert-chain / root-of-trust checks.
 */
struct pas_mrc_info {
	uint32_t num_roots;
	uint32_t activation_list;
	uint32_t revocation_list;
};

/*
 * Query the device's MRC provisioning state. A fuse PTA access failure falls
 * back to the single-root default (num_roots=1): the mandatory root-of-trust
 * bind still runs regardless, so a board without an accessible fuse PTA
 * session simply does not engage root selection.
 */
static void get_mrc_info(struct pas_mrc_info *info)
{
	static const TEE_UUID fuse_uuid = PTA_QCOM_FUSE_UUID;
	TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	TEE_Param params[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t pt = 0;

	info->num_roots = 1;
	info->activation_list = 0;
	info->revocation_list = 0;

	res = TEE_OpenTASession(&fuse_uuid, TEE_TIMEOUT_INFINITE, 0, NULL,
				&sess, NULL);
	if (res) {
		EMSG("PAS auth: cannot open fuse PTA: %#"PRIx32, res);
		return;
	}

	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
			     TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE);
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_GET_MRC_INFO, pt, params, NULL);
	TEE_CloseTASession(sess);
	if (res) {
		EMSG("PAS auth: cannot read MRC info: %#"PRIx32, res);
		return;
	}

	if (!params[0].value.a)
		return;

	info->num_roots = params[0].value.b;
	info->activation_list = params[1].value.a;
	info->revocation_list = params[1].value.b;
}

/*
 * Verify the QTI countersignature on a double-signed image, mirroring the
 * reference PIL signature-verification flow. No-op when no QTI material is
 * present, unless the image's SW_ID mandates DOUBLE_SIGNED, in which case
 * absent QTI material is fatal when secure boot is on. The QTI root anchor
 * is the QSEE production root from the platform's SHA-384 root-of-trust
 * table.
 */
static TEE_Result verify_qti_countersignature(const struct pas_hashseg *hs,
					      bool secboot_on)
{
	enum pas_sign_authority auth = PAS_OEM_SIGNED;
	uint32_t rot_hash_algo = TEE_ALG_SHA384;
	TEE_Result res = TEE_ERROR_GENERIC;
	const uint8_t *qleaf = NULL;
	const uint8_t *qroot = NULL;
	struct pas_meta meta = { };
	uint8_t *qsigned = NULL;
	size_t qsigned_len = 0;
	uint32_t qsig_hash_algo = 0;
	uint32_t qsig_algo = 0;
	uint32_t qsalt_len = 0;
	size_t qleaf_len = 0;
	size_t qroot_len = 0;

	if (pas_hashseg_get_meta(hs, &meta) == TEE_SUCCESS)
		auth = pas_policy_signer(meta.sw_id);

	if (!hs->qti_certs || !hs->qti_sig) {
		if (auth != PAS_DOUBLE_SIGNED)
			return TEE_SUCCESS;
		EMSG("PAS auth: DOUBLE_SIGNED, QTI absent");
		if (secboot_on)
			return TEE_ERROR_SECURITY;
		IMSG("PAS auth: no QTI material, tolerated");
		return TEE_SUCCESS;
	}

	res = pas_auth_verify_cert_chain(hs->qti_certs, hs->qti_certs_size,
					 eku_enforcement_enabled(), 1, 0,
					 &qleaf, &qleaf_len, &qroot,
					 &qroot_len);
	if (res) {
		EMSG("PAS auth: QTI cert chain invalid");
		if (secboot_on)
			return res;
		IMSG("PAS auth: QTI cert failure tolerated");
		return TEE_SUCCESS;
	}

	res = pas_auth_check_root_of_trust(rot_hash_algo,
					   sizeof(qti_root_of_trust),
					   qroot, qroot_len, qti_root_of_trust);
	if (res) {
		if (secboot_on) {
			EMSG("PAS auth: QTI root-of-trust mismatch");
			return res;
		}
		IMSG("PAS auth: QTI ROT mismatch tolerated");
	}

	res = pas_auth_sig_algo_from_leaf(qleaf, qleaf_len, &qsig_algo,
					  &qsig_hash_algo, &qsalt_len);
	if (res) {
		EMSG("PAS auth: cannot determine QTI sig algo");
		if (secboot_on)
			return res;
		IMSG("PAS auth: QTI sig algo tolerated");
		return TEE_SUCCESS;
	}

	res = pas_hashseg_signed_copy(hs, PAS_SIGNER_QTI, &qsigned,
				      &qsigned_len);
	if (res)
		return res;

	res = pas_auth_verify_signature(qsig_algo, qsig_hash_algo, qsalt_len,
					qleaf, qleaf_len, qsigned, qsigned_len,
					hs->qti_sig, hs->qti_sig_size);
	TEE_Free(qsigned);
	if (res) {
		EMSG("PAS auth: QTI signature verify failed");
		if (secboot_on)
			return res;
		IMSG("PAS auth: QTI sig failure tolerated");
		return TEE_SUCCESS;
	}

	return TEE_SUCCESS;
}

/*
 * Validate the certificate chain, bind its root to the device ROT and verify
 * the signature. A missing or mismatched anchor is tolerated when secure boot
 * is disabled; when enabled, every step must pass.
 */
static TEE_Result verify_authenticity(const struct pas_hashseg *hs,
				      uint32_t pas_id)
{
	uint8_t anchor[PTA_QCOM_FUSE_ROOT_OF_TRUST_SIZE] = { };
	uint32_t rot_hash_algo = TEE_ALG_SHA384;
	TEE_Result res = TEE_ERROR_GENERIC;
	TEE_Result rc = TEE_ERROR_GENERIC;
	struct pas_mrc_info mrc = { };
	uint8_t *signed_copy = NULL;
	const uint8_t *roots = NULL;
	uint32_t sig_hash_algo = 0;
	uint32_t root_cert_sel = 0;
	struct pas_meta meta = { };
	const uint8_t *leaf = NULL;
	bool secboot_on = false;
	uint32_t sig_algo = 0;
	uint32_t salt_len = 0;
	size_t signed_len = 0;
	size_t roots_len = 0;
	size_t leaf_len = 0;

	rc = get_root_anchor(anchor, &secboot_on);

	if (!hs->oem_certs || !hs->oem_sig || !hs->signed_region) {
		if (secboot_on) {
			EMSG("PAS auth: metadata is not OEM-signed");
			return TEE_ERROR_SECURITY;
		}
		IMSG("PAS auth: no OEM material, tolerated (NS boot)");
		return TEE_SUCCESS;
	}

	/*
	 * root_cert_sel (metadata word 28) picks which provisioned root this
	 * chain is verified against; images without OEM metadata (MBN v5)
	 * always use root 0. Selecting an index only chooses which already
	 * fuse-bound root to try - it does not grant trust by itself.
	 */
	if (pas_hashseg_get_meta(hs, &meta) == TEE_SUCCESS)
		root_cert_sel = meta.root_cert_sel;

	get_mrc_info(&mrc);
	if (root_cert_sel >= mrc.num_roots) {
		EMSG("PAS auth: root_cert_sel %"PRIu32" >= %"PRIu32" roots",
		     root_cert_sel, mrc.num_roots);
		if (secboot_on)
			return TEE_ERROR_SECURITY;
		IMSG("PAS auth: root selection out of range tolerated");
		root_cert_sel = 0;
		mrc.num_roots = 1;
	}

	if (mrc.num_roots > 1) {
		res = pas_auth_check_root_cert_index(root_cert_sel,
						     mrc.num_roots,
						     mrc.activation_list,
						     mrc.revocation_list);
		if (res) {
			EMSG("PAS auth: root cert %"PRIu32" not usable",
			     root_cert_sel);
			if (secboot_on)
				return res;
			IMSG("PAS auth: root selection failure tolerated");
			root_cert_sel = 0;
			mrc.num_roots = 1;
		}
	}

	res = pas_auth_verify_cert_chain(hs->oem_certs, hs->oem_certs_size,
					 eku_enforcement_enabled(),
					 mrc.num_roots, root_cert_sel, &leaf,
					 &leaf_len, &roots, &roots_len);
	if (res) {
		EMSG("PAS auth: OEM cert chain invalid: %#"PRIx32, res);
		if (secboot_on)
			return res;
		IMSG("PAS auth: cert chain failure tolerated (NS boot)");
		return TEE_SUCCESS;
	}

	if (!rc) {
		/*
		 * The root-of-trust digest covers every provisioned root
		 * concatenated (a single root when selection is disabled),
		 * matching how the anchor fuse is provisioned.
		 */
		res = pas_auth_check_root_of_trust(rot_hash_algo,
						   sizeof(anchor), roots,
						   roots_len, anchor);
		if (res && secboot_on) {
			EMSG("PAS auth: root-of-trust mismatch");
			goto out;
		}
		if (res)
			IMSG("PAS auth: root-of-trust mismatch tolerated");
	} else if (secboot_on) {
		EMSG("PAS auth: root-of-trust unavailable, secure boot on");
		res = rc;
		goto out;
	} else {
		IMSG("PAS auth: root-of-trust anchor unavailable");
	}

	/* Metadata is authenticated now; bind it to this peripheral. */
	res = check_metadata_version(hs, secboot_on);
	if (res)
		goto out;

	res = check_sw_binding(hs, pas_id, secboot_on);
	if (res)
		goto out;

	res = check_hw_binding(hs, secboot_on);
	if (res)
		goto out;

	res = pas_auth_sig_algo_from_leaf(leaf, leaf_len, &sig_algo,
					  &sig_hash_algo, &salt_len);
	if (res) {
		EMSG("PAS auth: cannot determine signature algorithm: %#"PRIx32,
		     res);
		if (secboot_on)
			goto out;
		IMSG("PAS auth: sig algo failure tolerated (NS boot)");
		res = TEE_SUCCESS;
		goto out;
	}

	res = pas_hashseg_signed_copy(hs, PAS_SIGNER_OEM, &signed_copy,
				      &signed_len);
	if (res)
		goto out;

	res = pas_auth_verify_signature(sig_algo, sig_hash_algo, salt_len, leaf,
					leaf_len, signed_copy, signed_len,
					hs->oem_sig, hs->oem_sig_size);
	TEE_Free(signed_copy);
	if (res) {
		EMSG("PAS auth: OEM signature verify failed: %#"PRIx32, res);
		if (secboot_on)
			goto out;
		IMSG("PAS auth: signature failure tolerated (NS boot)");
		res = TEE_SUCCESS;
		goto out;
	}

	res = verify_qti_countersignature(hs, secboot_on);
	if (res)
		goto out;

	DMSG("PAS auth: authenticity verified");
out:
	memzero_explicit(anchor, sizeof(anchor));

	return res;
}

#endif /* CFG_QCOM_PAS_SECURE_BOOT */

#ifdef CFG_QCOM_PAS_HASH_VERIFY
static TEE_Result verify_integrity(struct qcom_pas_session *s, uint32_t pas_id,
				   const struct pas_hashseg *hs,
				   TEE_Param params[TEE_NUM_PARAMS])
{
	struct pas_md_slot *slot = find_md_slot(s, pas_id);
	TEE_Param vp[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	size_t combined_size = 0;
	uint8_t *combined = NULL;
	uint32_t pt = 0;

	if (!slot || !slot->md || !slot->md_size) {
		EMSG("PAS auth: no metadata for pas_id=%"PRIu32, pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	if (ADD_OVERFLOW(slot->md_size, hs->hash_table_size, &combined_size))
		return TEE_ERROR_OVERFLOW;

	combined = TEE_Malloc(combined_size, TEE_MALLOC_FILL_ZERO);
	if (!combined)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(combined, slot->md, slot->md_size);
	TEE_MemMove(combined + slot->md_size, hs->hash_table,
		    hs->hash_table_size);

	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
			     TEE_PARAM_TYPE_VALUE_INPUT,
			     TEE_PARAM_TYPE_MEMREF_INPUT,
			     TEE_PARAM_TYPE_VALUE_INPUT);

	vp[0].value.a = params[0].value.a;
	vp[0].value.b = params[0].value.b;
	vp[1].value.a = params[1].value.a;
	vp[1].value.b = params[1].value.b;
	vp[2].memref.buffer = combined;
	vp[2].memref.size = combined_size;
	vp[3].value.a = hs->hash_size;
	vp[3].value.b = slot->md_size; /* ht offset in combined buf */

	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_PAS_VERIFY_IMAGE, pt, vp, NULL);

	TEE_Free(combined);
	return res;
}

#ifdef CFG_QCOM_PAS_SECURE_BOOT

/*
 * Advance the device ARB fuse to @version, mirroring the reference PIL
 * rollback-version-update flow. The PTA blow is monotonic and no-ops when
 * enforcement is inactive; a failure to advance must not defeat the
 * (already passed) rollback check, so it is logged and tolerated.
 */
#ifdef CFG_QCOM_PAS_ARB
static void blow_arb_fuse(TEE_TASessionHandle sess, uint32_t version)
{
	TEE_Param params[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t pt = 0;

	params[0].value.a = version;
	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_BLOW_PIL_ROLLBACK_VERSION, pt,
				  params, NULL);
	if (res)
		IMSG("PAS ARB: fuse advance failed: %#"PRIx32, res);
}

/*
 * Mirror the reference PIL anti-rollback check: reject firmware whose
 * anti_rollback field in the OEM metadata is below the device version fused
 * in QFPROM, then advance the fuse when the image is newer. Fuse PTA open
 * and read failures are non-fatal: ARB enforcement requires secure boot,
 * which requires an accessible QFPROM. MBN v5 images without OEM metadata
 * skip the check.
 */
static TEE_Result check_anti_rollback(const struct pas_hashseg *hs,
				      uint32_t pas_id __unused)
{
	static const TEE_UUID fuse_uuid = PTA_QCOM_FUSE_UUID;
	TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	TEE_Param params[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	struct pas_meta meta = { };
	uint32_t dev_ver = 0;
	uint32_t pt = 0;

	res = pas_hashseg_get_meta(hs, &meta);
	if (res == TEE_ERROR_NO_DATA) {
		/* MBN v5: no metadata → no ARB field to check. */
		return TEE_SUCCESS;
	}
	if (res) {
		EMSG("PAS ARB: bad OEM metadata");
		return res;
	}

	res = TEE_OpenTASession(&fuse_uuid, TEE_TIMEOUT_INFINITE, 0, NULL,
				&sess, NULL);
	if (res) {
		EMSG("PAS ARB: cannot open fuse PTA: %#"PRIx32, res);
		/* Non-fatal on non-secure-boot boards: ARB is not enforced. */
		return TEE_SUCCESS;
	}

	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_GET_PIL_ROLLBACK_VERSION,
				  pt, params, NULL);
	if (res) {
		EMSG("PAS ARB: cannot read device version: %#"PRIx32, res);
		TEE_CloseTASession(sess);
		return TEE_SUCCESS;
	}

	dev_ver = params[0].value.a;
	if (!dev_ver) {
		TEE_CloseTASession(sess);
		return TEE_SUCCESS; /* ARB enforcement disabled in fuses */
	}

	if (meta.anti_rollback < dev_ver) {
		EMSG("PAS ARB: image version %"PRIu32" < device %"PRIu32,
		     meta.anti_rollback, dev_ver);
		TEE_CloseTASession(sess);
		return TEE_ERROR_SECURITY;
	}

	if (meta.anti_rollback > dev_ver)
		blow_arb_fuse(sess, meta.anti_rollback);

	TEE_CloseTASession(sess);

	DMSG("PAS ARB: image version %"PRIu32" >= device %"PRIu32,
	     meta.anti_rollback, dev_ver);
	return TEE_SUCCESS;
}
#endif /* CFG_QCOM_PAS_ARB */

/*
 * Determine the per-segment hash digest size for @slot's metadata, mirroring
 * the reference segment-hash-algorithm selection: the OEM metadata's
 * root_cert_sel (word 28) selects the fuse-configured algorithm via the fuse
 * PTA on platforms that implement the field; images without OEM metadata
 * (MBN v5) or without the fuse field use the reference default
 * root_cert_sel of 0.
 */
#define SECBOOT_DEFAULT_ROOT_CERT_SEL	0U

static TEE_Result get_segment_hash_size(const struct pas_md_slot *slot,
					uint32_t *hash_size)
{
	static const TEE_UUID fuse_uuid = PTA_QCOM_FUSE_UUID;
	TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	TEE_Param params[TEE_NUM_PARAMS] = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t root_cert_sel = SECBOOT_DEFAULT_ROOT_CERT_SEL;
	uint32_t version = 0;
	uint32_t pt = 0;

	/*
	 * MBN v5 hash tables are always SHA-256 by format definition; the
	 * fuse-selected digest applies only to v6. Read the version first and
	 * short-circuit v5 before consulting the fuse.
	 */
	res = pas_hashseg_peek_version(slot->md, slot->md_size, &version);
	if (res)
		return res;
	if (version == PAS_MBN_VERSION_5) {
		*hash_size = TEE_SHA256_HASH_SIZE;
		return TEE_SUCCESS;
	}

	res = pas_hashseg_peek_root_cert_sel(slot->md, slot->md_size,
					     &root_cert_sel);
	if (res == TEE_ERROR_NO_DATA)
		root_cert_sel = SECBOOT_DEFAULT_ROOT_CERT_SEL;
	else if (res)
		return res;

	res = TEE_OpenTASession(&fuse_uuid, TEE_TIMEOUT_INFINITE, 0, NULL,
				&sess, NULL);
	if (res) {
		EMSG("PAS auth: cannot open fuse PTA: %#"PRIx32, res);
		return res;
	}

	params[0].value.a = root_cert_sel;
	pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT, TEE_PARAM_TYPE_NONE,
			     TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	res = TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_FUSE_GET_SEGMENT_HASH_SIZE, pt,
				  params, NULL);
	TEE_CloseTASession(sess);
	if (res) {
		EMSG("PAS auth: cannot read segment hash size: %#"PRIx32, res);
		return res;
	}

	*hash_size = params[0].value.b;

	return TEE_SUCCESS;
}

/*
 * Parse, authenticate and bind the INIT_IMAGE metadata: signature, cert
 * chain, SW_ID/HW binding and anti-rollback. Mirrors the reference PIL
 * image-authentication flow, which runs this crypto work BEFORE the REE
 * loads any ELF segments into the carveout, not at AUTH_AND_RESET. The
 * authenticated pas_hashseg is cached in the slot so verify_segments() can
 * re-check the loaded segments without re-doing the crypto.
 */
#endif /* CFG_QCOM_PAS_SECURE_BOOT */
static TEE_Result authenticate_metadata(struct qcom_pas_session *s,
					uint32_t pas_id)
{
	struct pas_md_slot *slot = find_md_slot(s, pas_id);
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t hash_size = TEE_SHA384_HASH_SIZE;

	if (!slot) {
		EMSG("PAS auth: no metadata pas_id=%"PRIu32
		     " (call INIT_IMAGE first)", pas_id);
		return TEE_ERROR_BAD_STATE;
	}

#ifdef CFG_QCOM_PAS_SECURE_BOOT
	res = get_segment_hash_size(slot, &hash_size);
	if (res) {
		EMSG("PAS auth: cannot determine segment hash size: %#"PRIx32,
		     res);
		return res;
	}
#endif

	res = pas_hashseg_parse(slot->md, slot->md_size, hash_size,
				&slot->hs);
	if (res) {
		EMSG("PAS auth: pas_hashseg_parse failed: %#"PRIx32, res);
		return res;
	}

#ifdef CFG_QCOM_PAS_SECURE_BOOT
	res = verify_authenticity(&slot->hs, pas_id);
	if (res) {
		EMSG("PAS auth: verify_authenticity failed: %#"PRIx32, res);
		return res;
	}

#ifdef CFG_QCOM_PAS_ARB
	res = check_anti_rollback(&slot->hs, pas_id);
	if (res) {
		EMSG("PAS auth: check_anti_rollback failed: %#"PRIx32, res);
		return res;
	}
#endif
#endif

	slot->authenticated = true;

	return TEE_SUCCESS;
}

/*
 * Re-verify the per-segment hashes of firmware the REE has now loaded into
 * its carveout, against the hash table authenticated at INIT_IMAGE. Mirrors
 * the reference PIL segment-verification flow, the only work AUTH_AND_RESET
 * does on the reference side.
 */
static TEE_Result verify_segments(struct qcom_pas_session *s, uint32_t pas_id,
				  TEE_Param params[TEE_NUM_PARAMS])
{
	struct pas_md_slot *slot = find_md_slot(s, pas_id);

	if (!slot || !slot->authenticated) {
		EMSG("PAS auth: pas_id=%"PRIu32
		     " not authenticated (call INIT_IMAGE first)", pas_id);
		return TEE_ERROR_BAD_STATE;
	}

	return verify_integrity(s, pas_id, &slot->hs, params);
}
#endif /* CFG_QCOM_PAS_HASH_VERIFY */

static TEE_Result qcom_pas_init_image(struct qcom_pas_session *s, uint32_t pt,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;

	if (IS_ENABLED(CFG_QCOM_PAS_HASH_VERIFY)) {
		res = save_metadata(s, pt, params);
		if (res != TEE_SUCCESS)
			return res;
	}

	res = TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
				  PTA_QCOM_PAS_INIT_IMAGE, pt, params, NULL);
	if (res)
		return res;

#ifdef CFG_QCOM_PAS_HASH_VERIFY
	res = authenticate_metadata(s, params[0].value.a);
	if (res)
		EMSG("PAS firmware auth failed: %#"PRIx32, res);
#endif

	return res;
}

static TEE_Result qcom_pas_auth_and_reset(struct qcom_pas_session *s,
					  uint32_t pt,
					  TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						TEE_PARAM_TYPE_VALUE_INPUT,
						TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE);

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

#ifdef CFG_QCOM_PAS_HASH_VERIFY
	{
		TEE_Result res = verify_segments(s, params[0].value.a, params);

		if (res) {
			EMSG("PAS auth: segment verification failed: %#"PRIx32,
			     res);
			return res;
		}
	}
#else
	(void)s;
#endif

	return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
				   PTA_QCOM_PAS_AUTH_AND_RESET,
				   pt, params, NULL);
}

TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t pt,
				    TEE_Param params[TEE_NUM_PARAMS],
				    void **sess_ctx)
{
	static const TEE_UUID uuid = PTA_QCOM_PAS_UUID;
	TEE_PropSetHandle h = TEE_HANDLE_NULL;
	struct qcom_pas_session *s = NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	TEE_Identity id = { };

	res = TEE_AllocatePropertyEnumerator(&h);
	if (res != TEE_SUCCESS)
		goto error;

	TEE_StartPropertyEnumerator(h, TEE_PROPSET_CURRENT_CLIENT);

	res = TEE_GetPropertyAsIdentity(h, NULL, &id);
	if (res != TEE_SUCCESS)
		goto error;

	if (id.login != TEE_LOGIN_REE_KERNEL) {
		res = TEE_ERROR_ACCESS_DENIED;
		goto error;
	}

	s = TEE_Malloc(sizeof(*s), TEE_MALLOC_FILL_ZERO);
	if (!s) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto error;
	}

	if (!session_refcount) {
		res = TEE_OpenTASession(&uuid, TEE_TIMEOUT_INFINITE, pt, params,
					&pta_session, NULL);
		if (res != TEE_SUCCESS)
			goto error_free;
	}

	session_refcount++;
	*sess_ctx = s;
	res = TEE_SUCCESS;
	goto out;

error_free:
	TEE_Free(s);
error:
	*sess_ctx = NULL;
out:
	if (h)
		TEE_FreePropertyEnumerator(h);

	return res;
}

void TA_CloseSessionEntryPoint(void *sess_ctx)
{
	struct qcom_pas_session *s = sess_ctx;

	if (s) {
		size_t i = 0;

		for (i = 0; i < PAS_MD_SLOTS; i++)
			TEE_Free(s->md[i].md);
		TEE_Free(s);
	}

	session_refcount--;

	if (!session_refcount)
		TEE_CloseTASession(pta_session);
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
				      uint32_t pt,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	struct qcom_pas_session *s = sess_ctx;

	switch (cmd_id) {
	case TA_QCOM_PAS_IS_SUPPORTED:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_IS_SUPPORTED,
					   pt, params, NULL);
	case TA_QCOM_PAS_CAPABILITIES:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_CAPABILITIES,
					   pt, params, NULL);
	case TA_QCOM_PAS_INIT_IMAGE:
		return qcom_pas_init_image(s, pt, params);
	case TA_QCOM_PAS_MEM_SETUP:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_MEM_SETUP,
					   pt, params, NULL);
	case TA_QCOM_PAS_GET_RESOURCE_TABLE:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_GET_RESOURCE_TABLE,
					   pt, params, NULL);
	case TA_QCOM_PAS_AUTH_AND_RESET:
		return qcom_pas_auth_and_reset(s, pt, params);
	case TA_QCOM_PAS_SET_REMOTE_STATE:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_SET_REMOTE_STATE,
					   pt, params, NULL);
	case TA_QCOM_PAS_SHUTDOWN:
		return TEE_InvokeTACommand(pta_session, TEE_TIMEOUT_INFINITE,
					   PTA_QCOM_PAS_SHUTDOWN,
					   pt, params, NULL);
	default:
		return TEE_ERROR_NOT_IMPLEMENTED;
	}
}
