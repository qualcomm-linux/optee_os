/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __RPMH_CLIENT_H__
#define __RPMH_CLIENT_H__

#include <stdint.h>
#include <stdbool.h>
#include <tee_api_types.h>

#define RPMH_MAX_TCS_SIZE    16

enum rpmh_set_enum {
	RPMH_SET_ACTIVE = 0,
	RPMH_SET_SLEEP = 1,
	RPMH_SET_WAKE = 2,
	RPMH_NUM_SETS = 3,
};

enum rscsw_drv_mapping {
	RSC_DRV_SECURE        = 0,
	RSC_DRV_CPUCP         = 1,
	RSC_DRV_L3            = RSC_DRV_CPUCP,
	RSC_DRV_HLOS          = 2,
	RSC_DRV_HYP           = 3,
	RSC_DRV_MAX,

	/* Virtual DRVs */
	RSC_DRV_VIRTUAL_DRVS = 0x3FFFFF00,
	RSC_DRV_VIRTUAL_SENSORS,
	RSC_DRV_VIRTUAL_MAX = 0x3FFFFFFF,
};

struct rpmh_command {
	uint32_t address;
	uint32_t data;
	bool completion;
};

struct rpmh_client;

/* Create RPMH client handle */
struct rpmh_client *rpmh_create_handle(enum rscsw_drv_mapping drv_id,
				       const char *name);

/* Issue command to RPMH resource */
TEE_Result rpmh_issue_command(struct rpmh_client *handle,
			      enum rpmh_set_enum set,
			      bool completion, uint32_t address,
			      uint32_t data, uint32_t *req_id);

/* Wait for command completion */
void rpmh_barrier_single(struct rpmh_client *handle, uint32_t req_id);

/* Check if driver ID is valid */
bool rpmh_is_drv_id_valid(enum rscsw_drv_mapping drv_id);

#endif /* __RPMH_CLIENT_H__ */
