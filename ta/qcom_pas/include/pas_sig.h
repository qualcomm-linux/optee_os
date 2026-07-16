/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PAS_AUTH_SIG_H
#define __PAS_AUTH_SIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>

/* Maximum supported digest size (SHA-384). */
#define PAS_AUTH_MAX_HASH_SIZE		48U

/*
 * pas_sig_verify_cert_chain() - parse and validate a DER certificate chain
 * @chain_der:      concatenated X.509 DER, leaf first, root(s) last
 * @chain_der_len:  length of @chain_der
 * @eku_enforced:   require the leaf to carry the code-signing Extended Key
 *                  Usage OID, gated on the OEM_CONFIG2 EKU_ENFORCEMENT_EN fuse
 * @num_roots:      number of provisioned root certificates in the chain (1 for
 *                  a conventional single-root chain; 2..4 when multiple roots
 *                  are provisioned)
 * @root_cert_sel:  index of the root the image chains to (0 when single-root)
 * @leaf_der:       out: pointer into @chain_der for the leaf cert
 * @leaf_der_len:   out: length of the leaf cert
 * @roots_der:      out (optional): pointer into @chain_der for the root region;
 *                  spans all @num_roots roots concatenated (single root when
 *                  @num_roots is 1). Pass NULL when not needed.
 * @roots_der_len:  out (optional): length of the root region; pass NULL when
 *                  not needed.
 *
 * With multiple roots, the chain is [leaf, (intermediate,) root_0..root_N-1];
 * the leaf is verified up to the @root_cert_sel-th root, and the returned
 * root region covers every provisioned root for binding to the device
 * root-of-trust digest. Verifies internal chain consistency using mbedTLS.
 * The returned DER pointers reference @chain_der and remain valid for its
 * lifetime.
 */
TEE_Result pas_sig_verify_cert_chain(const uint8_t *chain_der,
				     size_t chain_der_len, bool eku_enforced,
				     uint32_t num_roots,
				     uint32_t root_cert_sel,
				     const uint8_t **leaf_der,
				     size_t *leaf_der_len,
				     const uint8_t **roots_der,
				     size_t *roots_der_len);

/*
 * pas_sig_check_root_of_trust() - bind the chain root to the device anchor
 * @hash_algo:    TEE_ALG_SHA256 or TEE_ALG_SHA384
 * @hash_size:    digest size in bytes
 * @root_der:     DER bytes of the chain root certificate
 * @root_der_len: length of @root_der
 * @expected:     expected digest from the OTP root-of-trust fuse
 */
TEE_Result pas_sig_check_root_of_trust(uint32_t hash_algo, size_t hash_size,
				       const uint8_t *root_der,
				       size_t root_der_len,
				       const uint8_t *expected);

/*
 * pas_sig_algo_from_leaf() - pick signature scheme + digest from the leaf
 * @leaf_der:     DER bytes of the leaf certificate
 * @leaf_der_len: length of @leaf_der
 * @sig_algo:     out: matching TEE signature algorithm
 * @hash_algo:    out: message digest bound to the signature scheme
 *                (ECDSA -> SHA-384)
 *
 * The digest is derived from the leaf key type rather than supplied by the
 * caller, so the image uses the digest its signature was produced with.
 */
TEE_Result pas_sig_algo_from_leaf(const uint8_t *leaf_der,
				  size_t leaf_der_len, uint32_t *sig_algo,
				  uint32_t *hash_algo);

/*
 * pas_sig_verify_signature() - verify a signature using the leaf certificate
 * @sig_algo:    TEE signature algorithm (ECDSA)
 * @hash_algo:   matching digest algorithm
 * @leaf_der:    DER bytes of the leaf certificate
 * @leaf_der_len: length of @leaf_der
 * @msg:         signed message
 * @msg_len:     length of @msg
 * @sig:         signature bytes
 * @sig_len:     length of @sig
 */
TEE_Result pas_sig_verify_signature(uint32_t sig_algo, uint32_t hash_algo,
				    const uint8_t *leaf_der,
				    size_t leaf_der_len,
				    const uint8_t *msg, size_t msg_len,
				    const uint8_t *sig, size_t sig_len);

#endif /* __PAS_AUTH_SIG_H */
