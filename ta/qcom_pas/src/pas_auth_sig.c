// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <mbedtls/bignum.h>
#include <mbedtls/md.h>
#include <mbedtls/oid.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/x509_crt.h>
#include <pas_auth.h>
#include <string.h>
#include <string_ext.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <util.h>
#include <utee_defines.h>

static TEE_Result md_from_tee(uint32_t hash_algo, mbedtls_md_type_t *md)
{
	switch (hash_algo) {
	case TEE_ALG_SHA256:
		*md = MBEDTLS_MD_SHA256;
		return TEE_SUCCESS;
	case TEE_ALG_SHA384:
		*md = MBEDTLS_MD_SHA384;
		return TEE_SUCCESS;
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}

static TEE_Result digest(uint32_t hash_algo, const uint8_t *msg, size_t msg_len,
			 uint8_t *out, size_t *out_len)
{
	TEE_OperationHandle op = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;

	res = TEE_AllocateOperation(&op, hash_algo, TEE_MODE_DIGEST, 0);
	if (res != TEE_SUCCESS)
		return res;

	res = TEE_DigestDoFinal(op, msg, msg_len, out, out_len);

	TEE_FreeOperation(op);

	return res;
}

/*
 * Certificate policy for image-signing chains (kodiak/lemans):
 *   - signature MDs restricted to SHA-256 / SHA-384
 *   - public keys restricted to RSA (PSS) and ECDSA
 *   - ECDSA curve restricted to NIST P-384
 *   - notBefore/notAfter are NOT enforced (TZ has no trusted RTC)
 * Chain-structure checks (leaf !CA, intermediate CA=TRUE, pathlen) mirror
 * the reference secure-boot chain-list validation. RSA modulus bounds
 * (2048..4096) and the fixed public exponent (65537) are checked per-leaf
 * in pas_auth_sig_algo_from_leaf() (the mbedTLS profile has no max-bitlen
 * or exponent field).
 */
#define PAS_RSA_MIN_BITS	2048U
#define PAS_RSA_MAX_BITS	4096U
#define PAS_RSA_PUBLIC_EXPONENT	65537

static const mbedtls_x509_crt_profile pas_crt_profile = {
	.allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) |
		       MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384),
	.allowed_pks = MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_RSA) |
		       MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_RSASSA_PSS) |
		       MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_ECDSA) |
		       MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_ECKEY),
	.allowed_curves = MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP384R1),
	.rsa_min_bitlen = PAS_RSA_MIN_BITS,
};

/*
 * Enforce the leaf RSA key constraints beyond what the mbedTLS profile
 * checks: modulus in [2048, 4096] bits and public exponent exactly 65537.
 * The mbedTLS profile already rejects a modulus below rsa_min_bitlen
 * during chain verify; this adds the upper bound and the exponent check.
 */
static TEE_Result check_rsa_leaf_constraints(mbedtls_pk_context *pk)
{
	mbedtls_rsa_context *rsa = mbedtls_pk_rsa(*pk);
	mbedtls_mpi e = { };
	size_t bits = 0;
	int rc = 0;

	bits = mbedtls_pk_get_bitlen(pk);
	if (bits < PAS_RSA_MIN_BITS || bits > PAS_RSA_MAX_BITS) {
		EMSG("PAS auth: RSA modulus %zu bits out of [%u, %u]", bits,
		     PAS_RSA_MIN_BITS, PAS_RSA_MAX_BITS);
		return TEE_ERROR_SECURITY;
	}

	mbedtls_mpi_init(&e);
	if (mbedtls_rsa_export(rsa, NULL, NULL, NULL, NULL, &e)) {
		mbedtls_mpi_free(&e);
		return TEE_ERROR_SECURITY;
	}

	rc = mbedtls_mpi_cmp_int(&e, PAS_RSA_PUBLIC_EXPONENT);
	mbedtls_mpi_free(&e);
	if (rc) {
		EMSG("PAS auth: RSA public exponent != %d",
		     PAS_RSA_PUBLIC_EXPONENT);
		return TEE_ERROR_SECURITY;
	}

	return TEE_SUCCESS;
}

