// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE environment interface.
 *
 * Defines the platform abstraction layer used by the GPCE BAM
 * driver stack for memory management, address translation,
 * cache maintenance, synchronization, and platform services.
 */
#ifndef __UCLIB_PRV_ENV
#define __UCLIB_PRV_ENV

#include <stdlib.h>
#include <trace.h>
#include <io.h>


/* Convert endianness macros */
#define REVERSE_ENDIAN(c)                ((((c)>>24)&0xff)    | \
                                         (((c)<<8)&0xff0000)  | \
                                         (((c)>>8)&0xff00)    | \
                                         (((c)<<24)&0xff000000))

#define BIG2LITTLE(c)                    REVERSE_ENDIAN(c)

#define CAST_PTR_TYPE(ptr, type)         (type *)(ptr)

#define IS_SIZE_ALIGNED(ptr, sz)         (!((uintptr_t)ptr & (sz - 1)))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(Array)                (sizeof(Array)/sizeof((Array)[0]))
#endif

#define DIV_ROUND(x, len)                (((x) + (len) - 1) / (len))
#define ROUND_UP(x, align)               DIV_ROUND(x, align) * (align)

#define SET_U32_BIT(n,p) ((n) |= (UINT32_C(1) << (p)))

/* ROTATE32 — left rotate. FIPS specifies right rotations; our rotation
 * coefficients differ from the FIPS document by 32-N accordingly. */
#define ROTATE32(a,n)                    (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))

/*===========================================================================
              Conditional Check and Flow Control Macros
 ===========================================================================*/
#define UC_GUARD(cond, st, env)                 if (!(cond)) {                                                            \
                                                  EMSG("UCLIB: %s(%d), ret=%x", __FUNCTION__, __LINE__, st);              \
                                                  return (st);                                                            \
                                                }

#define UC_PASS(cond, st)                       if (cond) return (st)

#define UC_BAIL_IF_FALSE(cond)                  if (!(cond)) return

#define UC_JMP_IF_FALSE(cond, label)            if (!(cond)) goto label

#define UC_JMP_IF_TRUE(cond, label)             if (cond) goto label

#define UC_GUARD_OVERFLOW(addr, sz, st, env)    if (((uintptr_t)(addr) + (sz)) < (uintptr_t)(addr)) {                     \
                                                  if (env) {                                                              \
                                                    EMSG("UCLIB: %s(%d), ret=%x", __FUNCTION__, __LINE__, st); \
                                                  }                                                                       \
                                                  return (st);                                                            \
                                                }

#define UCLIB_ENV_MAGIC_NUMBER    0xD0E1A2D3

typedef enum
{
  ENV_CE_BASE                = 0x0,
  ENV_CE_BAM_BASE            = 0x1,
  ENV_CE_IRQ_NUM             = 0x2,
  ENV_BASE_ADDR_INVALID      = 0x7FFFFFFF
}env_base_addr_var_t;

typedef enum
{
  GET               = 0x1,
  SET               = 0x2,
  INCR              = 0x4,
  DECR              = 0x5,
  OP_MAX            = 0x7FFFFFFF
}env_var_op;


typedef enum
{
  ENV_BAM_TX_DESC           = 0x0,
  ENV_BAM_RX_DESC           = 0x1,
  ENV_BAM_RSLT              = 0x2,
  ENV_BAM_CONTAINER         = 0x3,
  ENV_BAM_CMD_DESC_BUF      = 0x4,
  ENV_BAM_ADDR_COUNT        = 0x5,
  ENV_BAM_TMP_OUT_BUF       = 0x6,
  ENV_BAM_ADDR_INVALID      = 0x7FFFFFFF
}env_bam_var_t;

typedef enum
{
  ENV_INIT_BAM_DRV_CTX       = 0x0,
  ENV_INIT_DATA_INVALID      = 0x7FFFFFFF
}env_init_data_var_t;

