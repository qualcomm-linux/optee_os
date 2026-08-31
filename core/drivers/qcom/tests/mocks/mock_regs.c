/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "mock_regs.h"

#define MOCK_REGS_MAX_REGIONS	8
/* Wide enough for the RPMh RSC window (RPMH_RSC_SIZE == 0x40000) and the
 * CmdDb window (AOP_CMD_DB_SIZE == 0x20000) used by the mocked
 * platform_config.h; anything bigger is a test bug, not a real limit.
 */
#define MOCK_REGS_MAX_REGION_SIZE	(1u * 1024 * 1024)

struct mock_region {
	paddr_t pa;
	size_t size;
	uint8_t *va;
	int in_use;
};

/* A pending injected value takes priority over the region's backing byte,
 * for exactly one matching read32.
 */
struct mock_inject {
	vaddr_t addr;
	uint32_t val;
	int pending;
};

#define MOCK_REGS_MAX_INJECTS	8

static struct mock_region regions[MOCK_REGS_MAX_REGIONS];
static struct mock_inject injects[MOCK_REGS_MAX_INJECTS];

void mock_regs_reset(void)
{
	int i = 0;

	for (i = 0; i < MOCK_REGS_MAX_REGIONS; i++) {
		if (regions[i].in_use)
			free(regions[i].va);
	}
	memset(regions, 0, sizeof(regions));
	memset(injects, 0, sizeof(injects));
}

static struct mock_region *find_region_by_pa(paddr_t pa)
{
	int i = 0;

	for (i = 0; i < MOCK_REGS_MAX_REGIONS; i++) {
		if (regions[i].in_use && pa >= regions[i].pa &&
		    pa < regions[i].pa + regions[i].size)
			return &regions[i];
	}

	return NULL;
}

static struct mock_region *find_region_by_va(vaddr_t va)
{
	int i = 0;

	for (i = 0; i < MOCK_REGS_MAX_REGIONS; i++) {
		if (regions[i].in_use && va >= (vaddr_t)regions[i].va &&
		    va < (vaddr_t)regions[i].va + regions[i].size)
			return &regions[i];
	}

	return NULL;
}

vaddr_t mock_regs_add_region(paddr_t pa, size_t size)
{
	int i = 0;

	if (!size || size > MOCK_REGS_MAX_REGION_SIZE)
		return 0;

	for (i = 0; i < MOCK_REGS_MAX_REGIONS; i++) {
		if (!regions[i].in_use) {
			regions[i].va = calloc(1, size);
			if (!regions[i].va)
				return 0;
			regions[i].pa = pa;
			regions[i].size = size;
			regions[i].in_use = 1;
			return (vaddr_t)regions[i].va;
		}
	}

	return 0;
}

void *mock_regs_phys_to_virt(paddr_t pa, size_t len)
{
	struct mock_region *r = find_region_by_pa(pa);

	if (!r || pa + len > r->pa + r->size)
		return NULL;

	return r->va + (pa - r->pa);
}

paddr_t mock_regs_virt_to_phys(void *va)
{
	struct mock_region *r = find_region_by_va((vaddr_t)va);

	if (!r)
		return 0;

	return r->pa + ((vaddr_t)va - (vaddr_t)r->va);
}

uint32_t mock_regs_read32(vaddr_t addr)
{
	int i = 0;
	struct mock_region *r = NULL;

	for (i = 0; i < MOCK_REGS_MAX_INJECTS; i++) {
		if (injects[i].pending && injects[i].addr == addr) {
			injects[i].pending = 0;
			return injects[i].val;
		}
	}

	r = find_region_by_va(addr);
	MOCK_ASSERT_MSG(r != NULL, "io_read32() at unmapped address");

	return *(uint32_t *)addr;
}

void mock_regs_write32(vaddr_t addr, uint32_t val)
{
	struct mock_region *r = find_region_by_va(addr);

	MOCK_ASSERT_MSG(r != NULL, "io_write32() at unmapped address");

	*(uint32_t *)addr = val;
}

void mock_regs_inject_read32(vaddr_t addr, uint32_t val)
{
	int i = 0;

	for (i = 0; i < MOCK_REGS_MAX_INJECTS; i++) {
		if (!injects[i].pending) {
			injects[i].addr = addr;
			injects[i].val = val;
			injects[i].pending = 1;
			return;
		}
	}

	MOCK_ASSERT_MSG(0, "mock_regs_inject_read32() queue full");
}