/*
 * Require the leaf to carry the code-signing Extended Key Usage OID when
 * @enforced is set. The caller derives @enforced from the OEM_CONFIG2
 * EKU_ENFORCEMENT_EN fuse.
 */
static TEE_Result check_eku(const mbedtls_x509_crt *leaf, bool enforced)
{
	size_t oid_len = MBEDTLS_OID_SIZE(MBEDTLS_OID_CODE_SIGNING);

	if (!enforced)
		return TEE_SUCCESS;

	if (mbedtls_x509_crt_check_extended_key_usage(leaf,
						      MBEDTLS_OID_CODE_SIGNING,
						      oid_len)) {
		EMSG("PAS auth: leaf cert missing code-signing EKU");
		return TEE_ERROR_SECURITY;
	}

	return TEE_SUCCESS;
}

/*
 * Enforce structural constraints across the parsed chain: the leaf must
 * not assert CA=TRUE, every issuer (non-leaf) must be a CA, and the leaf
 * must assert the digitalSignature KeyUsage bit. The pathLenConstraint is
 * validated inside mbedtls_x509_crt_verify_with_profile() during RFC 5280
 * path building. Uses the public ca_istrue/check_key_usage accessors (the
 * underlying fields are private).
 *
 * @eku_enforced additionally requires the leaf to carry the code-signing
 * Extended Key Usage OID, gated on the OEM_CONFIG2 EKU_ENFORCEMENT_EN fuse.
 */
static TEE_Result check_chain_constraints(const mbedtls_x509_crt *leaf,
					  bool eku_enforced)
{
	const mbedtls_x509_crt *crt = NULL;
	size_t depth = 0;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ku = MBEDTLS_X509_KU_DIGITAL_SIGNATURE;

	if (mbedtls_x509_crt_check_key_usage(leaf, ku)) {
		EMSG("PAS auth: leaf cert missing digitalSignature KeyUsage");
		return TEE_ERROR_SECURITY;
	}

	res = check_eku(leaf, eku_enforced);
	if (res)
		return res;

	for (crt = leaf; crt; crt = crt->next, depth++) {
		int ca = mbedtls_x509_crt_get_ca_istrue(crt);

		if (ca < 0) {
			EMSG("PAS auth: cannot read CA flag at depth %zu",
			     depth);
			return TEE_ERROR_SECURITY;
		}

		if (crt == leaf) {
			if (ca) {
				EMSG("PAS auth: leaf cert asserts CA");
				return TEE_ERROR_SECURITY;
			}
			continue;
		}

		/* Non-leaf certs (issuers) must be CAs. */
		if (!ca) {
			EMSG("PAS auth: issuer at depth %zu is not a CA",
			     depth);
			return TEE_ERROR_SECURITY;
		}
	}

	return TEE_SUCCESS;
}

/*
 * Compare two mbedTLS ASN.1 buffers for equality (same length and bytes).
 * Absent buffers (len == 0) never compare equal - callers gate on presence
 * before calling this.
 */
static bool asn1_buf_eq(const mbedtls_x509_buf *a, const mbedtls_x509_buf *b)
{
	return a->len && a->len == b->len && !memcmp(a->p, b->p, a->len);
}

/*
 * Verify that @issuer actually issued @subject: beyond the subject/issuer
 * name match mbedTLS's own chain-verify already performs, cross-check the
 * subject's Authority Key Identifier (key id + serial number, when present)
 * against the issuer's Subject Key Identifier and certificate serial
 * number. Name-only matching (what mbedTLS's parent search does on its own)
 * cannot distinguish two issuer candidates that share a subject DN but hold
 * different keys; AKID/SKID/serial linkage disambiguates that case.
 */
