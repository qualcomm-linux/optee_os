/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Test-only mirror of the CmdDb on-disk layout that cmd_db.c keeps private
 * (struct db_header/entry_header/slv_id_info and the magic/version/slv-id
 * constants are all file-local to cmd_db.c, not exposed via cmd_db.h).
 * Building a real, correctly laid-out blob is the only way to exercise
 * cmd_db_init()/cmd_db_get_addr()/cmd_db_get_priority() and the
 * rpmh_resource_commands.c code that calls cmd_db_get_priority()
 * internally - mirroring the struct layout here relies on both this file
 * and cmd_db.c being compiled by the same host compiler/ABI, which holds
 * since both are part of the same test binary build.
 */
#ifndef __CMD_DB_FIXTURE_H
#define __CMD_DB_FIXTURE_H

#include <stdint.h>

#define FIXTURE_CMD_DB_MAGIC_NUM	0x0c0330db
#define FIXTURE_CMD_DB_VER		0x00000001
#define FIXTURE_CMD_DB_MAX_SLV_ID	8

/* Valid range per cmd_db.c's is_valid_slv_id(): ARC=3 .. BCM=5. */
#define FIXTURE_SLV_ID_ARC		3

struct fixture_entry_header {
	uint64_t res_id;
	uint32_t priority[2];
	uint32_t addr;
	uint16_t len;
	uint16_t offset;
};

struct fixture_slv_id_info {
	uint16_t slv_id;
	uint16_t header_offset;
	uint16_t data_offset;
	uint16_t cnt;
	uint16_t version;
	uint16_t reserved[3];
};

struct fixture_db_header {
	uint32_t version;
	uint32_t magic_num;
	struct fixture_slv_id_info slv_id_info[FIXTURE_CMD_DB_MAX_SLV_ID];
	uint32_t check_sum;
	uint32_t reserved;
	uint8_t data[];
};

/*
 * Maps AOP_CMD_DB_BASE/AOP_CMD_DB_SIZE via mock_regs_add_region(), zeroes
 * the whole region, and writes a valid version/magic (so cmd_db_init()
 * accepts it). All slv_id_info entries start with cnt = 0.
 */
struct fixture_db_header *cmd_db_fixture_install(void);

/*
 * Adds a single resource entry for @slv_id (FIXTURE_SLV_ID_ARC..+2), giving
 * that slv_id_info a header_offset/data_offset into the header's trailing
 * data[] area and cnt = 1. Only one call per @slv_id is supported (each
 * call claims a fresh region of data[]; calling twice for the same slv_id
 * would silently overwrite the first entry's header_offset/data_offset).
 * @aux_data/@aux_len are optional trailing payload bytes copyable via
 * cmd_db_get_*(); pass NULL/0 for entries with no aux data.
 */
void cmd_db_fixture_add_entry(struct fixture_db_header *hdr, uint16_t slv_id,
			      const char *res_id, uint32_t addr,
			      uint32_t priority0, uint32_t priority1,
			      const uint8_t *aux_data, uint16_t aux_len);

#endif /* __CMD_DB_FIXTURE_H */
