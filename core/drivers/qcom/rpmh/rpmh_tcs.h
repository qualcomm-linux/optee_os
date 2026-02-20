/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __RPMH_TCS_H__
#define __RPMH_TCS_H__

#include <stdint.h>
#include <stdbool.h>
#include <drivers/qcom/rpmh/rpmh_client.h>

#define TCS_TIMEOUT_THRESHOLD 0x2EE00

#define RPMH_SLAVE_ID(address)  ((address) >> 16)
#define RPMH_OFFSET(address)    ((address) & 0xFFFF)

struct rpmh_cmd_details {
	uint32_t address;
	uint32_t data;
	bool completion;
};

struct rpmh_cmd {
	struct rpmh_cmd_details *details;
	uint32_t num_rcs;
	uint32_t dependency_bmsk;
	struct rpmh_client *client;
	uint32_t req_id;
	enum rpmh_set_enum set;
	bool amc;
	struct rpmh_cmd *next;
};

enum tcs_state {
	TCS_AMC_IDLE = 0,
	TCS_AMC_WAIT_FOR_DONE = 1,
	TCS_NON_AMC = 2,
	TCS_NUM_STATES = 3
};

struct tcs {
	uint8_t id;
	enum tcs_state state;
	uint64_t time_last_sent;
	struct rpmh_cmd *cmds;
};

struct rpmh_client;
struct rpmh_cmdq;

/* Initialize TCS management */
TEE_Result rpmh_tcs_init(void);

/* Check if driver ID is valid */
bool rpmh_tcs_drv_valid(enum rscsw_drv_mapping drv_id);

/* Get driver configuration index */
TEE_Result rpmh_tcs_find_drv_index(enum rscsw_drv_mapping drv_id,
				   uint32_t *drv_index);

/* Get number of commands per TCS */
TEE_Result rpmh_tcs_size(enum rscsw_drv_mapping drv_id, uint32_t *size);

/* Check if AMC is available */
bool rpmh_tcs_is_amc_free(enum rscsw_drv_mapping drv_id);

/* Send command via AMC */
TEE_Result rpmh_tcs_send(struct rpmh_cmd *cmd, enum rscsw_drv_mapping drv_id);

/* Check if AMC finished */
bool rpmh_tcs_is_amc_finished(enum rscsw_drv_mapping drv_id);

/* Clean up finished AMC */
TEE_Result rpmh_tcs_finish_active_amc(struct rpmh_client **client,
				      struct rpmh_cmdq *cmdq,
				      enum rscsw_drv_mapping drv_id,
				      uint32_t *req_id);

/* Check TCS slot availability */
TEE_Result rpmh_tcs_slots_available(enum rscsw_drv_mapping drv_id,
				    enum rpmh_set_enum set,
				    uint32_t slots,
				    uint32_t *tcs_index);

/* Check if TCS is stuck */
bool rpmh_tcs_is_stuck(enum rscsw_drv_mapping drv_id);

/* Get finished driver ID */
TEE_Result rpmh_tcs_get_finished_drv(enum rscsw_drv_mapping hw_drv,
				     enum rscsw_drv_mapping *finished_drv);

#endif /* __RPMH_TCS_H__ */
