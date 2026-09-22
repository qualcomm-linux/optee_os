/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __DRIVERS_QCOM_IPCC_H
#define __DRIVERS_QCOM_IPCC_H

#include <stdint.h>
#include <tee_api_types.h>

/*
 * IPCC (Inter-Processor Communication Controller) driver.
 *
 * The controller carries signals between the processors of a SoC. A signal is
 * the triple (protocol, client, signal number): the protocol groups clients
 * that talk to each other, a client is a processor or a hardware block, and
 * the signal number is a doorbell within that pair.
 *
 * A chipset instantiates only a subset of the protocols and clients below,
 * described by a per-chipset table. Everything not described is rejected, so a
 * caller can never signal a client this TEE was not configured to reach.
 *
 * Two mechanisms carry a signal and may coexist on one protocol: router mode
 * multiplexes every sender behind one interrupt per protocol, direct mode
 * gives a (client, signal) pair its own interrupt line and trigger register.
 * The choice is per client and is made by the chipset table; callers see the
 * same behaviour either way.
 *
 * Where a chipset has no IPCC these entry points come from ipcc_stub.c and
 * every one of them fails with TEE_ERROR_NOT_SUPPORTED.
 */

/*
 * Protocol enumeration based on the IPCC specification. A given SoC only
 * instantiates a subset of these; per-chipset config tables describe which.
 */
enum ipcc_protocol {
	IPCC_PROTO_MPROC = 0,
	IPCC_PROTO_COMPUTEL0 = 1,
	IPCC_PROTO_COMPUTEL1 = 2,
	IPCC_PROTO_PERIPH = 3,
	IPCC_PROTO_FENCE = 4,

	IPCC_PROTO_TOTAL,
};

/*
 * Client enumeration based on the IPCC specification. Numeric values mirror
 * IPCCDefs.h exactly so that a value received from remote processors (or
 * ported chipset config tables) needs no translation. The range is sparse,
 * so IPCC_CLIENT_TOTAL is an upper bound and not a client count.
 */