static TEE_Result check_issuer_linkage(const mbedtls_x509_crt *issuer,
				       const mbedtls_x509_crt *subject)
{
	const mbedtls_x509_authority *akid = &subject->authority_key_id;

	if (akid->keyIdentifier.len &&
	    issuer->subject_key_id.len &&
	    !asn1_buf_eq(&akid->keyIdentifier, &issuer->subject_key_id)) {
		EMSG("PAS auth: AKID/SKID mismatch in cert chain");
		return TEE_ERROR_SECURITY;
	}

	if (akid->authorityCertSerialNumber.len &&
	    !asn1_buf_eq(&akid->authorityCertSerialNumber, &issuer->serial)) {
		EMSG("PAS auth: AKID serial mismatch in cert chain");
		return TEE_ERROR_SECURITY;
	}

	return TEE_SUCCESS;
}

/*
 * Chain-length bounds matching the reference secure-boot chain-count check,
 * applied before any further validation. A chain carries a leaf, an optional
 * intermediate, and one or more roots; the non-root levels are bounded by
 * PAS_MAX_CERT_CHAIN_LEVEL and the roots by PAS_MAX_NUM_ROOT_CERTS.
 */
#define PAS_MIN_NUM_CERTS	2U
#define PAS_MAX_CERT_CHAIN_LEVEL 3U
#define PAS_MAX_NUM_ROOT_CERTS	4U
#define PAS_TOTAL_MAX_CERTS	(PAS_MAX_NUM_ROOT_CERTS + \
				 PAS_MAX_CERT_CHAIN_LEVEL - 1)

/* The fixed root index accepted when no provisioned root is usable. */
#define PAS_DEFAULT_SAFE_ROOT_INDEX	3U

TEE_Result pas_auth_check_root_cert_index(uint32_t root_cert_sel,
					  uint32_t num_roots,
					  uint32_t activation_list,
					  uint32_t revocation_list)
{
	uint32_t non_avail = 0;
	uint32_t mask = 0;

	if (!num_roots || num_roots > PAS_MAX_NUM_ROOT_CERTS ||
	    root_cert_sel >= num_roots)
		return TEE_ERROR_SECURITY;

	/*
	 * A root is unusable if it is revoked or not activated. If every
	 * provisioned root is unusable, only the fixed safe-root index is
	 * accepted; otherwise the selected index must be active and not
	 * revoked.
	 */
	mask = GENMASK_32(PAS_MAX_NUM_ROOT_CERTS - 1, 0);
	non_avail = (revocation_list & mask) | (~activation_list & mask);

	if (__builtin_popcount(non_avail) >= (int)PAS_MAX_NUM_ROOT_CERTS) {
		if (root_cert_sel == PAS_DEFAULT_SAFE_ROOT_INDEX)
			return TEE_SUCCESS;
		EMSG("PAS auth: no usable root cert, sel %"PRIu32,
		     root_cert_sel);
		return TEE_ERROR_SECURITY;
	}

	if ((activation_list >> root_cert_sel) & 1U &&
	    !((revocation_list >> root_cert_sel) & 1U))
		return TEE_SUCCESS;

	EMSG("PAS auth: root cert %"PRIu32" inactive or revoked",
	     root_cert_sel);
	return TEE_ERROR_SECURITY;
}

