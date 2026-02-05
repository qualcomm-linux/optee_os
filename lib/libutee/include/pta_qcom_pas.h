/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __PTA_QCOM_REMOTEPROC_H
#define __PTA_QCOM_REMOTEPROC_H

#include <stdint.h>
#include <util.h>

/*
 * Interface to the pseudo TA which provides platform implementation
 * of the remote processor management
 */

#define PTA_QCOM_PAS_UUID { 0xcff7d191, 0x7ca0, 0x4784, \
		{ 0xaf, 0x13, 0x48, 0x22, 0x3b, 0x9a, 0x4f, 0xbe} }

/*
 * Peripheral Authentication Service (PAS) supported.
 *
 * Get PAS support status.
 *
 * [in]  params[0].value.a:	Unique 32bit remote processor identifier
 */
#define PTA_QCOM_PAS_IS_SUPPORTED		1

/*
 * PAS capabilities.
 *
 * Get PAS capabilities.
 *
 * [in]  params[0].value.a:	Unique 32bit remote processor identifier
 * [out] params[1].value.a:	PAS capability flags
 */
#define PTA_QCOM_PAS_CAPABILITIES		2

/*
 * PAS image initialization.
 *
 * Perform PAS image initialization.
 *
 * [in]  params[0].value.a:	Unique 32bit remote processor identifier
 * [in]  params[1].memref:	Loadable firmware metadata
 */
#define PTA_QCOM_PAS_INIT_IMAGE			3

/*
 * PAS memory setup.
 *
 * Perform PAS memory setup.
 *
 * [in]  params[0].value.a:	Unique 32bit remote processor identifier
 * [in]  params[0].value.b:	Relocatable firmware size
 * [in]  params[1].value.a:	32bit LSB relocatable firmware memory address
 * [in]  params[1].value.b:	32bit MSB relocatable firmware memory address
 */
#define PTA_QCOM_PAS_MEM_SETUP			4

/*
 * PAS get resource table.
 *
 * Perform PAS image initialization.
 *
 * [in]     params[0].value.a:	Unique 32bit remote processor identifier
 * [in/out] params[1].memref:	Resource table config
 */
#define PTA_QCOM_PAS_GET_RESOURCE_TABLE		5

/*
 * PAS image authentication and co-processor reset.
 *
 * Perform PAS image authentication and co-processor reset.
 *
 * [in]  params[0].value.a:	Unique 32bit remote processor identifier
 * [in]  params[0].value.b:	Firmware size
 * [in]  params[1].value.a:	32bit LSB firmware memory address
 * [in]  params[1].value.b:	32bit MSB firmware memory address
 * [in]  params[2].memref:	Optional fw memory space shared/lent
 */
#define PTA_QCOM_PAS_AUTH_AND_RESET		6

/*
 * PAS co-processor set suspend/resume state.
 *
 * Perform PAS co-processor set suspend/resume state.
 *
 * [in]  params[0].value.a:	Unique 32bit remote processor identifier
 * [in]  params[0].value.b:	Co-processor state identifier
 */
#define PTA_QCOM_PAS_SET_REMOTE_STATE		7

/*
 * PAS co-processor shutdown.
 *
 * Perform PAS co-processor shutdown.
 *
 * [in]  params[0].value.a:	Unique 32bit remote processor identifier
 */
#define PTA_QCOM_PAS_SHUTDOWN			8

#endif /* __PTA_QCOM_REMOTEPROC_H */