enum ipcc_client {
	IPCC_CLIENT_AOP = 0,
	IPCC_CLIENT_TZ = 1,
	IPCC_CLIENT_MPSS = 2,
	IPCC_CLIENT_LPASS = 3,
	IPCC_CLIENT_SLPI = 4,
	IPCC_CLIENT_SDC = 5,
	IPCC_CLIENT_NSP0 = 6,
	IPCC_CLIENT_NPU = 7,
	IPCC_CLIENT_APPS = 8,
	IPCC_CLIENT_GPU = 9,
	IPCC_CLIENT_CVP = 10,
	IPCC_CLIENT_CAM = 11,
	IPCC_CLIENT_VPU = 12,
	IPCC_CLIENT_PCIE0 = 13,
	IPCC_CLIENT_PCIE1 = 14,
	IPCC_CLIENT_PCIE2 = 15,
	IPCC_CLIENT_SPSS = 16,
	IPCC_CLIENT_SMSS = 17,
	IPCC_CLIENT_NSP1 = 18,
	IPCC_CLIENT_PCIE3 = 19,
	IPCC_CLIENT_PCIE4 = 20,
	IPCC_CLIENT_PCIE5 = 21,
	IPCC_CLIENT_PCIE6 = 22,
	IPCC_CLIENT_TMESS = 23,
	IPCC_CLIENT_WPSS = 24,
	IPCC_CLIENT_DPU = 25,
	IPCC_CLIENT_IPA = 26,
	IPCC_CLIENT_SAIL0 = 27,
	IPCC_CLIENT_SAIL1 = 28,
	IPCC_CLIENT_SAIL2 = 29,
	IPCC_CLIENT_SAIL3 = 30,
	IPCC_CLIENT_GPDSP0 = 31,
	IPCC_CLIENT_GPDSP1 = 32,
	IPCC_CLIENT_APSS_NS1 = 33,
	IPCC_CLIENT_APSS_NS2 = 34,
	IPCC_CLIENT_APSS_NS3 = 35,
	IPCC_CLIENT_APSS_NS4 = 36,
	IPCC_CLIENT_APSS_NS5 = 37,
	IPCC_CLIENT_APSS_NS6 = 38,
	IPCC_CLIENT_APSS_NS7 = 39,
	IPCC_CLIENT_TENX = 40,
	IPCC_CLIENT_ORAN = 41,
	IPCC_CLIENT_MVMSS = 42,
	IPCC_CLIENT_DPU1 = 43,
	IPCC_CLIENT_PCIE7 = 44,
	IPCC_CLIENT_DBG = 45,
	IPCC_CLIENT_SOCCP = 46,
	IPCC_CLIENT_ICP1 = 47,
	IPCC_CLIENT_NSP2 = 48,
	IPCC_CLIENT_NSP3 = 49,
	IPCC_CLIENT_SAIL4 = 50,
	IPCC_CLIENT_SAIL5 = 51,
	IPCC_CLIENT_CPUCP = 52,
	IPCC_CLIENT_A78CSS = 53,
	IPCC_CLIENT_GPU1 = 54,
	IPCC_CLIENT_OOBSS = 55,
	IPCC_CLIENT_OOBSS_S = 56,
	IPCC_CLIENT_DCP = 57,
	IPCC_CLIENT_PDP0 = 58,
	IPCC_CLIENT_PDP1 = 59,
	IPCC_CLIENT_PDP2 = 60,
	IPCC_CLIENT_PDP3 = 61,
	IPCC_CLIENT_M55_WM = 62,
	IPCC_CLIENT_LSR = 63,
	IPCC_CLIENT_QECP = 64,
	IPCC_CLIENT_QUP_TOP0 = 66,
	IPCC_CLIENT_QUP_TOP1 = 67,
	IPCC_CLIENT_QUP_TOP2 = 68,
	IPCC_CLIENT_QUP_TOP3 = 69,
	IPCC_CLIENT_QUP_SSC0 = 70,
	IPCC_CLIENT_QUP_SSC1 = 71,
	IPCC_CLIENT_QUP_TOP4 = 84,
	IPCC_CLIENT_IFE0 = 128,
	IPCC_CLIENT_IFE1 = 129,
	IPCC_CLIENT_IFE2 = 130,
	IPCC_CLIENT_IFE3 = 131,
	IPCC_CLIENT_IFE4 = 132,
	IPCC_CLIENT_IFE5 = 133,
	IPCC_CLIENT_IFE6 = 134,
	IPCC_CLIENT_IFE7 = 135,
	IPCC_CLIENT_IFE8 = 136,
	IPCC_CLIENT_IFE9 = 137,
	IPCC_CLIENT_IFE10 = 138,
	IPCC_CLIENT_IFE11 = 139,
	IPCC_CLIENT_IFE12 = 140,

	IPCC_CLIENT_TOTAL,

	/* Alternate names for the same clients; both spellings are in use */
	IPCC_CLIENT_RPM = IPCC_CLIENT_AOP,
	IPCC_CLIENT_APSS_S = IPCC_CLIENT_TZ,
	IPCC_CLIENT_ADSP0 = IPCC_CLIENT_LPASS,
	IPCC_CLIENT_CDSP = IPCC_CLIENT_NSP0,
	IPCC_CLIENT_APSS_NS0 = IPCC_CLIENT_APPS,
	IPCC_CLIENT_GPU0 = IPCC_CLIENT_GPU,
	IPCC_CLIENT_ICP0 = IPCC_CLIENT_CAM,
	IPCC_CLIENT_DPU0 = IPCC_CLIENT_DPU,
	IPCC_CLIENT_ADSP1 = IPCC_CLIENT_GPDSP0,
	IPCC_CLIENT_ADSP2 = IPCC_CLIENT_GPDSP1,
	IPCC_CLIENT_DUMMY = IPCC_CLIENT_DBG,
	IPCC_CLIENT_APCP = IPCC_CLIENT_CPUCP,
	IPCC_CLIENT_PRIME_CORE = IPCC_CLIENT_M55_WM,
	IPCC_CLIENT_LMCU = IPCC_CLIENT_M55_WM,
	IPCC_CLIENT_M55_AM = IPCC_CLIENT_LSR,
	IPCC_CLIENT_CAM_ENG0 = IPCC_CLIENT_IFE0,
	IPCC_CLIENT_CAM_ENG1 = IPCC_CLIENT_IFE1,
	IPCC_CLIENT_CAM_ENG2 = IPCC_CLIENT_IFE2,
	IPCC_CLIENT_CAM_ENG3 = IPCC_CLIENT_IFE3,
	IPCC_CLIENT_CAM_ENG4 = IPCC_CLIENT_IFE4,
	IPCC_CLIENT_CAM_ENG5 = IPCC_CLIENT_IFE5,
	IPCC_CLIENT_CAM_ENG6 = IPCC_CLIENT_IFE6,
	IPCC_CLIENT_CAM_ENG7 = IPCC_CLIENT_IFE7,
	IPCC_CLIENT_CAM_ENG8 = IPCC_CLIENT_IFE8,
	IPCC_CLIENT_CAM_ENG9 = IPCC_CLIENT_IFE9,
	IPCC_CLIENT_CAM_ENG10 = IPCC_CLIENT_IFE10,
	IPCC_CLIENT_CAM_ENG11 = IPCC_CLIENT_IFE11,
	IPCC_CLIENT_CAM_ENG12 = IPCC_CLIENT_IFE12,
};

