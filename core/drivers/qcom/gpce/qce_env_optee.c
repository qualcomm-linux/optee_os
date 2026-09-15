// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * Qualcomm Crypto Engine (QCE) environment layer for OP-TEE.
 *
 * Provides the platform adaptation required by the GPCE BAM
 * driver stack, including memory mapping, buffer management,
 * address translation, cache maintenance, and driver context
 * services.
 */

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include "qce_env.h"
#include "ce_hal.h"
#include "qce_hwio_regs.h"
#include <mm/core_memprot.h>
#include <string.h>

#define QCE_BAM_DESC_FIFO_SIZE   304   /* 38 × 8-byte descriptors */
#define QCE_BAM_RSLT_BUF_SIZE    256   /* result dump buffer */
#define QCE_BAM_CMD_BUF_SIZE     4096  /* CE command descriptor buffer — must be >= sizeof(cmd_reg_t) + sizeof(cmd_ce_5xx_reg_t) */
#define QCE_BAM_CONTAINER_SIZE   256   /* BAM container buffer (mandatory) */
#define QCE_BAM_TMP_OUT_SIZE     16    /* BAM tmp output buffer (mandatory) */

static uint8_t qce_tx_desc[QCE_BAM_DESC_FIFO_SIZE] __aligned(8);
static uint8_t qce_rx_desc[QCE_BAM_DESC_FIFO_SIZE] __aligned(8);
static uint8_t qce_rslt[QCE_BAM_RSLT_BUF_SIZE]     __aligned(128);
static uint8_t qce_cmd_buf[QCE_BAM_CMD_BUF_SIZE]    __aligned(8);
static uint8_t qce_container[QCE_BAM_CONTAINER_SIZE] __aligned(128);
static uint8_t qce_tmp_out[QCE_BAM_TMP_OUT_SIZE]    __aligned(16);

void *g_bam_drv_ctx;

static vaddr_t ce_va;
static vaddr_t ce_bam_va;

static paddr_t ce_pa = CRYPTO0_CRYPTO_REG_BASE;
static paddr_t ce_bam_pa = CRYPTO0_CRYPTO_BAM_REG_BASE;

env_t *ucenv_get_env(void)
{
  return (env_t *)0xff;
}

TEE_Result ucenv_optee_init(void)
{
    static bool initialized = false;

    if (initialized)
      return TEE_SUCCESS;

	ce_va = (vaddr_t)core_mmu_add_mapping(MEM_AREA_IO_SEC,
				      ce_pa,
				      CRYPTO0_CRYPTO_REG_BASE_SIZE);
	if (!ce_va){
        EMSG("GPCE: failed to map CE");
		return -1;
    }

	ce_bam_va = (vaddr_t)core_mmu_add_mapping(MEM_AREA_IO_SEC,
					  ce_bam_pa,
					  CRYPTO0_CRYPTO_BAM_REG_BASE_SIZE);
	if (!ce_bam_va){
        EMSG("GPCE: failed to map BAM");
		return -1;
    }

    initialized = true;

	return TEE_SUCCESS;
}

vaddr_t ce_hwio_phys_to_virt(uintptr_t phys_addr)
{
	if ((phys_addr >= (uintptr_t)ce_pa) &&
	    (phys_addr < ((uintptr_t)ce_pa + CRYPTO0_CRYPTO_REG_BASE_SIZE))) {
		return ce_va + (phys_addr - (uintptr_t)ce_pa);
	}

	return 0;
}

void *ENV_zalloc(env_t *env, size_t size)
{
    (void)env;
    void *ptr = malloc(size);
    if (ptr == NULL)
        return NULL;
    memset(ptr, 0, size);
    return ptr;
}

void ENV_mem_free(env_t *env, void *ptr)
{
    (void)env;
    free(ptr);
}

void ENV_mem_clear(env_t *env, void *ptr, size_t size)
{
    (void)env;
    memset(ptr, 0, size);
}

size_t ENV_sec_memcpy(env_t *env, void *dst, size_t dst_sz,
                      const void *src, size_t src_sz)
{
    size_t n;

    (void)env;

    if (!dst || !src)
        return 0;

    n = src_sz < dst_sz ? src_sz : dst_sz;
    memcpy(dst, src, n);

    return n;
}

uintptr_t ENV_HW_base(env_t *env, env_base_addr_var_t var)
{
    (void)env;

    switch (var) {
    case ENV_CE_BAM_BASE:
        return ce_bam_va;
    default:
        return 0;
    }
}