TEE_Result pas_auth_verify_cert_chain(const uint8_t *chain_der,
				      size_t chain_der_len, bool eku_enforced,
				      uint32_t num_roots,
				      uint32_t root_cert_sel,
				      const uint8_t **leaf_der,
				      size_t *leaf_der_len,
				      const uint8_t **roots_der,
				      size_t *roots_der_len)
{
	TEE_Result res = TEE_ERROR_SECURITY;
	const mbedtls_x509_crt *crt = NULL;
	mbedtls_x509_crt *sel_root = NULL;
	mbedtls_x509_crt *leaf = NULL;
	mbedtls_x509_crt chain = { };
	mbedtls_x509_crt trust = { };
	size_t num_prefix = 0;
	size_t sel_index = 0;
	size_t roots_off = 0;
	size_t num_certs = 0;
	uint32_t flags = 0;
	size_t off = 0;
	size_t i = 0;
	int rc = 0;

	if (!chain_der || !chain_der_len || !leaf_der || !leaf_der_len ||
	    !num_roots || num_roots > PAS_MAX_NUM_ROOT_CERTS ||
	    root_cert_sel >= num_roots)
		return TEE_ERROR_BAD_PARAMETERS;

	mbedtls_x509_crt_init(&chain);
	mbedtls_x509_crt_init(&trust);

	/*
	 * The certificate region is concatenated DER certificates followed by
	 * 0xFF padding: leaf and optional intermediate first, then @num_roots
	 * provisioned roots. mbedtls_x509_crt_parse_der() consumes exactly one
	 * certificate per call, so walk the buffer certificate by certificate,
	 * stopping at the first byte that is not a DER SEQUENCE (the padding),
	 * and track each certificate's byte offset for the pointers handed
	 * back to the caller.
	 */
	while (off < chain_der_len && chain_der[off] == 0x30) {
		mbedtls_x509_crt *added = NULL;

		if (num_certs >= PAS_TOTAL_MAX_CERTS)
			break;

		if (mbedtls_x509_crt_parse_der(&chain, chain_der + off,
					       chain_der_len - off)) {
			EMSG("PAS auth: cert %zu parse failed", num_certs);
			goto out;
		}

		/* The just-parsed certificate is the last one in the list. */
		added = &chain;
		while (added->next)
			added = added->next;

		off += added->raw.len;
		num_certs++;
	}

	/*
	 * The non-root levels (leaf, optional intermediate) must satisfy the
	 * conventional chain bounds; the roots sit on top of them.
	 */
	if (num_certs <= num_roots) {
		EMSG("PAS auth: chain has %zu certs, need > %"PRIu32" roots",
		     num_certs, num_roots);
		goto out;
	}
	num_prefix = num_certs - num_roots;
	if (num_prefix < (PAS_MIN_NUM_CERTS - 1) ||
	    num_prefix > (PAS_MAX_CERT_CHAIN_LEVEL - 1)) {
		EMSG("PAS auth: chain has %zu non-root certs, want [%u, %u]",
		     num_prefix, PAS_MIN_NUM_CERTS - 1,
		     PAS_MAX_CERT_CHAIN_LEVEL - 1);
		goto out;
	}

	leaf = &chain;

	/*
	 * Locate the selected root (prefix certs, then root_cert_sel roots) and
	 * the byte offset where the root region begins (after the prefix).
	 */
	sel_index = num_prefix + root_cert_sel;
	for (crt = &chain, i = 0; crt; crt = crt->next, i++) {
		if (i < num_prefix)
			roots_off += crt->raw.len;
		if (i == sel_index)
			sel_root = (mbedtls_x509_crt *)crt;
	}
	if (!sel_root) {
		EMSG("PAS auth: selected root %zu not present", sel_index);
		goto out;
	}

	/*
	 * Verify internal chain integrity up to the selected root: parse it
	 * into a standalone trust store so mbedTLS matches it as the anchor
	 * without walking into the other roots in the chain list. Root-of-trust
	 * binding against the fuse digest happens later in
	 * pas_auth_check_root_of_trust().
	 */
	if (mbedtls_x509_crt_parse_der(&trust, sel_root->raw.p,
				       sel_root->raw.len)) {
		EMSG("PAS auth: root cert re-parse failed");
		goto out;
	}

	rc = mbedtls_x509_crt_verify_with_profile(leaf, &trust, NULL,
						  &pas_crt_profile, NULL,
						  &flags, NULL, NULL);
	/*
	 * Certificate validity dates are not checked (no trusted RTC), so an
	 * expired or not-yet-valid cert must not fail authentication. Clear
	 * those flags and re-evaluate; any other flag is fatal.
	 */
	flags &= ~(uint32_t)(MBEDTLS_X509_BADCERT_EXPIRED |
			     MBEDTLS_X509_BADCERT_FUTURE);
	if (rc && flags) {
		EMSG("PAS auth: cert chain verify failed (%#"PRIx32")", flags);
		goto out;
	}

	res = check_chain_constraints(leaf, eku_enforced);
	if (res)
		goto out;

	/*
	 * Cross-check issuer linkage across the prefix (leaf up to the last
	 * intermediate), then from the last prefix cert to the selected root.
	 * crt->next cannot be used to walk this last link: with multiple
	 * roots it points at whichever root sits next in the buffer, not
	 * necessarily the selected one.
	 */
	crt = leaf;
	for (i = 0; i + 1 < num_prefix; i++) {
		res = check_issuer_linkage(crt->next, crt);
		if (res)
			goto out;
		crt = crt->next;
	}
	res = check_issuer_linkage(sel_root, crt);
	if (res)
		goto out;

	/*
	 * mbedtls_x509_crt_parse_der() copies DER into its own allocation;
	 * raw.p is invalid after free. Hand out pointers back into chain_der:
	 * the leaf starts at chain_der[0]; the root region spans all roots,
	 * from the end of the prefix to the end of the last parsed cert.
	 */
	if (roots_off > off || off > chain_der_len ||
	    leaf->raw.len > chain_der_len) {
		EMSG("PAS auth: cert DER length exceeds chain buffer");
		res = TEE_ERROR_SECURITY;
		goto out;
	}

	*leaf_der = chain_der;
	*leaf_der_len = leaf->raw.len;
	if (roots_der)
		*roots_der = chain_der + roots_off;
	if (roots_der_len)
		*roots_der_len = off - roots_off;

	res = TEE_SUCCESS;
out:
	mbedtls_x509_crt_free(&trust);
	mbedtls_x509_crt_free(&chain);

	return res;
}

