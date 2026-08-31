/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/initcall.h.
 *
 * The real early_init()/driver_init() etc. register a function pointer into
 * a SCATTERED_ARRAY linker-section table that only exists in a linked OP-TEE
 * core image, and call_early_initcalls() walks that table at boot. Tests
 * call rpmh_client_init()/cmd_db_init() directly instead of relying on
 * auto-init, so these macros only need to keep the driver source
 * compiling; the registered function pointer is discarded.
 *
 * Some driver .c files rely on the real initcall.h's transitive #include
 * of trace.h to get EMSG()/etc. without including trace.h themselves -
 * keep that same transitive availability here.
 *
 * early_init(fn) etc. also stash a non-static function pointer named
 * "<fn>_early_init_hook" (and per-macro variants) at file scope, pointing
 * at the otherwise-static @fn. Driver init functions like cmd_db_init()/
 * rpmh_client_init() are file-local statics only reachable in production
 * via the real initcall machinery, so tests that need to actually run one
 * (e.g. to populate cmd_db's query_db.data from a fixture) declare
 * "extern TEE_Result (*fn_early_init_hook)(void);" and call it directly.
 */
#ifndef __INITCALL_H
#define __INITCALL_H

#include <trace.h>
#include <tee_api_types.h>

#define early_init(fn)		TEE_Result (*fn##_early_init_hook)(void) = fn
#define early_init_late(fn)	TEE_Result (*fn##_early_init_late_hook)(void) = fn
#define driver_init(fn)		TEE_Result (*fn##_driver_init_hook)(void) = fn
#define driver_init_late(fn)	TEE_Result (*fn##_driver_init_late_hook)(void) = fn
#define service_init(fn)	TEE_Result (*fn##_service_init_hook)(void) = fn

#endif /* __INITCALL_H */