typedef enum
{
  CRYPTO_OPERATION_AES = 0,
  CRYPTO_OPERATION_ECC = 1,
  CRYPTO_OPERATION_ECDH = 2,
  CRYPTO_OPERATION_MAC = 3,
  CRYPTO_OPERATION_KDF = 4,
  CRYPTO_OPERATION_RSA = 5,
  CRYPTO_OPERATION_SHA = 6,
  CRYPTO_OPERATION_TDES = 7,
  CRYPTO_OPERATION_SM2 = 8,
  CRYPTO_OPERATION_INVALID
} crypto_operation_enum;

/* Environment interface for Hardware Key Manager */
typedef struct env_hwkm_key_info_s
{
  const uint8_t *auth_key;
  size_t auth_key_sz;
  const uint8_t *encr_key;
  size_t encr_key_sz;
  uint8_t pipe;
  bool odd_idx;
  uint32_t auth_key_slot_id;
  uint32_t encr_key_slot_id;
  bool clear_only;
} env_hwkm_key_info_t;

typedef const struct env_s env_t;

typedef enum {
	UCLIB_SUCCESS = 0,

	/* Generic errors */
	UCLIB_ERR_FAILURE = 1,
	UCLIB_ERR_INVALID_ARG,
	UCLIB_ERR_NOT_SUPPORTED,
	UCLIB_ERR_OUT_OF_MEMORY,
	UCLIB_ERR_ADD_OVERFLOW,
    UCLIB_ERR_INV_DGST_CONTEXT,

	/* Context and state errors */
	UCLIB_ERR_CE_HAL_CTX_INVALID,
	UCLIB_ERR_INV_BAM_CTX,
	UCLIB_ERR_INV_BAM_CTX_SIZE,
    UCLIB_ERR_INV_ENV_PTR,
    UCLIB_ERR_INVALID_CONTEXT,

	/* BAM initialization and pipe errors */
	UCLIB_ERR_BAM_INIT_FAILED,
	UCLIB_ERR_BAM_INIT_TXPIPE,
	UCLIB_ERR_BAM_INIT_RXPIPE,
	UCLIB_ERR_BAM_PIPE_BUSY,
	UCLIB_ERR_BAM_TX_IRQ_MODE,
	UCLIB_ERR_BAM_RX_IRQ_MODE,
	UCLIB_ERR_BAM_TX_PIPE_POLL,

	/* BAM polling and interrupt errors */
	UCLIB_ERR_BAM_IRQ_WAIT,
	UCLIB_ERR_BAM_POLL_API_ERR,
	UCLIB_ERR_BAM_POLL_TX_TIMEOUT,
	UCLIB_ERR_BAM_POLL_RX_TIMEOUT,

	/* BAM transfer descriptor errors */
	UCLIB_ERR_BAM_XFR_CMD_DSC,
	UCLIB_ERR_BAM_XFR_DAT_DSC,
	UCLIB_ERR_BAM_XFR_DAT_DSC_RX,
	UCLIB_ERR_BAM_XFR_RSLT_DSC,

	/* BAM buffer and address translation errors */
	UCLIB_ERR_BAM_BUF_PA_XLATE,
	UCLIB_ERR_INV_BAM_BUF_ADDR,
	UCLIB_ERR_VTOP_FAILED,

	/* BAM event errors */
    UCLIB_ERR_MUTEX_LOCK,
    UCLIB_ERR_MUTEX_RELEASE,
	UCLIB_ERR_EVENT_CREATE_FAILED,
	UCLIB_ERR_EVENT_DEL_FAILED,

	/* Transfer validation errors */
	UCLIB_ERR_INV_AUTH_XFER_SZ,
	UCLIB_ERR_INV_CIPHER_XFER_SZ,

	/* Cipher/XTS configuration errors */
    UCLIB_ERR_CIPHER_SMALL_OUT_BUF,
	UCLIB_ERR_CIPHER_INV_XTS_KEY_PTR,
	UCLIB_ERR_CIPHER_INV_XTS_KEY_SZ,
	UCLIB_ERR_CIPHER_INV_XTS_DU_SIZE,

	/* CE hardware status errors */
	UCLIB_ERR_HAL_MAC_FAILED,
	UCLIB_ERR_HAL_ACCESS_VIOL,
	UCLIB_ERR_HAL_PIPE_ACTIVE_ERR,
	UCLIB_ERR_HAL_CFG_CHNG_ERR,
	UCLIB_ERR_HAL_DOUT_ERR,
	UCLIB_ERR_HAL_DIN_ERR,
	UCLIB_ERR_HAL_AXI_ERR,
	UCLIB_ERR_HAL_HSD_ERR,
	UCLIB_ERR_HAL_CE_BUSY_ERR,
	UCLIB_ERR_HAL_BIST_ERR,

	/* CE key and key-pipeline errors */
	UCLIB_ERR_HAL_KEY_ERR,
	UCLIB_ERR_HAL_PIPE_KEY_USAGE_ERR,
	UCLIB_ERR_HAL_PIPE_KEY_TIMER_ERR,
	UCLIB_ERR_HAL_PIPE_KEY_PAUSE_ERR,
	UCLIB_ERR_HAL_HW_KEY_USAGE_ERR,
    UCLIB_ERR_HAL_PHK_USAGE_ERR,
    UCLIB_ERR_HAL_SHK_USAGE_ERR,
    UCLIB_ERR_HAL_HMAC_KEY_ERR,
    UCLIB_ERR_HAL_INVALID_ALGO_CFG_ERR

} uclib_status_t;