TEE_Result pas_auth_check_root_of_trust(uint32_t hash_algo, size_t hash_size,
					const uint8_t *root_der,
					size_t root_der_len,
					const uint8_t *expected)
{
	if (!root_der || !root_der_len || !expected)
		return TEE_ERROR_BAD_PARAMETERS;

	return pas_auth_verify_hash(hash_algo, root_der, root_der_len,
				    expected, hash_size);
}

TEE_Result pas_auth_sig_algo_from_leaf(const uint8_t *leaf_der,
				       size_t leaf_der_len, uint32_t *sig_algo,
				       uint32_t *hash_algo, uint32_t *salt_len)
{
	mbedtls_pk_type_t pk_type = MBEDTLS_PK_NONE;
	TEE_Result res = TEE_ERROR_SECURITY;
	mbedtls_x509_crt leaf = { };

	if (!leaf_der || !leaf_der_len || !sig_algo || !hash_algo || !salt_len)
		return TEE_ERROR_BAD_PARAMETERS;

	mbedtls_x509_crt_init(&leaf);
	if (mbedtls_x509_crt_parse_der(&leaf, leaf_der, leaf_der_len)) {
		EMSG("PAS auth: leaf cert parse failed");
		goto out;
	}

	pk_type = mbedtls_pk_get_type(&leaf.pk);
	switch (pk_type) {
	case MBEDTLS_PK_RSA:
	case MBEDTLS_PK_RSASSA_PSS:
		res = check_rsa_leaf_constraints(&leaf.pk);
		if (res)
			break;
		/*
		 * RSA-PSS images are signed over SHA-256 with MGF1 = SHA-256
		 * and salt length = digest length (see verify).
		 */
		*hash_algo = TEE_ALG_SHA256;
		*sig_algo = TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256;
		*salt_len = TEE_SHA256_HASH_SIZE;
		res = TEE_SUCCESS;
		break;
	case MBEDTLS_PK_ECKEY:
	case MBEDTLS_PK_ECDSA:
		*hash_algo = TEE_ALG_SHA384;
		*sig_algo = TEE_ALG_ECDSA_SHA384;
		*salt_len = 0;
		res = TEE_SUCCESS;
		break;
	default:
		EMSG("PAS auth: unsupported key type %d", pk_type);
		res = TEE_ERROR_NOT_SUPPORTED;
		break;
	}
out:
	mbedtls_x509_crt_free(&leaf);

	return res;
}

