// core/drivers/qcom/smc/qcom_smc.c

// SPDX-License-Identifier: BSD-2-Clause

#include "qcom_smc.h"
#include <kernel/thread.h>
#include <kernel/thread_arch.h>
#include <string.h>
#include <trace.h>

static TEE_Result qcom_smc_forward_reset(struct thread_smc_args *args)
{
	/*
	 * A successful reset should not return. If execution reaches this
	 * point, TF-A either rejected the request or the platform reset
	 * returned unexpectedly.
	 */
	thread_smccc(args);

	EMSG("Secure reset unexpectedly returned: fid=0x%lx status=0x%lx",
	     args->a0, args->a0);

	return TEE_ERROR_GENERIC;
}

TEE_Result qcom_smc_system_reset(void)
{
	struct thread_smc_args args;

	memset(&args, 0, sizeof(args));
	args.a0 = QCOM_SIP_SVC_SECURE_RESET_AARCH64;
	/* x1 == 0 selects the plain reset path in TF-A */
	args.a1 = 0;

	return qcom_smc_forward_reset(&args);
}

TEE_Result qcom_smc_system_reset2(uint32_t reset_type,
				  uint64_t cookie)
{
	struct thread_smc_args args;

	memset(&args, 0, sizeof(args));
	args.a0 = QCOM_SIP_SVC_SECURE_RESET_AARCH64;
	args.a1 = reset_type;
	args.a2 = cookie;

	return qcom_smc_forward_reset(&args);
}
