// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * Signature-authentication backend for the PAS TA. Built only when
 * CFG_QCOM_PAS_AUTH is enabled (see pas_sig_auth.h).
 */

#include <config.h>
#include <pas_sig.h>
#include <pas_fuse.h>
#include <pas_mbn_parser.h>
#include <pas_meta.h>
#include <pas_policy.h>
#include <pta_qcom_fuse.h>
#include <pas_sig_auth.h>
#include <qcom_pas_priv.h>
#include <string_ext.h>
#include <tee_internal_api.h>
#include <utee_defines.h>
#include <util.h>

/*
 * Accepted metadata major versions: V0 (0) and V1 (1); minor must be 0.
 */
#define SECBOOT_METADATA_MAJOR_V0	0U
#define SECBOOT_METADATA_MAJOR_V1	1U
#define SECBOOT_METADATA_MINOR		0U

/*
 * Reject metadata whose major/minor version falls outside the accepted set.
 * Must be called after signature verification so the metadata is authenticated.
 */
static TEE_Result check_metadata_version(const struct pas_mbn *hs)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	struct pas_meta meta = { };

	res = pas_meta_get(hs, &meta);
	if (res == TEE_ERROR_NO_DATA)
		return TEE_SUCCESS; /* No OEM metadata (v5): nothing to check */
	if (res) {
		EMSG("PAS auth: bad OEM metadata in version check");
		return res;
	}

	if ((meta.major == SECBOOT_METADATA_MAJOR_V0 ||
	     meta.major == SECBOOT_METADATA_MAJOR_V1) &&
	    meta.minor == SECBOOT_METADATA_MINOR)
		return TEE_SUCCESS;

	EMSG("PAS auth: unsupported metadata version %"PRIu32".%"PRIu32,
	     meta.major, meta.minor);
	return TEE_ERROR_SECURITY;
}

/*
 * Bind the signed image to the peripheral being brought up: the metadata
 * SW_ID must match the value the platform assigns to this pas_id, so a
 * validly-signed image for one subsystem cannot be loaded onto another.
 * The metadata lives inside the already signature-verified region.
 */
static TEE_Result check_sw_binding(const struct pas_mbn *hs, uint32_t pas_id)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	struct pas_meta meta = { };
	uint32_t expected = 0;

	res = pas_meta_get(hs, &meta);
	if (res == TEE_ERROR_NO_DATA) {
		/* No OEM metadata (MBN v5): nothing to bind. */
		return TEE_SUCCESS;
	}
	if (res) {
		EMSG("PAS auth: bad OEM metadata");
		return res;
	}

	res = pas_policy_expected_swid(pas_id, &expected);
	if (res) {
		EMSG("PAS auth: no SW_ID binding for pas_id %"PRIu32, pas_id);
		return res;
	}

	if (meta.sw_id != expected) {
		EMSG("PAS auth: SW_ID got %#"PRIx32" want %#"PRIx32,
		     meta.sw_id, expected);
		return TEE_ERROR_SECURITY;
	}

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
 * True if the 2-bit option field at @shift within @flags requests its
 * SN-gated enable value.
 */
static bool pas_meta_option_sn_gated(uint32_t flags, uint32_t shift)
{
	return pas_meta_option(flags, shift) == PAS_META_OPTION_ENABLE_SN;
}

/*
 * Reject metadata whose 2-bit option fields carry the reserved value 3.
 */
static TEE_Result check_metadata_options(const struct pas_meta *meta)
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
	return TEE_ERROR_SECURITY;
}

/*
 * Device-identity fields consumed by the HW binding checks below, read via
 * pas_fuse_get_hw_binding_info().
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
					  const struct pas_device_ids *ids)
{
	bool oem_independent = meta->flags &
				BIT32(PAS_META_FLAG_OEM_ID_INDEPENDENT);
	bool model_independent = false;

	/*
	 * v0 metadata has no MODEL_ID_INDEPENDENT bit; it is implied by
	 * OEM_ID_INDEPENDENT.
	 */
	if (meta->major == 0)
		model_independent = oem_independent;
	else
		model_independent = meta->flags &
				    BIT32(PAS_META_FLAG_MODEL_ID_INDEPENDENT);

	if (!oem_independent && meta->oem_id != ids->oem_id) {
		EMSG("PAS auth: OEM_ID got %#"PRIx32" want %#"PRIx32,
		     meta->oem_id, ids->oem_id);
		return TEE_ERROR_SECURITY;
	}

	if (!model_independent && meta->model_id != ids->model_id) {
		EMSG("PAS auth: MODEL_ID got %#"PRIx32" want %#"PRIx32,
		     meta->model_id, ids->model_id);
		return TEE_ERROR_SECURITY;
	}

	return TEE_SUCCESS;
}