/*
 * The signature field in the hash segment is a fixed-size reservation, so the
 * actual signature is shorter than @field_len and the remainder is padding.
 * mbedTLS rejects trailing bytes after the signature, so derive the true
 * length: an ECDSA signature is a DER SEQUENCE whose encoded length gives the
 * exact size; an RSA signature is exactly the modulus size.
 */
static size_t ecdsa_der_sig_len(const uint8_t *sig, size_t field_len)
{
	size_t len = 0;

	if (field_len < 2 || sig[0] != 0x30)
		return field_len;

	if (sig[1] < 0x80) {
		/* Short-form length: one length byte follows the tag. */
		len = (size_t)sig[1] + 2;
	} else if (sig[1] == 0x81 && field_len >= 3) {
		/* Long-form, one length byte. */
		len = (size_t)sig[2] + 3;
	} else {
		return field_len;
	}

	return len <= field_len ? len : field_len;
}

TEE_Result pas_auth_verify_signature(uint32_t sig_algo, uint32_t hash_algo,
				     uint32_t salt_len,
				     const uint8_t *leaf_der,
				     size_t leaf_der_len,
				     const uint8_t *msg, size_t msg_len,
				     const uint8_t *sig, size_t sig_len)
{
	uint8_t dgst[PAS_AUTH_MAX_HASH_SIZE] = { };
	mbedtls_md_type_t md = MBEDTLS_MD_NONE;
	TEE_Result res = TEE_ERROR_SECURITY;
	size_t dgst_len = sizeof(dgst);
	mbedtls_x509_crt leaf = { };
	size_t actual_sig_len = 0;
	int rc = 0;

	if (!leaf_der || !leaf_der_len || !msg || !msg_len || !sig || !sig_len)
		return TEE_ERROR_BAD_PARAMETERS;

	res = md_from_tee(hash_algo, &md);
	if (res != TEE_SUCCESS)
		return res;

	res = digest(hash_algo, msg, msg_len, dgst, &dgst_len);
	if (res != TEE_SUCCESS)
		return res;

	mbedtls_x509_crt_init(&leaf);
	if (mbedtls_x509_crt_parse_der(&leaf, leaf_der, leaf_der_len)) {
		res = TEE_ERROR_SECURITY;
		goto out;
	}

	switch (sig_algo) {
	case TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256:
	case TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA384: {
		/*
		 * RSA-PSS uses MGF1 = SHA-256, matching the message digest and
		 * salt length chosen in pas_auth_sig_algo_from_leaf(). The
		 * signature is exactly the modulus size; any bytes past that in
		 * the reserved field are padding.
		 */
		mbedtls_pk_rsassa_pss_options opts = {
			.mgf1_hash_id = MBEDTLS_MD_SHA256,
			.expected_salt_len = salt_len,
		};

		actual_sig_len = mbedtls_pk_get_len(&leaf.pk);
		if (!actual_sig_len || actual_sig_len > sig_len) {
			res = TEE_ERROR_SECURITY;
			goto out;
		}

		rc = mbedtls_pk_verify_ext(MBEDTLS_PK_RSASSA_PSS, &opts,
					   &leaf.pk, md, dgst, dgst_len,
					   sig, actual_sig_len);
		break;
	}
	case TEE_ALG_ECDSA_SHA256:
	case TEE_ALG_ECDSA_SHA384:
		actual_sig_len = ecdsa_der_sig_len(sig, sig_len);
		rc = mbedtls_pk_verify(&leaf.pk, md, dgst, dgst_len,
				       sig, actual_sig_len);
		break;
	default:
		res = TEE_ERROR_NOT_SUPPORTED;
		goto out;
	}

	if (rc) {
		EMSG("PAS auth: signature verify failed (%d)", rc);
		res = TEE_ERROR_SECURITY;
		goto out;
	}

	res = TEE_SUCCESS;
out:
	mbedtls_x509_crt_free(&leaf);
	memzero_explicit(dgst, sizeof(dgst));

	return res;
}
