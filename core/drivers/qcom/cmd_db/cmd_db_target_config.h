/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __CMD_DB_TARGET_CONFIG_H__
#define __CMD_DB_TARGET_CONFIG_H__

#include <platform_config.h>
#include <stdint.h>
#include <types_ext.h>

#define AOP_MSG_RAM_END (AOP_MSG_RAM_BASE + AOP_MSG_RAM_SIZE)

#define CMD_DB_METADATA_OFFSET		0x10000
#define CMD_DB_ADDR_FIELD_OFFSET	0xC

#define AOP_METADATA_ADDR (AOP_MSG_RAM_END - CMD_DB_METADATA_OFFSET + \
			   CMD_DB_ADDR_FIELD_OFFSET)

struct cmd_db_target_config {
	paddr_t aop_metadata_addr;
	size_t aop_metadata_size;
};

struct aop_cmd_db_metadata {
	uint32_t addr;
	uint32_t size;
};

static inline struct cmd_db_target_config cmd_db_get_target_config(void)
{
	struct cmd_db_target_config cfg = {
		.aop_metadata_addr = AOP_METADATA_ADDR,
		.aop_metadata_size = sizeof(struct aop_cmd_db_metadata),
	};
	return cfg;
}

#endif /* __CMD_DB_TARGET_CONFIG_H__ */