/*
 * Bind HW_ID (JTAG authentication bits) to the device fuse; checked only
 * when IN_USE_JTAG_ID is set.
 */
static TEE_Result check_jtag_binding(const struct pas_meta *meta,
				     const struct pas_device_ids *ids)
{
	if (!(meta->flags & BIT32(PAS_META_FLAG_IN_USE_JTAG_ID)))
		return TEE_SUCCESS;

	if (meta->hw_id == ids->jtag_id)
		return TEE_SUCCESS;

	EMSG("PAS auth: HW_ID got %#"PRIx32" want %#"PRIx32, meta->hw_id,
	     ids->jtag_id);
	return TEE_ERROR_SECURITY;
}

/*
 * Bind the device serial number against the metadata allow-list. Checked when
 * the metadata's USE_SERIAL_NUMBER flag is set, the APPS SECURE_BOOTn
 * USE_SERIAL_NUM fuse override forces it, or the DEBUG/root-revoke-activate/
 * UIE-key-switch option requests its SN-gated enable value. When a trigger
 * applies, a device with no fused serial is treated as unbindable and the
 * check fails rather than being skipped.
 */
static TEE_Result check_serial_binding(const struct pas_meta *meta,
				       const struct pas_device_ids *ids,
				       bool use_serial_num_override)
{
	static const uint32_t sn_gated_shifts[] = {
		PAS_META_FLAG_DEBUG_SHIFT,
		PAS_META_FLAG_ROOT_REVOKE_ACTIVATE_SHIFT,
		PAS_META_FLAG_UIE_KEY_SWITCH_SHIFT,
	};
	bool sn_gated = false;
	size_t i = 0;

	for (i = 0; i < ARRAY_SIZE(sn_gated_shifts); i++) {
		if (pas_meta_option_sn_gated(meta->flags, sn_gated_shifts[i])) {
			sn_gated = true;
			break;
		}
	}

	if (!(meta->flags & BIT32(PAS_META_FLAG_USE_SERIAL_NUMBER)) &&
	    !use_serial_num_override && !sn_gated)
		return TEE_SUCCESS;

	if (!ids->serial_num) {
		EMSG("PAS auth: serial binding required, no fused serial");
		return TEE_ERROR_SECURITY;
	}

	for (i = 0; i < ARRAY_SIZE(meta->serial_num); i++) {
		if (meta->serial_num[i] &&
		    meta->serial_num[i] == ids->serial_num)
			return TEE_SUCCESS;
	}

	EMSG("PAS auth: serial number %#"PRIx32" not in metadata allow-list",
	     ids->serial_num);
	return TEE_ERROR_SECURITY;
}

/*
 * Bind the SoC family|device version against the metadata allow-list;
 * checked only when IN_USE_SOC_HW_VERSION is set.
 */
static TEE_Result check_soc_vers_binding(const struct pas_meta *meta,
					 uint32_t fam_dev)
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
	return TEE_ERROR_SECURITY;
}

/*
 * Bind the signed image to this device's fuses and reject malformed option
 * fields (HW/OEM/MODEL/serial/SoC binding checks; UIE key-switch handling
 * excluded, out of scope for PAS peripheral images).
 */
static TEE_Result check_hw_binding(const struct pas_mbn *hs)
{
	struct pas_fuse_hw_binding_info info = { };
	struct pas_device_ids ids = { };
	struct pas_meta meta = { };
	TEE_Result res = TEE_ERROR_GENERIC;
	bool need_soc_vers = false;

	res = pas_meta_get(hs, &meta);
	if (res == TEE_ERROR_NO_DATA)
		return TEE_SUCCESS; /* No OEM metadata (v5): nothing to bind */
	if (res) {
		EMSG("PAS auth: bad OEM metadata in HW binding check");
		return res;
	}

	res = check_metadata_options(&meta);
	if (res)
		return res;

	need_soc_vers = meta.flags & BIT32(PAS_META_FLAG_IN_USE_SOC_HW_VERSION);
	res = pas_fuse_get_hw_binding_info(need_soc_vers, &info);
	if (res)
		return res;

	ids.oem_id = info.oem_id;
	ids.model_id = info.model_id;
	ids.jtag_id = info.jtag_id;
	ids.serial_num = info.serial_num;

	res = check_oem_model_binding(&meta, &ids);
	if (res)
		return res;

	res = check_jtag_binding(&meta, &ids);
	if (res)
		return res;

	res = check_serial_binding(&meta, &ids, info.use_serial_num_override);
	if (res)
		return res;

	res = check_soc_vers_binding(&meta, info.soc_fam_dev);
	if (res)
		return res;

	return TEE_SUCCESS;
}