/* Opaque handle returned by ipcc_attach() */
struct ipcc_handle;

/*
 * Callback invoked from interrupt context when a registered signal is
 * received. It must not sleep and must not call back into this API.
 */
typedef void (*ipcc_callback_t)(void *cb_data, enum ipcc_client sender_id,
				uint16_t signal);

/*
 * ipcc_attach() - Attach to an IPCC protocol
 * @h:		Output handle, to be used for subsequent Rx/Tx calls
 * @protocol:	Protocol to attach to
 *
 * Multiple callers may attach to the same protocol; the protocol's shared Rx
 * interrupt is registered on first attach only.
 *
 * Return TEE_SUCCESS on success, a TEE_Result error code otherwise.
 */
TEE_Result ipcc_attach(struct ipcc_handle **h, enum ipcc_protocol protocol);

/*
 * ipcc_detach() - Detach a handle obtained from ipcc_attach()
 * @h:		Handle to release, set to NULL on success
 *
 * Signal ranges still registered on the handle are released with it.
 *
 * Return TEE_SUCCESS on success, a TEE_Result error code otherwise.
 */
TEE_Result ipcc_detach(struct ipcc_handle **h);

/*
 * ipcc_register_interrupt() - Register a callback for a range of signals
 * @h:			Handle from ipcc_attach()
 * @sender_id:		Client the signals are expected from
 * @signal_low:		First signal number of the range, inclusive
 * @signal_high:	Last signal number of the range, inclusive
 * @cb:			Callback, invoked in interrupt context
 * @cb_data:		Opaque value passed back to @cb
 *
 * Ranges registered for one sender must not overlap.
 *
 * Return TEE_SUCCESS on success, a TEE_Result error code otherwise.
 */
TEE_Result ipcc_register_interrupt(struct ipcc_handle *h,
				   enum ipcc_client sender_id,
				   uint16_t signal_low, uint16_t signal_high,
				   ipcc_callback_t cb, void *cb_data);

/*
 * ipcc_deregister_interrupt() - Release a registered signal range
 * @h:			Handle from ipcc_attach()
 * @sender_id:		Client the range was registered for
 * @signal_low:		First signal number of the range, inclusive
 * @signal_high:	Last signal number of the range, inclusive
 *
 * The range must match a range passed to ipcc_register_interrupt() exactly.
 *
 * Return TEE_SUCCESS on success, a TEE_Result error code otherwise.
 */
TEE_Result ipcc_deregister_interrupt(struct ipcc_handle *h,
				     enum ipcc_client sender_id,
				     uint16_t signal_low,
				     uint16_t signal_high);

/*
 * ipcc_trigger() - Raise a range of signals towards a client
 * @h:			Handle from ipcc_attach()
 * @target_id:		Client to signal
 * @signal_low:		First signal number of the range, inclusive
 * @signal_high:	Last signal number of the range, inclusive
 *
 * Return TEE_SUCCESS on success, a TEE_Result error code otherwise.
 */
TEE_Result ipcc_trigger(struct ipcc_handle *h, enum ipcc_client target_id,
			uint16_t signal_low, uint16_t signal_high);

#endif /* __DRIVERS_QCOM_IPCC_H */
