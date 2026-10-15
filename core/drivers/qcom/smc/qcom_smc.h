/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Qualcomm OP-TEE secure-SMC forwarding interface
 *
 * This is a thin Secure EL1 -> EL3 forwarding layer. It does not implement
 * reset locally and does not perform reset through hardware directly.
 *
 * PSCI SMCs are architecturally reserved for Non-secure callers, so TF-A's
 * psci_smc_handler() rejects these requests from OP-TEE (Secure EL1).
 * Instead, TF-A exposes a secure-only SiP call (QTI_SIP_SVC_SECURE_RESET_ID,
 * see plat/qti/common/src/qti_syscall.c) that forwards to the same
 * qti_system_reset()/qti_system_reset2() backends used by the PSCI path.
 */

#ifndef QCOM_SMC_H
#define QCOM_SMC_H

#include <stdint.h>
#include <tee_api_types.h>

/*
 * QTI SiP secure-reset FID (Fast call, SMC64, OEN=SIP, NUM=0xD09). Only
 * accepted by TF-A when the caller is Secure world.
 */
#define QCOM_SIP_SVC_SECURE_RESET_AARCH64 \
	UINT32_C(0xC2000D09)

/*
 * Requests a plain system reset, forwarded via the SiP secure-reset call
 * with x1 = 0 (no reset_type/cookie).
 *
 * This function should not return when TF-A accepts the request and the
 * platform reset succeeds. A return value indicates that the request was
 * rejected or that reset unexpectedly returned.
 */
TEE_Result qcom_smc_system_reset(void);

/*
 * Requests a system reset with a QTI vendor-specific reset_type (warm/hard
 * reset config, cookie-store flag), forwarded via the SiP secure-reset
 * call. There is no architectural/vendor distinction on this path, unlike
 * PSCI SYSTEM_RESET2 - every request on this call is vendor-specific.
 *
 *   x1 = reset_type
 *   x2 = cookie
 *
 * This function should not return when TF-A accepts the request and the
 * platform reset succeeds.
 */
TEE_Result qcom_smc_system_reset2(uint32_t reset_type,
				  uint64_t cookie);

#endif /* QCOM_SMC_H */