/*
 * Reject a UIE-encrypted image. Image decryption is not supported here, so an
 * encrypted image cannot be authenticated as plaintext. Act only when the
 * image carries a UIE parameter block AND the OEM_CONFIG0 image-encryption
 * fuse is provisioned: an image with a UIE block on a device without the fuse
 * is treated as unencrypted.
 */
static TEE_Result reject_if_encrypted(const struct pas_mbn *hs)
{
	if (!hs->uie_encrypted)
		return TEE_SUCCESS;

	if (!pas_fuse_get_image_encryption_en())
		return TEE_SUCCESS;

	EMSG("PAS auth: UIE image encryption not supported");

	return TEE_ERROR_NOT_SUPPORTED;
}

/*
 * Enforce the policy-required signing authority. pas_policy_signer() names the
 * authority this port supports for @pas_id's SW_ID (currently always
 * PAS_OEM_SIGNED); any image carrying QTI signing material fails here because
 * QTI countersignature verification is not implemented. If a future SW_ID
 * requires PAS_QTI_SIGNED or PAS_DOUBLE_SIGNED, this fails hard too rather
 * than silently accepting the image on the OEM signature alone.
 */
static TEE_Result reject_if_double_signed(const struct pas_mbn *hs,
					  uint32_t pas_id)
{
	enum pas_sign_authority required = PAS_OEM_SIGNED;
	uint32_t swid = 0;
	TEE_Result res = TEE_ERROR_GENERIC;

	res = pas_policy_expected_swid(pas_id, &swid);
	if (res) {
		EMSG("PAS auth: no SW_ID binding for pas_id %"PRIu32, pas_id);
		return res;
	}

	required = pas_policy_signer(swid);

	if (required != PAS_OEM_SIGNED) {
		EMSG("PAS auth: signer class %d for SW_ID %#"PRIx32
		     " not implemented", required, swid);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	if (hs->qti_certs || hs->qti_sig) {
		EMSG("PAS auth: QTI-countersigned images are not supported");
		return TEE_ERROR_NOT_SUPPORTED;
	}

	return TEE_SUCCESS;
}

/*
 * Validate the certificate chain, bind its root to the device ROT and verify
 * the signature. Called only when secure boot is enabled; the caller has
 * already fail-closed on a fuse-PTA read error, so every step here runs
 * unconditionally.
 * @anchor is the OEM root-of-trust digest the caller already read from the
 * fuse PTA (PTA_QCOM_FUSE_ROOT_OF_TRUST_SIZE bytes).
 */
static TEE_Result verify_authenticity(const struct pas_mbn *hs,
				      uint32_t pas_id,
				      const uint8_t *anchor)
{
	uint32_t rot_hash_algo = TEE_ALG_SHA384;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint8_t *signed_copy = NULL;
	const uint8_t *roots = NULL;
	uint32_t sig_hash_algo = 0;
	const uint8_t *leaf = NULL;
	uint32_t sig_algo = 0;
	size_t signed_len = 0;
	size_t roots_len = 0;
	size_t leaf_len = 0;
	bool eku_enforced = false;

	if (!hs->oem_certs || !hs->oem_sig || !hs->signed_region) {
		EMSG("PAS auth: metadata is not OEM-signed");
		return TEE_ERROR_SECURITY;
	}

	/*
	 * Fail closed on a fuse-PTA read failure. A failed EKU enforcement
	 * fuse read aborts the whole authentication attempt; never fall back
	 * to "not enforced".
	 */
	res = pas_fuse_get_eku_enforcement_en(&eku_enforced);
	if (res) {
		EMSG("PAS auth: cannot read EKU enforcement fuse: %#"PRIx32,
		     res);
		return res;
	}

	res = pas_sig_verify_cert_chain(hs->oem_certs, hs->oem_certs_size,
					eku_enforced, 1, 0, &leaf,
					&leaf_len, &roots, &roots_len);
	if (res) {
		EMSG("PAS auth: OEM cert chain invalid: %#"PRIx32, res);
		return res;
	}

	/*
	 * The root-of-trust digest covers the single provisioned root,
	 * matching how the anchor fuse is provisioned.
	 */
	res = pas_sig_check_root_of_trust(rot_hash_algo,
					  PTA_QCOM_FUSE_ROOT_OF_TRUST_SIZE,
					  roots, roots_len, anchor);
	if (res) {
		EMSG("PAS auth: root-of-trust mismatch");
		return res;
	}

	/* Metadata is authenticated now; bind it to this peripheral. */
	res = check_metadata_version(hs);
	if (res)
		return res;

	res = check_sw_binding(hs, pas_id);
	if (res)
		return res;

	res = check_hw_binding(hs);
	if (res)
		return res;

	res = pas_sig_algo_from_leaf(leaf, leaf_len, &sig_algo,
				     &sig_hash_algo);
	if (res) {
		EMSG("PAS auth: cannot determine signature algorithm: %#"PRIx32,
		     res);
		return res;
	}

	res = pas_meta_signed_copy(hs, &signed_copy, &signed_len);
	if (res)
		return res;

	res = pas_sig_verify_signature(sig_algo, sig_hash_algo, leaf,
				       leaf_len, signed_copy, signed_len,
				       hs->oem_sig, hs->oem_sig_size);
	TEE_Free(signed_copy);
	if (res) {
		EMSG("PAS auth: OEM signature verify failed: %#"PRIx32, res);
		return res;
	}

	return TEE_SUCCESS;
}

/*
 * Determine the per-segment hash digest size for @slot's metadata. The OEM
 * metadata's root_cert_sel (word 28) selects the fuse-configured algorithm
 * via the fuse PTA on platforms that implement the field; images without OEM
 * metadata (MBN v5) or without the fuse field fall back to root_cert_sel 0.
 */
#define SECBOOT_DEFAULT_ROOT_CERT_SEL	0U

TEE_Result pas_sig_auth_hash_size(const struct pas_md_slot *slot,
				  uint32_t *hash_size)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t root_cert_sel = SECBOOT_DEFAULT_ROOT_CERT_SEL;
	uint32_t version = 0;

	/*
	 * MBN v5 hash tables are always SHA-256 by format definition; the
	 * fuse-selected digest applies only to v6. Read the version first and
	 * short-circuit v5 before consulting the fuse.
	 */
	res = pas_meta_peek_version(slot->meta_data, slot->meta_data_size,
				    &version);
	if (res)
		return res;
	if (version == PAS_MBN_VERSION_5) {
		*hash_size = TEE_SHA256_HASH_SIZE;
		return TEE_SUCCESS;
	}

	res = pas_meta_peek_root_cert_sel(slot->meta_data,
					  slot->meta_data_size,
					  &root_cert_sel);
	if (res == TEE_ERROR_NO_DATA)
		root_cert_sel = SECBOOT_DEFAULT_ROOT_CERT_SEL;
	else if (res)
		return res;

	res = pas_fuse_get_segment_hash_size(root_cert_sel, hash_size);
	if (res) {
		/*
		 * Fuse-PTA read failed. A silent SHA-384 fallback would
		 * mis-hash a v6 image whose fuse-selected algorithm differs.
		 * Fail hard rather than defaulting to any digest.
		 */
		EMSG("PAS auth: segment hash size read failed: %#"PRIx32, res);
		return TEE_ERROR_NOT_SUPPORTED;
	}

	return TEE_SUCCESS;
}

TEE_Result pas_sig_auth_authenticate(const struct pas_mbn *hs,
				     const uint8_t *meta_data,
				     size_t meta_data_size,
				     uint32_t pas_id, uint32_t hash_size,
				     const uint8_t *anchor)
{
	TEE_Result res = TEE_ERROR_GENERIC;

	res = reject_if_encrypted(hs);
	if (res)
		return res;

	res = reject_if_double_signed(hs, pas_id);
	if (res)
		return res;

	res = verify_authenticity(hs, pas_id, anchor);
	if (res)
		return res;

	res = pas_meta_verify_preamble(meta_data, meta_data_size,
				       hs->hash_table, hash_size);
	if (res)
		return res;

	return TEE_SUCCESS;
}