uint8_t *ENV_get_ce_bam_buf_addr(env_t *env, env_bam_var_t var)
{
    (void)env;

    switch (var) {
    case ENV_BAM_TX_DESC:      return qce_tx_desc;
    case ENV_BAM_RX_DESC:      return qce_rx_desc;
    case ENV_BAM_RSLT:         return qce_rslt;
    case ENV_BAM_CONTAINER:    return qce_container;
    case ENV_BAM_CMD_DESC_BUF: return qce_cmd_buf;
    case ENV_BAM_TMP_OUT_BUF:  return qce_tmp_out;
    default:                   return NULL;
    }
}

bool ENV_is_bam_polling_mode(env_t *env)
{
    (void)env;
    return true;
}

size_t ENV_get_maxBamDataSize(env_t *env)
{
    (void)env;
    return CE_BAM_MAX_XFER_SIZE * ((QCE_BAM_DESC_FIFO_SIZE / 8) - 6);
}

int ENV_ce_clk_enable(env_t *env)
{
    (void)env;
    return UCLIB_SUCCESS;
}

int ENV_ce_clk_disable(env_t *env)
{
    (void)env;
    return UCLIB_SUCCESS;
}

int ENV_vtop(env_t *env, uintptr_t va, uintptr_t *pa)
{
    (void)env;

    if (!pa)
        return UCLIB_ERR_INVALID_ARG;

    *pa = (uintptr_t)virt_to_phys((void *)va);
    return UCLIB_SUCCESS;
}

void ENV_dcache_clean_and_invalidate_region(env_t *env, void *addr, size_t length)
{
    (void)env;

    if (addr && length)
        cache_op_inner(DCACHE_AREA_CLEAN_INV, addr, length);
}

size_t ENV_dcache_line_size(env_t *env)
{
    (void)env;
    return 64;
}

void ENV_mem_barrier(env_t *env)
{
    (void)env;
    dsb();
}

int ENV_mutex_lock(env_t *env)
{
    (void)env;
    return UCLIB_SUCCESS;
}

int ENV_mutex_release(env_t *env)
{
    (void)env;
    return UCLIB_SUCCESS;
}

void *ENV_get_driver_ctx(env_t *env, env_var_op operation,
                         env_init_data_var_t var, void *data)
{
    void **ctx;

    (void)env;

    if (var != ENV_INIT_BAM_DRV_CTX)
        return NULL;

    ctx = &g_bam_drv_ctx;

    if (operation == SET)
        *ctx = data;

    return *ctx;
}

size_t ENV_get_ce_bam_buf_size(env_t *env, env_bam_var_t var)
{
    (void)env;

    switch (var) {
    case ENV_BAM_TX_DESC:
    case ENV_BAM_RX_DESC:
        return QCE_BAM_DESC_FIFO_SIZE;

    case ENV_BAM_CMD_DESC_BUF:
        return QCE_BAM_CMD_BUF_SIZE;

    default:
        return 0;
    }
}

int ENV_install_hw_key(env_t *env, env_hwkm_key_info_t *key_info)
{
    (void)env;

    if (!key_info)
        return UCLIB_ERR_INVALID_ARG;

    return UCLIB_ERR_NOT_SUPPORTED;
}

bool ENV_is_use_550_features(env_t *env)
{
    (void)env;
    return true;
}

bool ENV_cmds_over_hwio(env_t *env)
{
    (void)env;
    return true;
}

uint8_t ENV_get_default_pipeset_num(env_t *env)
{
    (void)env;
    return 0;
}

int ENV_event_create(env_t *env)
{
    (void)env;
    return UCLIB_SUCCESS;
}

int ENV_event_wait(env_t *env)
{
    (void)env;
    return UCLIB_SUCCESS;
}

int ENV_event_trigger(env_t *env)
{
    (void)env;
    return UCLIB_SUCCESS;
}

void ENV_reg_dump_init(env_t *env, void *bam_handle)
{
    (void)env;
    (void)bam_handle;
}

size_t ENV_BAM_get_max_poll_count(env_t *env)
{
    (void)env;
    return 0x4000;
}

size_t ENV_BAM_get_max_rlst_dump_cnt(env_t *env)
{
    (void)env;
    return 0;
}

uint32_t ENV_BAM_get_ce_bam_pipe_set_cnt(env_t *env)
{
    (void)env;
    return 1;
}
