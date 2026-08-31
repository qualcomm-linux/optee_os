/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#include <string.h>
#include <tee_api_types.h>

#include "cmd_db_fixture.h"
#include "harness.h"
#include "mocks/mock_regs.h"
#include "platform_config.h"

/*
 * cmd_db_init() is a static function in cmd_db.c, only reachable in
 * production via the real early_init() initcall machinery. The mocked
 * early_init() (mocks/initcall.h) exposes it as this function-pointer
 * hook instead, so the fixture can run real init logic (magic/version
 * check, query_db.data = phys_to_virt(...)) against the fake blob below
 * rather than duplicating that logic here.
 */
extern TEE_Result (*cmd_db_init_early_init_hook)(void);

/* Next free byte offset into hdr->data[] for the next add_entry() call;
 * reset on every cmd_db_fixture_install().
 */
static uint16_t next_data_offset;

struct fixture_db_header *cmd_db_fixture_install(void)
{
	vaddr_t va = mock_regs_add_region(AOP_CMD_DB_BASE, AOP_CMD_DB_SIZE);
	struct fixture_db_header *hdr = (struct fixture_db_header *)va;

	TEST_ASSERT_MSG(va != 0, "failed to allocate fake CmdDb region");

	hdr->version = FIXTURE_CMD_DB_VER;
	hdr->magic_num = FIXTURE_CMD_DB_MAGIC_NUM;

	next_data_offset = 0;

	TEST_ASSERT_EQ_INT(cmd_db_init_early_init_hook(), TEE_SUCCESS);

	return hdr;
}

void cmd_db_fixture_add_entry(struct fixture_db_header *hdr, uint16_t slv_id,
			      const char *res_id, uint32_t addr,
			      uint32_t priority0, uint32_t priority1,
			      const uint8_t *aux_data, uint16_t aux_len)
{
	struct fixture_entry_header entry = { 0 };
	struct fixture_slv_id_info *slot = &hdr->slv_id_info[0];
	size_t i = 0;

	for (i = 0; i < FIXTURE_CMD_DB_MAX_SLV_ID; i++) {
		if (hdr->slv_id_info[i].slv_id == slv_id ||
		    hdr->slv_id_info[i].cnt == 0) {
			slot = &hdr->slv_id_info[i];
			break;
		}
	}

	slot->slv_id = slv_id;
	slot->version = 1;
	slot->header_offset = next_data_offset;
	next_data_offset += sizeof(struct fixture_entry_header);
	slot->data_offset = next_data_offset;
	next_data_offset += aux_len;
	slot->cnt = 1;

	entry.res_id = 0;
	if (res_id) {
		size_t n = strlen(res_id);
		size_t j = 0;

		for (j = 0; j < n && j < sizeof(entry.res_id); j++)
			entry.res_id |= ((uint64_t)(uint8_t)res_id[j]) << (8 * j);
	}
	entry.priority[0] = priority0;
	entry.priority[1] = priority1;
	entry.addr = addr;
	entry.len = aux_len;
	entry.offset = 0;

	memcpy(&hdr->data[slot->header_offset], &entry, sizeof(entry));

	if (aux_data && aux_len)
		memcpy(&hdr->data[slot->data_offset], aux_data, aux_len);
}