env_t * ucenv_get_env(void);
void * ENV_zalloc(env_t *env, size_t size);
void ENV_mem_free(env_t *env, void *ptr);
void ENV_mem_clear(env_t *env, void *ptr, size_t size);
size_t ENV_sec_memcpy(env_t *env, void *dst, size_t dst_sz, const void *src, size_t src_sz);
uintptr_t ENV_HW_base(env_t *env, env_base_addr_var_t var);
bool ENV_is_bam_polling_mode(env_t *env);
size_t ENV_get_maxBamDataSize(env_t *env);
int ENV_ce_clk_enable(env_t *env);
int ENV_ce_clk_disable(env_t *env);
int ENV_vtop(env_t *env, uintptr_t va, uintptr_t *pa);
void ENV_dcache_clean_and_invalidate_region(env_t *env, void *addr, size_t length);
size_t ENV_dcache_line_size(env_t *env);
void ENV_mem_barrier(env_t *env);
int ENV_mutex_lock(env_t *env);
int ENV_mutex_release(env_t *env);
int ENV_install_hw_key(env_t *env, env_hwkm_key_info_t *key_info);
bool ENV_is_use_550_features(env_t *env);
bool ENV_cmds_over_hwio(env_t *env);
uint8_t ENV_get_default_pipeset_num(env_t *env);
void* ENV_get_driver_ctx(env_t *env, env_var_op operation, env_init_data_var_t var, void *data);
uint8_t* ENV_get_ce_bam_buf_addr(env_t *env, env_bam_var_t var);
size_t ENV_get_ce_bam_buf_size(env_t *env, env_bam_var_t var);
int ENV_event_create(env_t *env);
int ENV_event_wait(env_t *env);
int ENV_event_trigger(env_t *env);
void ENV_reg_dump_init(env_t *env, void *bam_handle);
size_t ENV_BAM_get_max_poll_count(env_t *env);
size_t ENV_BAM_get_max_rlst_dump_cnt(env_t *env);
uint32_t ENV_BAM_get_ce_bam_pipe_set_cnt(env_t *env);
vaddr_t ce_hwio_phys_to_virt(uintptr_t phys_addr);

#endif /* __UCLIB_PRV_ENV */
