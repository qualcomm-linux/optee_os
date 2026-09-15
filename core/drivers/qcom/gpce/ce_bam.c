// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * Crypto Engine BAM HAL implementation.
 *
 * Provides BAM-based transport services for the Qualcomm
 * Crypto Engine, including command descriptor generation,
 * context programming, data transfer, result handling,
 * and transfer synchronization.
 */

#include "qce_env.h"
#include "ce_bam.h"
#include "ce_hal.h"
#include "bam.h"
#include "hash.h"

// Default BAM pipes for secure use cases
#define TX_PIPE_NUM               0x0
#define RX_PIPE_NUM               0x1
#define BAM_PIPE_SIZE             0x100

#define QUEUE_TX_PIPE_XFER        true
#define QUEUE_RX_PIPE_XFER        false

#define LAST_PIPE_XFER_REQ        true
#define NEXT_PIPE_XFER_REQ        false

#define CMD_REG_ADD(addr,val)                                                 \
    {                                                                         \
      UC_GUARD(                                                               \
        ctx->ce_reg_idx < (ctx->ce_reg_len / sizeof(bam_ce_type)),            \
        UCLIB_ERR_FAILURE,                                                    \
        env);                                                                 \
      BAM_FILL_CE(                                                            \
        (uintptr_t)(ctx->ce_reg_set),                                         \
        (ctx->ce_reg_idx)++,                                                  \
        addr,                                                                 \
        BAM_CE_OP_WRITE,                                                      \
        val,                                                                  \
        0xFFFFFFFF);                                                          \
    }

#define CE_CMD_WRITE(addr, val)                                               \
  {                                                                           \
    if (use_hwio)                                                             \
    {                                                                         \
      out_dword(addr, val);                                                   \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      CMD_REG_ADD(addr, val);                                                 \
    };                                                                        \
  }

#define COPY_WORD(arr, idx, val)                                              \
  {                                                                           \
    if (IS_SIZE_ALIGNED(arr, 4))                                              \
    {                                                                         \
      val = (CAST_PTR_TYPE(arr, uint32_t))[idx];                              \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      ENV_sec_memcpy(                                                         \
        env,                                                                  \
        (void *) &val,                                                        \
        sizeof(val),                                                          \
        arr + (idx * sizeof(val)),                                            \
        sizeof(val));                                                         \
    }                                                                         \
  }

#define RESULT_SIZE_PER_SQZ      200    /* size of uint32_t CRYPTO_AUTH_IV[50] */

typedef struct ce_bam_ptr_s {
  uintptr_t va;
  uintptr_t pa;
}ce_bam_ptr_t;

// TODO: the following definitions should be comming from cfg.h
// Do not hardcode here
#define ENCR_KEY_COUNT            8
#define AES_CCM_CNTR_VECTOR_SIZE  4
#define AES_CCM_NONCE_VECTOR_SIZE 4

typedef enum {
  TX_PIPE_FLG = 1,
  RX_PIPE_FLG
} CE_BAM_PIPE_FLG;

typedef struct cmd_reg_s
{
  bam_ce_type CE_CRYPTO_STATUS;
  bam_ce_type CE_CRYPTO_STATUS2;

  bam_ce_type CE_CONFIG_CMD;
  bam_ce_type CE_AUTH_SEG_CFG_CMD;
  bam_ce_type CE_ENCR_SEG_CFG_CMD;

  bam_ce_type CE_SEG_SIZE_CMD;

  bam_ce_type CE_AUTH_SEG_SIZE_CMD;
  bam_ce_type CE_AUTH_SEG_START_CMD;
  bam_ce_type CE_ENCR_SEG_SIZE_CMD;
  bam_ce_type CE_ENCR_SEG_START_CMD;

  bam_ce_type CE_AUTH_BYTECNT0_CMD;
  bam_ce_type CE_AUTH_BYTECNT1_CMD;
  bam_ce_type CE_AUTH_BYTECNT2_CMD;
  bam_ce_type CE_AUTH_BYTECNT3_CMD;
  bam_ce_type CE_AUTH_IV_CMD[MAX_AUTH_IV_COUNT];
  bam_ce_type CE_AUTH_KEY_CMD[MAX_AUTH_KEY_COUNT];
  bam_ce_type CE_AUTH_INFO_NONCE_CMD[AES_CCM_NONCE_VECTOR_SIZE];

  bam_ce_type CE_ENCR_CNTR_IV0_CMD;
  bam_ce_type CE_ENCR_CNTR_IV1_CMD;
  bam_ce_type CE_ENCR_CNTR_IV2_CMD;
  bam_ce_type CE_ENCR_CNTR_IV3_CMD;
  bam_ce_type CE_ENCR_CNTR_MASK_CMD1;   //impact IV3 which is LSB
  bam_ce_type CE_ENCR_CNTR_MASK0_CMD1;  //Impact IV10 which is MSB (crypto 5.1)
  bam_ce_type CE_ENCR_CNTR_MASK1_CMD1;  //Impact IV1 (crypto 5.1)
  bam_ce_type CE_ENCR_CNTR_MASK2_CMD1;  //Impact IV2 (crypto 5.1)
  bam_ce_type CE_ENCR_KEY_CMD[ENCR_KEY_COUNT];
  bam_ce_type CE_ENCR_XTS_KEY_CMD[ENCR_KEY_COUNT];
  bam_ce_type CE_ENCR_XTS_DU_SIZE_CMD;
  bam_ce_type CE_ENCR_CCM_INIT_CNTR_CMD[AES_CCM_CNTR_VECTOR_SIZE];

  bam_ce_type CE_AUTH_GOPROC_CMD;

} cmd_reg_t;

typedef struct cmd_ce_550_reg_s
{
  bam_ce_type CE_DATA_PATT_PROC_CFG_CMD; // Added for CE 5.5.0
  bam_ce_type CE_DATA_PARTIAL_BLOCK_PROC_CFG_CMD; //Added for CE 5.5.0

  bam_ce_type CE_ENCR_PIPE_KEY_CMD[ENCR_KEY_COUNT];
  bam_ce_type CE_ENCR_PIPE_XTS_KEY_CMD[ENCR_KEY_COUNT];

  bam_ce_type CE_ENCR_PIPE_KEY_USAGE_RULES_CMD;
  bam_ce_type CE_PIPE_KEY_TIMER_EN_CMD;
  bam_ce_type CE_PIPE_KEY_TIMER_LIMIT_LO_CMD;
  bam_ce_type CE_PIPE_KEY_TIMER_LIMIT_HI_CMD;
  bam_ce_type CE_PIPE_KEY_PAUSE_CFG_CMD;
} cmd_ce_550_reg_t;

typedef struct cmd_ce_560_580_reg_s
{
  bam_ce_type CE_CRYPTO_STATUS3;
  bam_ce_type CE_CRYPTO_STATUS4;
  bam_ce_type CE_CRYPTO_STATUS6;  //Added for CE 5.7.0

  bam_ce_type CE_DATA_PATT_PROC_CFG_CMD;
  bam_ce_type CE_DATA_PARTIAL_BLOCK_PROC_CFG_CMD;

  bam_ce_type CE_PIPE_KEY_TIMER_EN_CMD;
  bam_ce_type CE_PIPE_KEY_TIMER_LIMIT_LO_CMD;
  bam_ce_type CE_PIPE_KEY_TIMER_LIMIT_HI_CMD;
  bam_ce_type CE_PIPE_KEY_PAUSE_CFG_CMD;
  bam_ce_type CE_AUTH_IVM_CMD[MAX_AUTH_IVM_COUNT];
} cmd_ce_560_580_reg_t;

/* cb ctx to be passed in bam pipe init */
typedef struct pipe_cb_ctx_s
{
  /* set to indicate xfer on tx/rx pipe completed and callback received*/
  bool    tx_xfer_done;
  bool    rx_xfer_done;

  /* set as per result from tx/rx pipe callback */
  int     tx_result;
  int     rx_result;

  ce_hal_ctx_t *hal;
} pipe_cb_ctx_t;

typedef struct BAM_DRIVER_CTX_s
{
  bam_handle    handle;
  bam_handle    tx_pipe_handle;
  bam_handle    rx_pipe_handle;
  uint32_t      tx_pipe_num;
  uint32_t      rx_pipe_num;
  bool          init_done;

  bam_ce_type   *ce_reg_set;
  size_t        ce_reg_len;
  size_t        ce_reg_idx;

  ce_bam_ptr_t  ce_rslt_dump_ptr;
  ce_bam_ptr_t  ce_rslt1_dump_ptr;
  ce_bam_ptr_t  ce_cmd_desc_ptr;

  pipe_cb_ctx_t *pipe_cb_ctx;

  size_t        tx_num_desc;
  size_t        rx_num_desc;
}BAM_DRIVER_CTX_t;

/*===========================================================================
                 DEFINITIONS AND TYPE DECLARATIONS
 ===========================================================================*/

/* Call flow for interrupt mode operation
 * 1. Init BAM handle with BAM base va/pa addr, IRQ num, IRQ mask, cb_function, cb_data
 * 2. Init BAM pipes with Pipe cb_function, pipe cb_ctx
`* 3. Set BAM IRQ mode
 * 4. Create an event object
 * 5. Pipe transfer operation:
 *    a. Modify pipe cb_ctx to indicate transfer being done for corresponding pipe. Also initialize the
 *       pipe result to zero for that pipe.
 *    b. Call BAM pipe transfer
 *    c. When transfer is done, BAM driver would call the corresponding cb fn with pipe result.
 *       The pipe result (bam_result_type) holds the type of event that occured (bam_event_type) and
 *       the pipe cb_data that was passed during pipe init
 *    d. If the event is DESC_DONE, we set the pipe-isr-result based on the state
 *       of the engine. If the event is EOT we set pipe_isr_result for that pipe to indicate success.
 *       If any other event type is received, the result is set to an error value.
 *       If result is received for all pipes, we trigger the event. This event
 *       indicates that all pipe transfers are done.
 *    e. The BAM_isr_wait has been waiting for this event and the wait is complete at this point.
 * 6. Repeat Step 6 as needed
 * 7. Delete the event object
 */

/* Modify the pipe cb ctx to indicate data xfer to be done on a pipe and reset the result */
static void CE_BAM_set_pipe_cb_ctx(ce_hal_ctx_t *me, CE_BAM_PIPE_FLG pipe_flg) {
  env_t *env = me->env;
  BAM_DRIVER_CTX_t *ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_BAIL_IF_FALSE(ctx);
  ctx->pipe_cb_ctx->hal = me;

  if (TX_PIPE_FLG == pipe_flg) {
    ctx->pipe_cb_ctx->tx_xfer_done = false;
    ctx->pipe_cb_ctx->tx_result = UCLIB_SUCCESS;
  }
  else if (RX_PIPE_FLG == pipe_flg) {
    ctx->pipe_cb_ctx->rx_xfer_done = false;
    ctx->pipe_cb_ctx->rx_result = UCLIB_SUCCESS;
  }
}

/* Helper function to check if all expected pipe cb results have been received
 * and if so trigger an event
 */
static void CE_BAM_trigger_transfers_done_event(pipe_cb_ctx_t *cb_ctx)
{
  env_t *env = cb_ctx->hal->env;

  if (cb_ctx->tx_xfer_done && cb_ctx->rx_xfer_done)
  {
    ENV_event_trigger(env);
  }
}

static bool CE_BAM_is_busy_err_code(int status)
{
  return (UCLIB_ERR_BAM_PIPE_BUSY == status ||
          UCLIB_ERR_HAL_CE_BUSY_ERR == status ||
          UCLIB_ERR_BAM_POLL_TX_TIMEOUT == status ||
          UCLIB_ERR_BAM_POLL_RX_TIMEOUT == status);
}

static bool CE_BAM_is_fatal_err_code(int status, bool treat_busy_as_err)
{
  if (UCLIB_SUCCESS == status || UCLIB_ERR_HAL_MAC_FAILED == status)
    return false;
  else if (CE_BAM_is_busy_err_code(status))
    return treat_busy_as_err;
  else
    return true;
}

static void CE_BAM_pipe_cb_helper(bam_result_type data, CE_BAM_PIPE_FLG pipe_flg) {
  pipe_cb_ctx_t *cb_data = CAST_PTR_TYPE(data.cb_data, pipe_cb_ctx_t);
  int ret;

  if (BAM_EVENT_DESC_DONE == data.event)
  {
    if (TX_PIPE_FLG == pipe_flg)
    {
      ret = CE_HAL_get_ce_state(cb_data->hal, true);
      if (CE_BAM_is_fatal_err_code(ret, false))
      {
        cb_data->tx_result = ret;
        cb_data->tx_xfer_done = true;
        cb_data->rx_xfer_done = true;   // also setting this to trigger DAL event and exit isr wait
      }
      else
      {
        return;
      }
    }
    else
    {
      /* For the RX pipe we do not care about DESC_DONE events, and only care
       * about the EOT event.
       * In fact, this event should not occur..we should not be getting
       * interrupted for DESC_DONE events on the RX pipe since we are not
       * setting the INT flag on any RX descriptor. We only set the EOT flag
       * for the result dump buffer.
       */
      return;
    }
  }
  else if (BAM_EVENT_EOT == data.event)
  {
    if (TX_PIPE_FLG == pipe_flg)
    {
      cb_data->tx_result = UCLIB_SUCCESS;
      cb_data->tx_xfer_done = true;
    }
    else
    {
      cb_data->rx_result = UCLIB_SUCCESS;
      cb_data->rx_xfer_done = true;
    }
  }
  else {
    if (TX_PIPE_FLG == pipe_flg)
    {
      cb_data->tx_result = UCLIB_ERR_FAILURE;
      cb_data->tx_xfer_done = true;
    }
    else
    {
      cb_data->rx_result = UCLIB_ERR_FAILURE;
      cb_data->rx_xfer_done = true;
    }
  }
  CE_BAM_trigger_transfers_done_event(cb_data);
}

/* BAM Pipe ISR callback function (tx pipe) */
static void CE_BAM_tx_pipe_isr_cb(bam_result_type data) {
  CE_BAM_pipe_cb_helper(data, TX_PIPE_FLG);
}

/* BAM Pipe ISR callback function (rx pipe) */
static void CE_BAM_rx_pipe_isr_cb(bam_result_type data) {
  CE_BAM_pipe_cb_helper(data, RX_PIPE_FLG);
}

/* Waits for the event that will be triggered by xx_pipe_isr_cb after all pipe operations done
 * (Equivalent to bam_pipe_polling in polling mode)
 */
static int CE_BAM_isr_wait(env_t *env) {
  int ret = UCLIB_SUCCESS;
  BAM_DRIVER_CTX_t *ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(ctx, UCLIB_ERR_INV_BAM_CTX, env);
  UC_GUARD(UCLIB_SUCCESS == (ret = ENV_event_wait(env)), ret, env);

  /* determine which error to return */
  ret = (ctx->pipe_cb_ctx->tx_result != UCLIB_SUCCESS) ? ctx->pipe_cb_ctx->tx_result : ctx->pipe_cb_ctx->rx_result;

  // reset cb ctx
  ctx->pipe_cb_ctx->tx_xfer_done = ctx->pipe_cb_ctx->rx_xfer_done = false;
  ctx->pipe_cb_ctx->tx_result = ctx->pipe_cb_ctx->rx_result = UCLIB_SUCCESS;

  return ret;
}

static size_t get_ce_ver_cmd_reg_size(env_t *env)
{
  if (CE_HWIO_IN(CRYPTO0_CRYPTO_VERSION) >= CE_HW_VERSION(5,5,0))
  {
    return MAX(sizeof(cmd_ce_550_reg_t), sizeof(cmd_ce_560_580_reg_t));
  }
  else
  {
    return 0;
  }
}

static int CE_BAM_pipe_transfer
(
  ce_hal_ctx_t *me,
  bam_paddr buf_pa,
  uint16_t buf_size,
  uint16_t xfer_opts,
  CE_BAM_PIPE_FLG user_data,
  BAM_DRIVER_CTX_t *bam_ctx
)
{
  bam_handle pipehandle;
  if (TX_PIPE_FLG == user_data)
  {
    ++bam_ctx->tx_num_desc;
    pipehandle = bam_ctx->tx_pipe_handle;
  }
  else
  {
    ++bam_ctx->rx_num_desc;
    pipehandle = bam_ctx->rx_pipe_handle;
  }
  if (!ENV_is_bam_polling_mode(me->env)) {
    CE_BAM_set_pipe_cb_ctx(me, user_data);
  }
  return bam_pipe_transfer(pipehandle, buf_pa, buf_size, xfer_opts, (void *)user_data);
}

static void clear_cmd_regs(env_t *env, BAM_DRIVER_CTX_t *ctx)
{
  // Clear command registers
  ENV_mem_clear(env, ctx->ce_reg_set, ctx->ce_reg_len);
  ctx->ce_reg_idx = 0;
  ctx->tx_num_desc = ctx->rx_num_desc = 0;
}

static int CE_BAM_polling(ce_hal_ctx_t *me, bool is_tx_pipe, bool blocking)
{
  int ret = UCLIB_SUCCESS;
  env_t *env = me->env;
  bam_result_type pipe_result = {0};
  bam_handle h;
  int timeout_err;
  int loop_cnt = 0;

  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);

  size_t num_iter = 0;
  size_t max_iter = ENV_BAM_get_max_poll_count(env);

  if (is_tx_pipe)
  {
    h = ctx->tx_pipe_handle;
    max_iter *= ctx->tx_num_desc;
    timeout_err = UCLIB_ERR_BAM_POLL_TX_TIMEOUT;
  }
  else
  {
    h = ctx->rx_pipe_handle;
    max_iter *= ctx->rx_num_desc;
    timeout_err = UCLIB_ERR_BAM_POLL_RX_TIMEOUT;
  }

  //Set up polling wait loop for BAM operation
  do {
    UC_GUARD(
      BAM_SUCCESS == bam_pipe_poll(h, &pipe_result),
      UCLIB_ERR_BAM_POLL_API_ERR,
      env);

    /* We expect this condition to only be hit when the command-descriptor has
     * been processed/acknowledged. If it is hit more often, for example for
     * data descriptors, then that could degrade performance
     */
    if (is_tx_pipe && BAM_EVENT_DESC_DONE == pipe_result.event)
    {
      ret = CE_HAL_get_ce_state(me, true);
      UC_GUARD(!CE_BAM_is_fatal_err_code(ret, false), ret, env);
      pipe_result.event = BAM_EVENT_INVALID;
    }

    ++num_iter;
    if (num_iter > max_iter)
    {
     /* We observed that in few cases it is taking more iterations than expected.
        So, this fix is to ignore the CE busy error incase we reached the max_iteration count
        i.e, max_iteration count is decided by number of RX/TX descriptors multiplied by
        0x4000 iterations (exprimented value for each descriptor to process).
        Incase of the real error secnario there is a LLCC timout occurs after 13.6ms
        which is used for capturing the system level issues.
     */
     if((UCLIB_SUCCESS == (ret = CE_HAL_get_ce_state(me, true))))
     {
       return timeout_err;
     }
     else
     {
       if(ret == UCLIB_ERR_HAL_CE_BUSY_ERR)
       {
         num_iter = 0;
         loop_cnt++;
         /* incase of UCLIB_ERR_HAL_CE_BUSY_ERR if operation is not completing
            after waiting for 10 iterations we are returing the error which will
            avoid infinite loop
         */
         if(loop_cnt > 10)
         {
           return ret;
         }
       }
       else
       {
         return ret;
       }
     }
    }

  } while (BAM_EVENT_INVALID == pipe_result.event && blocking);

  return
    (BAM_EVENT_INVALID == pipe_result.event)
    ? UCLIB_ERR_BAM_PIPE_BUSY
    : UCLIB_SUCCESS;
}

static int CE_BAM_wait_for_xfer(ce_hal_ctx_t *me, bool blocking)
{
  int ret = UCLIB_SUCCESS;
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);

  if (ENV_is_bam_polling_mode(me->env)) {
    if (ctx->tx_num_desc)
    {
      //Set up tx polling wait loop for write operation
      UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_polling(me, true, blocking)), __Exit);
      CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_IS_ZERO_LEN_STR);
      ctx->tx_num_desc = 0;
    }

    if (ctx->rx_num_desc)
    {
      //Set up rx pipe polling wait loop for read operation
      UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_polling(me, false, blocking)), __Exit);
      ctx->rx_num_desc = 0;
    }
  }
  else {
    UC_JMP_IF_FALSE(UCLIB_SUCCESS == (ret = CE_BAM_isr_wait(env)), __Exit);
  }

  //Make sure CE operation is also complete
  while ((UCLIB_ERR_HAL_CE_BUSY_ERR == (ret = CE_HAL_get_ce_state(me, true))) && blocking);

__Exit:
  if (CE_BAM_is_fatal_err_code(ret, blocking))
  {
    /* Calling bam_pipe_enable will reset the descriptor offsets so that any
     * data that was not consumed by the engine when the error occurred can be
     * discareded.
     * Also, tt appears that calling bam_pipe_enable also unlocks the pipes so
     * a further unlock command is not necessary.
     */
    (void) bam_pipe_enable(ctx->tx_pipe_handle);
    (void) bam_pipe_enable(ctx->rx_pipe_handle);
  }
  return ret;
}

static int CE_BAM_set_up_xfer(ce_hal_ctx_t *me)
{
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  uint16_t bam_flags = BAM_IOVEC_FLAG_CMD | BAM_IOVEC_FLAG_INT;

  //Disable the tx/rx pipe IRQs
  if (ENV_is_bam_polling_mode(env)) {
    UC_GUARD(BAM_SUCCESS == bam_pipe_setirqmode(ctx->tx_pipe_handle,
                                              0,
                                              BAM_O_DESC_DONE | BAM_O_EOT),
           UCLIB_ERR_BAM_TX_IRQ_MODE, env);

    UC_GUARD(BAM_SUCCESS == bam_pipe_setirqmode(ctx->rx_pipe_handle,
                                              0,
                                              BAM_O_DESC_DONE | BAM_O_EOT),
           UCLIB_ERR_BAM_RX_IRQ_MODE, env);
  }
  else {
    /* Creates an event object if not already created i.e. NULL. Only resets the event in subsequent calls */
    int ret = UCLIB_SUCCESS;
    UC_GUARD(UCLIB_SUCCESS == (ret = ENV_event_create(env)), ret, env);
  }

  if (!ENV_cmds_over_hwio(env))
  {
    if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_CFG_OP))
    {
      /* standalone/TA usecase */
      bam_flags |= BAM_IOVEC_FLAG_EOT;
    }
    else if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_AUTH_ENG_EN) ||
             CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_CIPHER_ENG_EN))
    {
      /* enabling the empty string support zero size data
       - In case of sz is zero, we enable the INT flag in Bam_pipe_transfer with cmd flag
         So that at Bam pollling wait call can retrun on that Flag ,otherwise the code get
         stuck in function bam polling wait at TX_pipe handle .
       - CMD flag can not be used with EOT flag . So only INT is enabled
       - IN case of sz>0 then these event flag options are passed through
         Bam_pipe_transfer call for data transfer[Line 2014] which is not required when
         sz = 0
      */
      bam_flags |= BAM_IOVEC_FLAG_LCK;
      if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_AUTH_ENG_EN) &&
          CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_ZERO_LEN_STR))
      {
        if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_CIPHER_ENG_EN))
        {
          bam_flags |= BAM_IOVEC_FLAG_EOT;
        }
        CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_IS_ZERO_LEN_STR);
      }
    }
    else
    {
      /* Invalid scenario. No commands to send */
      return UCLIB_ERR_BAM_XFR_CMD_DSC;
    }

    /* Flush the command desc buffer if cached */
    if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_CMD_DSC_UNCACHED))
      ENV_dcache_clean_and_invalidate_region(
        env,
        (void *)(ctx->ce_cmd_desc_ptr.va),
        ctx->ce_reg_idx * sizeof(bam_ce_type));

    /* Transfer command desc buffer to CE using BAM */
    UC_GUARD(BAM_SUCCESS == CE_BAM_pipe_transfer(
      me,
      ctx->ce_cmd_desc_ptr.pa,
      ctx->ce_reg_idx * sizeof(bam_ce_type),
      bam_flags,
      TX_PIPE_FLG,
      ctx),
    UCLIB_ERR_BAM_XFR_CMD_DSC,
    env);
  }

  return UCLIB_SUCCESS;
}

static int CE_BAM_queue_xfer(ce_hal_ctx_t *me, const void *buf, size_t sz, bool last_xfer, bool is_tx_pipe)
{
  //Perform write operation data buffer -> CE using BAM
  env_t *env = me->env;
  uintptr_t buf_pa = 0x0;

  if (VA_OP_INVALID == me->va_op) {
    UC_GUARD(UCLIB_SUCCESS == ENV_vtop(env, (uintptr_t)buf, &buf_pa), UCLIB_ERR_BAM_BUF_PA_XLATE, env);

    //Flush the input buffer if cached
    if ((is_tx_pipe && !CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_IN_BUF_UNCACHED)) ||
        (!is_tx_pipe && !CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_OUT_BUF_UNCACHED)))
      ENV_dcache_clean_and_invalidate_region(env, (void *)buf, sz);
  } else {
    buf_pa = (uintptr_t)buf;
  }

  //In case of zero size data xfer is not required
  uint16_t bam_flags;
  if (is_tx_pipe && last_xfer)
    bam_flags = (BAM_IOVEC_FLAG_INT|BAM_IOVEC_FLAG_EOT|BAM_IOVEC_FLAG_NWD);
  else if (!is_tx_pipe && last_xfer && ENV_cmds_over_hwio(env))
    bam_flags = (BAM_IOVEC_FLAG_INT|BAM_IOVEC_FLAG_EOT);
  else
    bam_flags = BAM_IOVEC_FLAG_NO_SUBMIT;

  if (!is_tx_pipe && (VA_OP_INVALID != me->va_op) && last_xfer)
    bam_flags = (BAM_IOVEC_FLAG_INT|BAM_IOVEC_FLAG_EOT);

  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  if (is_tx_pipe) {
    UC_GUARD(BAM_SUCCESS == CE_BAM_pipe_transfer(me,
                                              buf_pa,
                                              sz,
                                              bam_flags,
                                              TX_PIPE_FLG,
                                              ctx),
                                              UCLIB_ERR_BAM_XFR_DAT_DSC, env);
  } else {
    UC_GUARD(BAM_SUCCESS == CE_BAM_pipe_transfer(me,
                                              buf_pa,
                                              sz,
                                              bam_flags,
                                              RX_PIPE_FLG,
                                              ctx),
                                              UCLIB_ERR_BAM_XFR_DAT_DSC_RX, env);
  }

  return UCLIB_SUCCESS;
}

static int CE_BAM_toggle_engine_lock(ce_hal_ctx_t *me, bool lock)
{
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  uint16_t flags = BAM_IOVEC_FLAG_CMD | BAM_IOVEC_FLAG_EOT | (lock ? BAM_IOVEC_FLAG_LCK : BAM_IOVEC_FLAG_UNLCK);
  int ret;

  // Unlock the pipe by writing dummy config register
  uint32_t data =
    (UINT32_C(0) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, MASK_ERR_INTR)) |
    (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, MASK_OP_DONE_INTR)) |
    (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, MASK_DIN_INTR)) |
    (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, MASK_DOUT_INTR)) |
    (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, HIGH_SPD_DATA_EN_N)) |
    (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, LITTLE_ENDIAN_MODE)) |
    (0xE0000 & CE_HWIO_FMSK(CRYPTO0_CRYPTO_CONFIG, REQ_SIZE)) |
    (UINT32_C(0xF) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, PIPE_SET_SELECT));
  bool use_hwio = ENV_cmds_over_hwio(env);

  if (use_hwio)
  {
    return UCLIB_SUCCESS;
  }

  /* Clear command list */
  clear_cmd_regs(env, ctx);

  /* Set CE_CONFIG_CMD_dummy for BAM pipe unlock purpose and HWIO access to
   * CE Context Registers
   */
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_CONFIG), data);

  /* Flush the command desc buffer if cached */
  if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_CMD_DSC_UNCACHED))
    ENV_dcache_clean_and_invalidate_region(
      env,
      (void *)(ctx->ce_cmd_desc_ptr.va),
      ctx->ce_reg_idx * sizeof(bam_ce_type));

  if (!ENV_is_bam_polling_mode(env)) {
     /* Since engine unlock is the only case where we don't queue descriptor on rx pipe,
     setting Rx transfer done to true here so that we trigger DAL event upon EOT on tx pipe only */
    ctx->pipe_cb_ctx->rx_xfer_done = true;
    ctx->pipe_cb_ctx->rx_result = UCLIB_SUCCESS;
  }

  /* Transfer command desc buffer to CE using BAM */
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_pipe_transfer(me,
                                                      ctx->ce_cmd_desc_ptr.pa,
                                                      ctx->ce_reg_idx * sizeof(bam_ce_type),
                                                      flags,
                                                      TX_PIPE_FLG,
                                                      ctx)),
    UCLIB_ERR_BAM_XFR_CMD_DSC,
    env);

  if (ENV_is_bam_polling_mode(env)) {
    UC_GUARD(UCLIB_SUCCESS == CE_BAM_polling(me, true, true), UCLIB_ERR_BAM_TX_PIPE_POLL, me->env);
  }
  else {
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_isr_wait(env)), ret, env);
  }

  return UCLIB_SUCCESS;
}

static bool CE_BAM_use_result_dump(ce_hal_ctx_t *me, bool last)
{
  env_t *env = me->env;
  return
    VA_OP_INVALID == me->va_op &&
    !ENV_cmds_over_hwio(env) &&
    (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_NON_BLOCKING) || last);
}

static int CE_BAM_queue_result(ce_hal_ctx_t *me, bool last)
{
  //Perform write operation data buffer -> CE using BAM
  env_t *env = me->env;

  if (CE_BAM_use_result_dump(me, last) || (me->hw_ver >= CE_HW_VERSION(5,9,0) && (CE_HAL_SHA3_224 <= me->alg && CE_HAL_KMAC256 >= me->alg)))
  {
    //Flush result buffer if cached
    void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
    UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
    BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
    size_t rlst_dump_sz = (CE_HAL_SHA3_224 <= me->alg && CE_HAL_KMAC256 >= me->alg) ? sizeof(ce_result_shake_t) : sizeof(ce_result_t);

    if ( me->hw_ver >= CE_HW_VERSION(5,9,0) && (CE_HAL_SHAKE128 <= me->alg && CE_HAL_KMAC256 >= me->alg)) {
      rlst_dump_sz = ENV_BAM_get_max_rlst_dump_cnt(env) * rlst_dump_sz;
    }

    if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_RSLT_BUF_UNCACHED)) {
      ENV_dcache_clean_and_invalidate_region(me->env, (void *)ctx->ce_rslt_dump_ptr.va, rlst_dump_sz);
    }

    UC_GUARD(BAM_SUCCESS == CE_BAM_pipe_transfer(me,
                                              ctx->ce_rslt_dump_ptr.pa,
                                              rlst_dump_sz,
                                              (BAM_IOVEC_FLAG_INT|BAM_IOVEC_FLAG_EOT),
                                              RX_PIPE_FLG,
                                              ctx),
                                              UCLIB_ERR_BAM_XFR_RSLT_DSC, env);
  }
  return UCLIB_SUCCESS;
}

static int CE_BAM_aligned_buf_reset(ce_hal_ctx_t  *me)
{
  if (me->container) {
    /* Make sure all accesses to container buffer is
       complete before clear the content
     */
    ENV_mem_barrier(me->env);
    size_t  cache_line_sz = ENV_dcache_line_size(me->env);
    ENV_mem_clear(me->env, me->container, 4*cache_line_sz);
    me->container = NULL;
  }
  me->header = NULL;
  me->header_size = 0;
  me->tailer = NULL;
  me->tailer_size = 0;
  me->aligned_obuf = NULL;
  me->aligned_obuf_size = 0;
  me->obuf = NULL;
  me->obuf_size = 0;
  me->spare = NULL;
  return UCLIB_SUCCESS;
}

static int CE_BAM_aligned_buf_init(ce_hal_ctx_t  *me,
                                   const uint8_t *obuf,
                                   size_t        osz)
{
  size_t  cache_line_sz = 0;
  if(me!= NULL)
  {
    if (!me->container) {
     cache_line_sz = ENV_dcache_line_size(me->env);
     UC_GUARD(cache_line_sz <= 64, UCLIB_ERR_OUT_OF_MEMORY, me->env);
     me->container = ENV_get_ce_bam_buf_addr(me->env, ENV_BAM_CONTAINER);
     UC_GUARD(me->container, UCLIB_ERR_OUT_OF_MEMORY, me->env);
     } else {
         if (me->obuf)
           return UCLIB_ERR_NOT_SUPPORTED;
     }

    me->header = IS_SIZE_ALIGNED(me->container, cache_line_sz)?me->container:CAST_PTR_TYPE(ROUND_UP((uintptr_t)me->container, (uintptr_t)cache_line_sz), uint8_t);
    me->tailer = me->header + cache_line_sz;
    me->spare = me->tailer + cache_line_sz;
    UC_GUARD_OVERFLOW(obuf, cache_line_sz, UCLIB_ERR_ADD_OVERFLOW, me->env);
    me->aligned_obuf = IS_SIZE_ALIGNED(obuf, cache_line_sz)?CAST_PTR_TYPE(obuf, uint8_t):CAST_PTR_TYPE(ROUND_UP((uintptr_t)obuf, (uintptr_t)cache_line_sz), uint8_t);

    me->header_size       = ((size_t)(me->aligned_obuf - obuf) > osz)?osz:(size_t)(me->aligned_obuf - obuf);
    me->aligned_obuf_size = ((osz - me->header_size) / cache_line_sz) * cache_line_sz;
    me->tailer_size       = (osz - me->header_size) % cache_line_sz;

    me->obuf              = CAST_PTR_TYPE(obuf, uint8_t);
    me->obuf_size         = osz;
  }

  return UCLIB_SUCCESS;
}

static int CE_BAM_get_align_buf(ce_hal_ctx_t  *me,
                                align_buf_t   type,
                                uint8_t       **obuf,
                                size_t        *osz,
                                bool          *last)
{

  switch (type)
  {
    case BLK_BUF_HEADER:
      *osz = me->header_size;
      *obuf = me->header_size?me->header:NULL;
      *last = (me->aligned_obuf_size || me->tailer_size)?false:true;
      break;

    case BLK_BUF_ALIGNED:
      *osz = me->aligned_obuf_size;
      *obuf = me->aligned_obuf_size?me->aligned_obuf:NULL;
      *last = (me->tailer_size)?false:true;
      break;

    case BLK_BUF_TAILER:
      *osz = me->tailer_size;
      *obuf = me->tailer_size?me->tailer:NULL;
      *last = (me->tailer_size)?true:false;
      break;

    default:
      *osz = 0;
      *obuf = NULL;
      *last = false;
      break;
  }

  return UCLIB_SUCCESS;
}

static int CE_BAM_buf_cpy_align_buf(ce_hal_ctx_t  *me)
{
  if (me->obuf && me->obuf_size) {
    ENV_dcache_clean_and_invalidate_region(me->env, (void *)me->obuf, me->obuf_size);

    if (me->header_size) {
      ENV_sec_memcpy(me->env,
                     me->obuf,
                     me->obuf_size,
                     me->header,
                     me->header_size);
    }

    if (me->tailer_size) {
      ENV_sec_memcpy(me->env,
                    (me->obuf + me->obuf_size - me->tailer_size),
                     me->tailer_size,
                     me->tailer,
                     me->tailer_size);
    }

    CE_BAM_aligned_buf_reset(me);
  }

  return UCLIB_SUCCESS;
}

/**
 * @brief Initializes CE BAM pipe.
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful, nagative value otherwise.
 */
static int CE_BAM_pipe_init( ce_hal_ctx_t *me )
{
  bam_callback_type     tx_pipe_isr, rx_pipe_isr;
  bam_pipe_config_type  bam_tx_pipe_cfg={0};
  bam_pipe_config_type  bam_rx_pipe_cfg={0};
  uint32_t              tx_pipe_num = me->pipe_set_num*2;
  uint32_t              rx_pipe_num = me->pipe_set_num*2+1;

  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t *env = me->env;

  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);

  // check inited or not
  UC_PASS(
    ctx->init_done == true           &&
    tx_pipe_num == ctx->tx_pipe_num  &&
    rx_pipe_num == ctx->rx_pipe_num,
    UCLIB_SUCCESS);

  // deinit current pipe handle
  int ret;
  if( ctx->tx_pipe_handle != NULL)
  {
     UC_GUARD( UCLIB_SUCCESS == (ret = bam_pipe_deinit(ctx->tx_pipe_handle)), ret, env);
     ctx->tx_pipe_handle = NULL;
  }

  if( ctx->rx_pipe_handle != NULL)
  {
     UC_GUARD( UCLIB_SUCCESS == (ret = bam_pipe_deinit(ctx->rx_pipe_handle)), ret, env);
     ctx->rx_pipe_handle = NULL;
  }

  //TX Pipe
  uint8_t *tx_desc = ENV_get_ce_bam_buf_addr(env, ENV_BAM_TX_DESC);
  UC_GUARD(tx_desc, UCLIB_ERR_INV_BAM_BUF_ADDR, env);

  if (!ENV_is_bam_polling_mode(env)) {
    if (!ctx->pipe_cb_ctx) {
      UC_GUARD( NULL != (ctx->pipe_cb_ctx = ENV_zalloc(env, sizeof(pipe_cb_ctx_t))),
                UCLIB_ERR_OUT_OF_MEMORY,
                env);
    }
    CE_BAM_set_pipe_cb_ctx(me, TX_PIPE_FLG);

    tx_pipe_isr.func = CE_BAM_tx_pipe_isr_cb;
    tx_pipe_isr.data = (void *)(ctx->pipe_cb_ctx); // when pipe isr cb is received, this value is used to identify which pipe
  }

  bam_tx_pipe_cfg.options      = BAM_O_DESC_DONE | BAM_O_EOT;
  bam_tx_pipe_cfg.dir          = BAM_DIR_CONSUMER;
  bam_tx_pipe_cfg.mode         = BAM_MODE_SYSTEM;
  bam_tx_pipe_cfg.desc_base_va = (bam_vaddr)tx_desc;
  UC_GUARD(UCLIB_SUCCESS == ENV_vtop(env, bam_tx_pipe_cfg.desc_base_va, (uintptr_t *)&bam_tx_pipe_cfg.desc_base_pa), UCLIB_ERR_VTOP_FAILED, env);
  bam_tx_pipe_cfg.desc_size    = (bam_vaddr)ENV_get_ce_bam_buf_size(env, ENV_BAM_TX_DESC);

  bam_tx_pipe_cfg.evt_thresh    = 1;
  bam_tx_pipe_cfg.peer_base_pa  = 0;
  bam_tx_pipe_cfg.peer_pipe_num = 0;
  bam_tx_pipe_cfg.data_base_pa  = 0;
  bam_tx_pipe_cfg.data_size     = 0;

  UC_GUARD(ctx->tx_pipe_handle = bam_pipe_init(ctx->handle, tx_pipe_num, &bam_tx_pipe_cfg, &tx_pipe_isr), UCLIB_ERR_BAM_INIT_TXPIPE, env);

  //RX Pipe
  uint8_t *rx_desc = ENV_get_ce_bam_buf_addr(env, ENV_BAM_RX_DESC);
  UC_GUARD(rx_desc, UCLIB_ERR_INV_BAM_BUF_ADDR, env);

  if (!ENV_is_bam_polling_mode(env)) {
    CE_BAM_set_pipe_cb_ctx(me, RX_PIPE_FLG);

    rx_pipe_isr.func = CE_BAM_rx_pipe_isr_cb;
    rx_pipe_isr.data = (void *)(ctx->pipe_cb_ctx); // when pipe isr cb is received, this value is used to identify which pipe
  }

  bam_rx_pipe_cfg.options       = BAM_O_DESC_DONE | BAM_O_EOT;
  bam_rx_pipe_cfg.dir           = BAM_DIR_PRODUCER;
  bam_rx_pipe_cfg.mode          = BAM_MODE_SYSTEM;
  bam_rx_pipe_cfg.desc_base_va  = (bam_vaddr)rx_desc;
  UC_GUARD(UCLIB_SUCCESS == ENV_vtop(env, bam_rx_pipe_cfg.desc_base_va, (uintptr_t *)&bam_rx_pipe_cfg.desc_base_pa), UCLIB_ERR_VTOP_FAILED, env);
  bam_rx_pipe_cfg.desc_size     = (bam_vaddr)ENV_get_ce_bam_buf_size(env, ENV_BAM_RX_DESC);

  bam_rx_pipe_cfg.evt_thresh    = 1;
  bam_rx_pipe_cfg.peer_base_pa  = 0;
  bam_rx_pipe_cfg.peer_pipe_num = 0;
  bam_rx_pipe_cfg.data_base_pa  = 0;
  bam_rx_pipe_cfg.data_size     = 0;

  UC_GUARD(ctx->rx_pipe_handle = bam_pipe_init(ctx->handle, rx_pipe_num, &bam_rx_pipe_cfg, &rx_pipe_isr), UCLIB_ERR_BAM_INIT_RXPIPE, env);

  // book keeping here to avoid reinit the same thing.
  ctx->tx_pipe_num = tx_pipe_num;
  ctx->rx_pipe_num = rx_pipe_num;

  ctx->init_done = true;

  return UCLIB_SUCCESS;
}

static int CE_BAM_save_ctx(ce_hal_ctx_t *me, bool last, bool locked)
{
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(env, UCLIB_ERR_INV_ENV_PTR, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  bool use_rslt_dmp = CE_BAM_use_result_dump(me, last);
  bool is_shake_opt = (CE_HAL_SHA3_224 <= me->alg && CE_HAL_KMAC256 >= me->alg) ? true : false;
  ce_result_t *rslt = (ce_result_t *)ctx->ce_rslt_dump_ptr.va;
  ce_result_shake_t *rslt_shake = (ce_result_shake_t *)ctx->ce_rslt_dump_ptr.va;
  UC_GUARD(rslt, UCLIB_ERR_INV_BAM_CTX, env);
  if (
      /* For a non-VA operation, if the engine has been unlocked there is
       * nothing to do. We can read the results before unlocking - either from
       * the result-dump or over HWIO (since the engine would not have been
       * locked in HWIO mode)
       */
      (VA_OP_INVALID == me->va_op && !locked) ||

      /* For a VA operation on HW >= 5.6 we read the result before unlocking */
      (VA_OP_INVALID != me->va_op && !locked && me->hw_ver >= CE_HW_VERSION(5,6,0)) ||

      /* For a VA operation on HW < 5.6 we can only read the result after
       * unlocking
       */
      (VA_OP_INVALID != me->va_op && locked && me->hw_ver < CE_HW_VERSION(5,6,0)))
  {
    return UCLIB_SUCCESS;
  }

  if (!locked && VA_OP_INVALID != me->va_op)
  {
    // Make sure the engine unlock is complete before accessing registers
    while (CE_HWIO_INF(CRYPTO0_CRYPTO_STATUS2, LOCKED));
  }

  /* Auth Engine Context if enabled */
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_AUTH_ENG_EN))
  {
    uint32_t   max_auth_byte_cnt = MAX_AUTH_BYTE_COUNT;
    uint32_t   max_auth_iv_cnt = MAX_AUTH_IV_COUNT;

    if (me->hw_ver < CE_HW_VERSION(5,4,1))
    {
      max_auth_byte_cnt = MAX_AUTH_BYTE_COUNT/2;
      max_auth_iv_cnt = MAX_AUTH_IV_COUNT/2;
    }

    // Save IVs
    for (int i=0; i<max_auth_iv_cnt; i++) {
      me->auth_iv[i] =
        use_rslt_dmp ?
        (is_shake_opt ? rslt_shake->CRYPTO_AUTH_IV[i] : rslt->CRYPTO_AUTH_IV[i]) :
        CE_HWIO_INI(CRYPTO0_CRYPTO_AUTH_IVn, i);
    }

    // save IVm for GPCE ver > 5.9.0 for the context switching
#ifdef HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_ADDR  // CE_050900
    if (me->hw_ver >= CE_HW_VERSION(5,9,0) && is_shake_opt) {
      for (size_t i = 0; i < MAX_AUTH_IVM_COUNT; ++i) {
        me->auth_ivm[i] =
          use_rslt_dmp ?
          rslt_shake->CRYPTO_AUTH_IV[MAX_AUTH_IV_COUNT + i] :
          CE_HWIO_INI(CRYPTO0_CRYPTO_AUTH_IVm, MIN_AUTH_IVM_IDX+i);
      }
   }
#endif

    // Save ByteCounts
    me->auth_byte_cnt[0] =
      use_rslt_dmp ?
      BIG2LITTLE(is_shake_opt ? rslt_shake->CRYPTO_AUTH_BYTECNT[0] : rslt->CRYPTO_AUTH_BYTECNT[0]) :
      CE_HWIO_IN(CRYPTO0_CRYPTO_AUTH_BYTECNT0);
    me->auth_byte_cnt[1] =
      use_rslt_dmp ?
      BIG2LITTLE(is_shake_opt ? rslt_shake->CRYPTO_AUTH_BYTECNT[1] : rslt->CRYPTO_AUTH_BYTECNT[1]) :
      CE_HWIO_IN(CRYPTO0_CRYPTO_AUTH_BYTECNT1);

    if (MAX_AUTH_BYTE_COUNT == max_auth_byte_cnt)
    {
      me->auth_byte_cnt[2] =
        use_rslt_dmp ?
        BIG2LITTLE(is_shake_opt ? rslt_shake->CRYPTO_AUTH_BYTECNT[2] : rslt->CRYPTO_AUTH_BYTECNT[2]) :
        CE_HWIO_IN(CRYPTO0_CRYPTO_AUTH_BYTECNT2);
      me->auth_byte_cnt[3] =
        use_rslt_dmp ?
        BIG2LITTLE(is_shake_opt ? rslt_shake->CRYPTO_AUTH_BYTECNT[3] : rslt->CRYPTO_AUTH_BYTECNT[3]) :
        CE_HWIO_IN(CRYPTO0_CRYPTO_AUTH_BYTECNT3);
    }
  }


  /* Cipher Engine Context if enabled */
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_CIPHER_ENG_EN))
  {
    for (int i=0; i<4; i++)
    {
      uint32_t tmp;
      switch (i) {
        case 0: tmp = use_rslt_dmp ?
		              (is_shake_opt ? rslt_shake->CRYPTO_ENCR_CNTR0_IV0 : rslt->CRYPTO_ENCR_CNTR0_IV0) :
		              CE_HWIO_IN(CRYPTO0_CRYPTO_ENCR_CNTR0_IV0);
		break;
        case 1: tmp = use_rslt_dmp ?
                      (is_shake_opt ? rslt_shake->CRYPTO_ENCR_CNTR1_IV1 : rslt->CRYPTO_ENCR_CNTR1_IV1) :
                      CE_HWIO_IN(CRYPTO0_CRYPTO_ENCR_CNTR1_IV1);
        break;
        case 2: tmp = use_rslt_dmp ?
                      (is_shake_opt ? rslt_shake->CRYPTO_ENCR_CNTR2_IV2 : rslt->CRYPTO_ENCR_CNTR2_IV2) :
                      CE_HWIO_IN(CRYPTO0_CRYPTO_ENCR_CNTR2_IV2);
        break;
        default: tmp = use_rslt_dmp ?
                       (is_shake_opt ? rslt_shake->CRYPTO_ENCR_CNTR3_IV3 : rslt->CRYPTO_ENCR_CNTR3_IV3) :
                       CE_HWIO_IN(CRYPTO0_CRYPTO_ENCR_CNTR3_IV3);
        break;
      }
      if (me->cipher_iv) {
          me->cipher_iv[i] = tmp;
      }
    }
  }

  return UCLIB_SUCCESS;
}

static int CE_BAM_pending_xfer_done(ce_hal_ctx_t *me, bool blocking)
{
  if (!me->xfer_pending)
    return UCLIB_SUCCESS;

  int ret = CE_BAM_wait_for_xfer(me, blocking);
  if (CE_BAM_is_busy_err_code(ret))
  {
    /* Convert the BAM TX/RX polling error values to the generic busy error value
     * so that the caller of this API doesn't need to check against multiple
     * error values
     */
    return UCLIB_ERR_HAL_CE_BUSY_ERR;
  }

  me->xfer_pending = false;

  if (UCLIB_SUCCESS == ret)
  {
    /* Update output buffer if valid */
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_buf_cpy_align_buf(me)), ret, me->env);
  }

  return ret;
}

/**
 * @brief Initializes CE BAM driver.
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful, nagative value otherwise.
 */
int CE_BAM_init(ce_hal_ctx_t *me)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  env_t *env = me->env;
  int ret = UCLIB_SUCCESS;

  /* cmds over hwio not tested with interrupt mode, so return error for that combination */
  if (!ENV_is_bam_polling_mode(env) && ENV_cmds_over_hwio(env))
    return UCLIB_ERR_NOT_SUPPORTED;

  BAM_DRIVER_CTX_t *ctx = (BAM_DRIVER_CTX_t*)ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_JMP_IF_TRUE (NULL != ctx, __BAM_PIPE_INIT);

  size_t cmd_desc_sz = sizeof(cmd_reg_t) + get_ce_ver_cmd_reg_size(env);
  UC_GUARD(ENV_get_ce_bam_buf_size(env, ENV_BAM_CMD_DESC_BUF) >= cmd_desc_sz, UCLIB_ERR_INV_BAM_CTX_SIZE, me->env);

  uint8_t *rslt = ENV_get_ce_bam_buf_addr(env, ENV_BAM_RSLT);
  UC_GUARD(rslt, UCLIB_ERR_INV_BAM_BUF_ADDR, env);
  uintptr_t rslt_dmp_ptr_pa, rslt_dmp_ptr_va = (bam_vaddr)rslt;
  UC_GUARD(
    UCLIB_SUCCESS == ENV_vtop(env, rslt_dmp_ptr_va, &rslt_dmp_ptr_pa),
    UCLIB_ERR_VTOP_FAILED,
    env);
  size_t max_rslt_cnt = ENV_BAM_get_max_rlst_dump_cnt(env);
  uintptr_t rslt1_dmp_ptr_pa, rslt1_dmp_ptr_va = (CE_HAL_SHA3_224 <= me->alg && CE_HAL_KMAC256 >= me->alg) ?
                                                 (bam_vaddr)&rslt[max_rslt_cnt * sizeof(ce_result_shake_t)] :
                                                 (bam_vaddr)&rslt[max_rslt_cnt * sizeof(ce_result_t)];

  UC_GUARD(
    UCLIB_SUCCESS == ENV_vtop(env, rslt1_dmp_ptr_va, &rslt1_dmp_ptr_pa),
    UCLIB_ERR_VTOP_FAILED,
    env);

  uint8_t* bam_cmd_desc_buf = ENV_get_ce_bam_buf_addr(env, ENV_BAM_CMD_DESC_BUF);
  UC_GUARD(bam_cmd_desc_buf, UCLIB_ERR_INV_BAM_BUF_ADDR, env);
  bam_ce_type * cmd_desc_buf = CAST_PTR_TYPE(bam_cmd_desc_buf, bam_ce_type);
  uintptr_t cmd_desc_ptr_pa, cmd_desc_ptr_va = (uintptr_t)(cmd_desc_buf);
  UC_GUARD(
    UCLIB_SUCCESS == (ret = ENV_vtop(env, cmd_desc_ptr_va, &cmd_desc_ptr_pa)),
    UCLIB_ERR_VTOP_FAILED,
    env);

  // BAM init
  bam_callback_type    bam_isr;
  bam_config_type      bam_cfg={0};

  bam_cfg.bam_va         = ENV_HW_base(env, ENV_CE_BAM_BASE);
  // get bam base pa
  UC_GUARD(UCLIB_SUCCESS == ENV_vtop(env, bam_cfg.bam_va, (uintptr_t*)&bam_cfg.bam_pa), UCLIB_ERR_VTOP_FAILED, env);
  bam_cfg.bam_irq        = ENV_HW_base(env, ENV_CE_IRQ_NUM);
  bam_cfg.sum_thresh     = 8192;
  bam_cfg.options        = 0;
  bam_cfg.bam_mti_irq_pa = 0;
  bam_cfg.bam_irq_mask = (BAM_IRQ_HRESP_ERR_EN | BAM_IRQ_ERR_EN);
  bam_isr.func = NULL;
  bam_isr.data = (void *)env;

  /* Pass any uncached memory left over to the BAM driver. In the PBL image
   * this is used by the BAM driver to store some internal data structures
   * that need to reside in uncached memory
   */
  bam_cfg.data = bam_cmd_desc_buf + cmd_desc_sz;

  bam_handle handle;
  UC_GUARD(
    handle = bam_init(&bam_cfg, &bam_isr),
    UCLIB_ERR_BAM_INIT_FAILED,
    env);

  /* Allocate space for the UCLIB BAM driver context. This is done as a last
   * step to avoid having to free the memory if a failure is encountered with
   * bam_init, vtop, etc.
   */
  UC_GUARD(
    NULL != (ctx = ENV_zalloc(env, sizeof(BAM_DRIVER_CTX_t))),
    UCLIB_ERR_OUT_OF_MEMORY,
    env);

  // CE Result dump and command descriptor buffers
  ctx->ce_rslt_dump_ptr.va = rslt_dmp_ptr_va;
  ctx->ce_rslt_dump_ptr.pa = rslt_dmp_ptr_pa;
  ctx->ce_rslt1_dump_ptr.va = rslt1_dmp_ptr_va;
  ctx->ce_rslt1_dump_ptr.pa = rslt1_dmp_ptr_pa;

  // Command descriptor info
  ctx->ce_reg_len = cmd_desc_sz;
  ctx->ce_reg_set = cmd_desc_buf;
  ctx->ce_reg_idx = 0;
  ctx->ce_cmd_desc_ptr.va = cmd_desc_ptr_va;
  ctx->ce_cmd_desc_ptr.pa = cmd_desc_ptr_pa;

  // BAM handle
  ctx->handle = handle;

  // Store the allocated BAM driver context into the environment
  ENV_get_driver_ctx(env, SET, ENV_INIT_BAM_DRV_CTX, ctx);

  /* Register the dump callbback */
  ENV_reg_dump_init(env, (void*)ctx->handle);

__BAM_PIPE_INIT:
  UC_GUARD( UCLIB_SUCCESS == (ret = CE_BAM_pipe_init( me )), ret, env);
  me->ready = CE_BAM_pending_xfer_done;
  me->engine_toggle_lock = CE_BAM_toggle_engine_lock;
  return UCLIB_SUCCESS;
}
/**
 * @brief Determine the 13 least significant bits of settings for
 *        CRYPTO0_CRYPTO_AUTH_SEG_CFG or CRYPTO0_CRYPTO_AUTH_PIPEm_KEY_USAGE_RULES
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return uint32_t with the 13 least significant bits set to the appropriate
 *         values based on the context
 */
static uint32_t CE_BAM_get_auth_seg_cfg_settings(ce_hal_ctx_t *me)
{
  uint32_t auth_cfg = 0;

  if ((CE_HAL_HASH == me->mode) || (CE_HAL_HMAC == me->mode) || (CE_HAL_KMAC == me->mode) || (CE_HAL_HMAC == me->hc_mode))
  {
    if  (CE_HAL_SM3 == me->alg)
    {
      auth_cfg |= CE_HAL_AUTH_ALG_SM3 << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_ALG);
    }
    else
    {
      auth_cfg |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_ALG); // SHA algorithm
      if (CE_HAL_SHA3_224 == me->alg)
      {
        auth_cfg |= CE_HAL_SHA3_HASH_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_SHA3_256 == me->alg)
      {
        auth_cfg |= CE_HAL_SHA3_HASH_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_SHA3_384 == me->alg)
      {
        auth_cfg |= CE_HAL_SHA3_HASH_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_SHA3_512 == me->alg)
      {
        auth_cfg |= CE_HAL_SHA3_HASH_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_SHAKE128 == me->alg)
      {
        auth_cfg |= CE_HAL_SHAKE_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_SHAKE256 == me->alg)
      {
        auth_cfg |= CE_HAL_SHAKE_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_CSHAKE128 == me->alg)
      {
        auth_cfg |= CE_HAL_CSHAKE_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_CSHAKE256 == me->alg)
      {
        auth_cfg |= CE_HAL_CSHAKE_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_KMAC128 == me->alg)
      {
        auth_cfg |= CE_HAL_KMAC_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
      else if (CE_HAL_KMAC256 == me->alg)
      {
        auth_cfg |= CE_HAL_KMAC_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE);
      }
    }
  }

  if ((CE_HAL_HMAC == me->mode) ||
      (CE_HAL_KMAC == me->mode) ||
      (CE_HAL_CMAC == me->mode) ||
      (CE_HAL_CCM == me->mode)  ||
      (CE_HAL_GCM == me->mode)  ||
      (CE_HAL_HC_CBC == me->mode)  ||
      (CE_HAL_HC_CTS == me->mode)  ||
      (CE_HAL_HC_CTR == me->mode)) {
	if (CE_HAL_GCM == me->mode) {
      /* AUTH_MODE setting for GCM is from HPG 4.1.10 and 4.1.11 example setting as below: */
      /* 129. Set the CRYPTO_AUTH_SEG_CFG register to 0x00035e82 for AES256 GCM Encryption */
      /* 146. Set the CRYPTO_AUTH_SEG_CFG register to 0x00430682 for AES256 GCM Decryption */
      auth_cfg |= (UINT32_C(2) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE));
    } else if ((CE_HAL_HMAC == me->mode) ||
               (CE_HAL_KMAC == me->mode) ||
               (CE_HAL_CMAC == me->mode) ||
               (CE_HAL_HMAC == me->hc_mode)) {
      if (me->alg >= CE_HAL_SHA3_224 && me->alg <= CE_HAL_SHA3_512) {
        auth_cfg |= (CE_HAL_SHA3_HMAC_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE));
      } else if (me->alg >= CE_HAL_SHAKE128 && me->alg <= CE_HAL_SHAKE256) {
        auth_cfg |= (CE_HAL_SHAKE_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE));
      } else if (me->alg >= CE_HAL_CSHAKE128 && me->alg <= CE_HAL_CSHAKE256) {
        auth_cfg |= (CE_HAL_CSHAKE_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE));
      } else if (me->alg >= CE_HAL_KMAC128 && me->alg <= CE_HAL_KMAC256) {
        auth_cfg |= (CE_HAL_KMAC_AUTH_MODE << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE));
      } else {
        auth_cfg |= (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_MODE));
      }
    } else {
      if(CE_HAL_CCM != me->mode) {
        auth_cfg |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_ALG);
      }
    }
    if ((CE_HAL_CMAC == me->mode) || (CE_HAL_CCM == me->mode) || (CE_HAL_GCM == me->mode)) {
      auth_cfg |= UINT32_C(2) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_ALG); // AES algorithm
      auth_cfg |= (me->key_sz << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_KEY_SZ));
    }
  }

  return auth_cfg;
}

/**
 * @brief Set up CE auth context
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param cipher_start  [in]    Cipher seg. start offset
 * @param cipher_sz     [in]    Cipher seg. size
 * @param seg_sz        [in]    Total seg. size
 * @param sw_key        [in]    Pointer to SW Key
 * @param sw_key_sz     [in]    SW Key size
 * @param first         [in]    first seg.
 * @param last          [in]    last seg.
 *
 * @return UCLIB_SUCCESS if successful. negative value otherwise.
 */
static int CE_BAM_set_auth_ctx(ce_hal_ctx_t *me,
                               size_t  auth_start,
                               size_t  auth_sz,
                               size_t  seg_sz,
                               uint8_t *sw_key,
                               size_t  sw_key_sz,
                               bool    first,
                               bool    last)
{
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  uint32_t data = 0;
  bool use_hwio = ENV_cmds_over_hwio(env);

  /*Updating the for hash cipher mode*/
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_HC_KEYS))
  {
    sw_key   =  me->hc_hmac_key;
    sw_key_sz = me->hc_key_sz;
  }
  int32_t      max_iv_cnt = MAX_AUTH_IV_COUNT;
  int32_t      max_key_cnt = 1 + HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_MAXn;

#ifdef HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_MAXn_CE541
  if (me->hw_ver < CE_HW_VERSION(5,4,1))
  {
    max_iv_cnt = MAX_AUTH_IV_COUNT / 2;
    max_key_cnt = 1 + HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_MAXn_CE541;
  }
  else if (me->hw_ver < CE_HW_VERSION(5,6,0))
  {
    max_key_cnt = 1 + HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_MAXn_CE551;
  }
#endif

  UC_GUARD((auth_start + auth_sz) <= seg_sz, UCLIB_ERR_INV_AUTH_XFER_SZ, env);
  if (!auth_sz)
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_ZERO_LEN_STR);

  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_SEG_SIZE), (uint32_t) auth_sz);
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_SEG_START), (uint32_t) auth_start);

  if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_NON_BLOCKING) || first)
  {
    //AUTH IV
    for (size_t i = 0; i < MAX_AUTH_IV_COUNT; ++i)
      CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_AUTH_IVn, (i % max_iv_cnt)), me->auth_iv[i % max_iv_cnt]);

    // Set ByteCounts
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_BYTECNT0), me->auth_byte_cnt[0]);
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_BYTECNT1), me->auth_byte_cnt[1]);
    if (me->hw_ver >= CE_HW_VERSION(5,4,1))
    {
      CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_BYTECNT2), me->auth_byte_cnt[2]);
      CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_BYTECNT3), me->auth_byte_cnt[3]);
    }

    //AUTH IVM for CE ver >= 5.8.0
    #ifdef HWIO_CRYPTO0_CRYPTO_AUTH_IVm_ADDR
    if (me->hw_ver >= CE_HW_VERSION(5,8,0) && (CE_HAL_SHA3_224 <= me->alg && CE_HAL_KMAC256 >= me->alg)) {
      for (size_t i = 0; i < MAX_AUTH_IVM_COUNT; ++i)
        CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_AUTH_IVm, (MIN_AUTH_IVM_IDX+i)), me->auth_ivm[i]);
    }
    #endif
  }

  // AUTH KEY
  bool needs_key = (CE_HAL_HMAC == me->mode) ||
                   (CE_HAL_KMAC == me->mode) ||
                   (CE_HAL_HMAC == me->hc_mode) ||
                   (CE_HAL_CMAC == me->mode) ||
                   (CE_HAL_CCM == me->mode) ||
                   (CE_HAL_GCM == me->mode) ||
                   (CE_HAL_KMAC128 == me->alg) ||
                   (CE_HAL_KMAC256 == me->alg);
  for (size_t i = 0; i < max_key_cnt; ++i)
  {
    if (needs_key && sw_key && i < sw_key_sz / 4)
    {
      COPY_WORD(sw_key, i, data);
    }
    else
    {
      data = 0;
    }
    CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_AUTH_KEYn, i), data);
  }

  // Set Nonce in CCM/GCM mode
  for (int i = 0; i < AES_CCM_NONCE_VECTOR_SIZE; i++)
  {
    if ((CE_HAL_CCM == me->mode) || (CE_HAL_GCM == me->mode))
    {
      data = me->nonce[i];
    }
    else
    {
      data = 0;
    }
    CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_AUTH_INFO_NONCEn, i), data);
  }

#ifdef HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_ADDR
  data = 0;
  if (me->alg >= CE_HAL_SHAKE128 && me->alg <= CE_HAL_KMAC256) {
     if(last) {
        /* for the final data being sent, squeeze the result dump */
        data |= me->sqz_cnt << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR, CNTR);
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR), data);
    } else if (!auth_sz && !seg_sz) {
        /* for squeeze processing, squeeze cntr and restore bits need to be set */
        data |= me->sqz_cnt << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR, CNTR);
        data |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR, RESTORE);
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR), data);
    } else {
        /* for the update data being sent, squeeze ctnr need set to 1 as SW WA for multiple absorv supports  */
        data |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR, CNTR);
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR), data);
    }
    CE_HAL_set_auth_cipher_order(me, true); // set auth position before encr for XOFs
  }
#endif

  data = 0;
  data |= CE_BAM_get_auth_seg_cfg_settings(me);

  // Set Auth Position
  if ((CE_HAL_CCM != me->mode) && (CE_HAL_GCM != me->mode)){
    if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_HASH_BEFORE_ENCR))
      data |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_POS);
  }

  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_HW_AUTH_KEY))
  {
    #if defined HWIO_CRYPTO0_CRYPTO_QC_KEY_AUTH_KEY_USAGE_RULE_ADDR && defined HWIO_CRYPTO0_CRYPTO_OEM_KEY_AUTH_KEY_USAGE_RULE_ADDR
    {
      CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_QC_KEY_AUTH_KEY_USAGE_RULE), data);
      CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_OEM_KEY_AUTH_KEY_USAGE_RULE), data);
    }
    #endif

    // Set HW key if enable
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_AUTH_SEG_CFG, USE_HW_KEY_AUTH);
  }
  else if (CE_HAL_wrapped_key_present(me, AUTH_WRAPPED_KEY))
  {
    // Configure to use pipe key if necessary
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_AUTH_SEG_CFG, USE_PIPE_KEY_AUTH);
  }

  if ((CE_HAL_HASH == me->mode) || (CE_HAL_HMAC == me->mode) || (CE_HAL_KMAC == me->mode) || (CE_HAL_HMAC == me->hc_mode))
  {
    data |= me->auth_sz << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_SIZE);
  }

  data |= (last?(UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, LAST)):0);
  data |= (first?(UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, FIRST)):0);
  if ((CE_HAL_HMAC == me->mode) ||
      (CE_HAL_CMAC == me->mode) ||
      (CE_HAL_CCM == me->mode)  ||
      (CE_HAL_GCM == me->mode)  ||
      (CE_HAL_HC_CBC == me->mode)  ||
      (CE_HAL_HC_CTS == me->mode)  ||
      (CE_HAL_HC_CTR == me->mode)) {
    if (CE_HAL_CCM == me->mode) {
      data |= (UINT32_C(4) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_NONCE_NUM_WORDS)); // Nonce size in word
      /* CCM Decryption */
      if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_ENCRYPTION))
        data |= (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_POS));
    } else if (CE_HAL_GCM == me->mode){
      if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_ENCRYPTION))
        data |= (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_POS));
    }

    data |= (me->auth_sz << CE_HWIO_SHFT(CRYPTO0_CRYPTO_AUTH_SEG_CFG, AUTH_SIZE));
  }

  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_SEG_CFG), data);

  if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_CIPHER_ENG_EN))
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_SEG_CFG), 0);

  return UCLIB_SUCCESS;
}

/**
 * @brief Determine the 11 least significant bits of settings for
 *        CRYPTO_ENCR_SEG_CFG or CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_USAGE_RULES
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param last          [in]    Whether this is the last segment
 *
 * @return uint32_t with the 11 least significant bits set to the appropriate
 *         values based on the context
 */
static uint32_t CE_BAM_get_encr_seg_cfg_settings(ce_hal_ctx_t *me, bool last)
{
  uint32_t encr_cfg = 0;

  /* Set Cipher Configuration */
  /* Encryption/Decryption */
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_ENCRYPTION))
    encr_cfg |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCODE);

  /*Setting the LAST flag unconditionally as it was required for off-target BAM emulation tests.
  On hardware, this is only required for CCM and GCM modes.*/
  encr_cfg |= (last?(UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, LAST)):0);

  /* Cipher Mode */
  switch (me->mode) {
  case CE_HAL_ECB:
    encr_cfg |= UINT32_C(0) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_MODE);
    break;
  case CE_HAL_CTS:
  case CE_HAL_CBC:
  case CE_HAL_CBC_VA:
  case CE_HAL_HC_CBC:
  case CE_HAL_HC_CTS:
    encr_cfg |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_MODE);
    break;
  case CE_HAL_CTR:
  case CE_HAL_CTR_VA:
  case CE_HAL_HC_CTR:
    encr_cfg |= UINT32_C(2) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_MODE);
    break;
  case CE_HAL_XTS:
    encr_cfg |= UINT32_C(3) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_MODE);
    break;
  case CE_HAL_CCM:
    encr_cfg |= UINT32_C(4) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_MODE);

    break;
  case CE_HAL_GCM:
    encr_cfg |= UINT32_C(6) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_MODE);
    break;
  case CE_HAL_IDSA:
    encr_cfg |= UINT32_C(5) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_MODE);
    break;
  default: break;
  }

  /* Key Size and algorithm */
  switch (me->alg) {
  case CE_HAL_DES:
    encr_cfg |= UINT32_C(0) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_KEY_SZ);
    encr_cfg |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_ALG);
    break;
  case CE_HAL_2DES:
    encr_cfg |= UINT32_C(2) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_KEY_SZ);
    encr_cfg |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_ALG);
    break;
  case CE_HAL_3DES:
    encr_cfg |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_KEY_SZ);
    encr_cfg |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_ALG);
    break;
  case CE_HAL_AES:
    encr_cfg |= me->key_sz << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_KEY_SZ);
    encr_cfg |= UINT32_C(2) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_ALG);
    break;
  case CE_HAL_SM4:
    encr_cfg |= me->key_sz << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_KEY_SZ);
    encr_cfg |= CE_HAL_ENCR_ALG_SM4 << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_ALG);
    break;
  case CE_HAL_KASUMI:
    encr_cfg |= UINT32_C(4) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_ALG);
    break;
  case CE_HAL_SNOW3G:
    encr_cfg |= UINT32_C(5) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_ALG);
    break;
  case CE_HAL_ZUC:
    encr_cfg |= UINT32_C(6) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ENCR_ALG);
    break;
  default: break;
  }

  return encr_cfg;
}

/**
 * @brief Switch the endianness of the input if the HW requires it
 *        This is a workaround for HW bug QCTDD05833753 in CE 5.5.0 which requires that
 *        CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG, CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG,
 *        and CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN have their configurations
 *        written in the endianness matching the LITTLE_ENDIAN_MODE
 *        bit set in CRYPTO0_CRYPTO_CONFIG.
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param cipher_start  [in]    Cipher seg. start offset
 *
 * @return UCLIB_SUCCESS if successful. negative value otherwise.
 */
static inline uint32_t correct_endianness(ce_hal_ctx_t *me, uint32_t val)
{
  uint32_t ver = me->hw_ver;

  // Check if HW requires the workaround.
  if ((ver == CE_HW_VERSION(5,5,0) || ver == CE_HW_VERSION(5,5,1) || ver == CE_HW_VERSION(5,5,2)) &&
        CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_LITTLE_ENDIAN))
  {
    return (((val & 0x000000FFU) << 24) |
            ((val & 0x0000FF00U) <<  8) |
            ((val & 0x00FF0000U) >>  8) |
            ((val & 0xFF000000U) >> 24));
  }

  return val;
}

/**
 * @brief Set up CE cipher context
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param cipher_start  [in]    Cipher seg. start offset
 * @param cipher_sz     [in]    Cipher seg. size
 * @param seg_sz        [in]    Total seg. size
 * @param sw_key        [in]    Pointer to SW Key
 * @param sw_key_sz     [in]    SW Key size
 * @param first         [in]    first seg.
 * @param last          [in]    last seg.
 *
 * @return UCLIB_SUCCESS if successful. negative value otherwise.
 */
static int CE_BAM_set_cipher_ctx(ce_hal_ctx_t *me,
                                 size_t  cipher_start,
                                 size_t  cipher_sz,
                                 size_t  seg_sz,
                                 uint8_t *sw_key,
                                 size_t  sw_key_sz,
                                 bool    first,
                                 bool    last)
{
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  uint32_t data;
  size_t words_in_key;
  bool use_hwio = ENV_cmds_over_hwio(env);
  bool use_pipe_key = CE_HAL_wrapped_key_present(me, CIPHER_WRAPPED_KEY);

  UC_GUARD((cipher_start + cipher_sz) <= seg_sz, UCLIB_ERR_INV_CIPHER_XFER_SZ, env);
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_SEG_SIZE), (uint32_t) cipher_sz);
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_SEG_START), (uint32_t) cipher_start);

  /* Set Key */
  data = 0;
  data |= CE_BAM_get_encr_seg_cfg_settings(me, last);


  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_HW_CIPHER_KEY))
  {
    #if defined HWIO_CRYPTO0_CRYPTO_QC_KEY_ENCR_KEY_USAGE_RULE_ADDR && defined HWIO_CRYPTO0_CRYPTO_OEM_KEY_ENCR_KEY_USAGE_RULE_ADDR
      CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_QC_KEY_ENCR_KEY_USAGE_RULE), data);
      CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_OEM_KEY_ENCR_KEY_USAGE_RULE), data);
    #endif
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENCR_SEG_CFG, USE_HW_KEY_ENCR);
  }
  else if (use_pipe_key || me->legacy_pipe_key_enable)
  {
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENCR_SEG_CFG, USE_PIPE_KEY_ENCR);
  }

  /* Odd keys are only available for pipe keys and for CE engine version > 5.5.0 */
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_USE_ODD_KEYS))
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_ENCR_SEG_CFG, ODD_KEY_SEL);

  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_SEG_CFG), data);

  //ENCR Key
  words_in_key = sw_key_sz / 4;
  for (uint32_t i = 0; i < ENCR_KEY_COUNT; i++)
  {
    data = 0;
    if (sw_key && i < words_in_key)
    {
      COPY_WORD(sw_key, i, data);
    }
    CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_ENCR_KEYn, i), data);
  }

  /* Set Key2 */
  words_in_key = (me->mode == CE_HAL_XTS) ? (sizeof(me->xts_key) / 4) : 0;
  for (uint32_t i = 0; i < ENCR_KEY_COUNT; i++)
  {
    data = 0;
    if (me->mode == CE_HAL_XTS && i < words_in_key)
    {
      COPY_WORD(me->xts_key, i, data);
    }
    CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_ENCR_XTS_KEYn, i), data);
  }

  /* Set XTS DU_SIZE */
  data =
    (me->mode == CE_HAL_XTS) ?
    (uint32_t) me->xts_du_sz :
    0;
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE), data);


  /* Set IV */
  for (int i=0; i<4; i++)
  {
    uint32_t tmp = me->cipher_iv ? me->cipher_iv[i] : 0;
    uint32_t mask = me->mask[i];
    switch (i)
    {
      case 0:
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_CNTR0_IV0), tmp);
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_CNTR_MASK0), mask);
        break;
      case 1:
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_CNTR1_IV1), tmp);
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_CNTR_MASK1), mask);
        break;
      case 2:
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_CNTR2_IV2), tmp);
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_CNTR_MASK2), mask);
        break;
      default:
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_CNTR3_IV3), tmp);
        CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_ENCR_CNTR_MASK), mask);
        break;
    }
  }

  /* Set CCM Init Counter */
  for (int i = 0; i < AES_CCM_CNTR_VECTOR_SIZE; i++)
  {
    data = ((CE_HAL_CCM == me->mode)|| (CE_HAL_GCM == me->mode)) ? me->cntr[i] : 0;
    CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn, i), data);
  }
  if (CE_HAL_are_CE_550_features_supported(me))
  {
    if (me->pattern_size > 0 && me->process_data_size > 0)
    {
      data = 0;
      data |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG, PATT_EN);
      data |= (me->pattern_size - 1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG, PATT_SZ);
      data |= (me->process_data_size - 1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG, PROC_DATA_SZ);
      if (first) // Only enable the partial block offset on the first pattern processed
        data |= me->pattern_offset << CE_HWIO_SHFT(CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG, PATT_OFFSET);

      data = correct_endianness(me, data);
    }
    else {
      data = 0;
    }
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG), data);

    if (me->partial_block_offset > 0)
    {
      data = 0;
      if (first) // Only enable the partial block offset on the first block processed
      {
        data |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG, PARTIAL_EN);
        data |= me->partial_block_offset << CE_HWIO_SHFT(CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG, PARTIAL_BLOCK_OFFSET);
        data = correct_endianness(me, data);
      }
    }
    else {
      data = 0;
    }
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG), data);
  }

  if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_AUTH_ENG_EN))
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_AUTH_SEG_CFG), 0);

  return UCLIB_SUCCESS;
}

static int CE_BAM_set_common_ctx(ce_hal_ctx_t *me, size_t seg_sz)
{
  int ret;
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  uint32_t data = 0;
  bool use_hwio = ENV_cmds_over_hwio(env);

  #ifdef HWIO_CRYPTO0_CRYPTO_STATUS6_ADDR
  /* CRYPTO_STATUS6 is available in Crypto 5.6.3 and from Crypto 5.7 and later */
  if (CE_HW_VERSION(5,6,3) == me->hw_ver || me->hw_ver >= CE_HW_VERSION(5,7,0))
  {
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_STATUS6), 0);
  }
  #endif
  if (me->hw_ver >= CE_HW_VERSION(5,5,2))
  {
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_STATUS4), 0);
    CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_STATUS3), 0);
  }
#ifdef HWIO_CRYPTO0_CRYPTO_STATUS5_ADDR
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_STATUS5), 0);
#endif
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_STATUS2), 0);
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_STATUS), 0);

  data = ((UINT32_C(0) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, MASK_ERR_INTR)) |
          (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, MASK_OP_DONE_INTR)) |
          (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, MASK_DIN_INTR)) |
          (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, MASK_DOUT_INTR)) |
          (UINT32_C(0) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, HIGH_SPD_DATA_EN_N)) |
          (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, LITTLE_ENDIAN_MODE)));

  // Set 8 8-byte beats per each burst = 64 bytes per burst (recommended by SWI doc.)
  data |= (0xE0000 & CE_HWIO_FMSK(CRYPTO0_CRYPTO_CONFIG, REQ_SIZE));

  /* HW CE Config - Little Endian bit */
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_LITTLE_ENDIAN)) {
    data |= (UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, LITTLE_ENDIAN_MODE));
  } else {
    data &= (~(UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, LITTLE_ENDIAN_MODE)));
  }

  // pipe set number config
  data &= ~CE_HWIO_FMSK(CRYPTO0_CRYPTO_CONFIG, PIPE_SET_SELECT);
  data |= (me->pipe_set_num << CE_HWIO_SHFT(CRYPTO0_CRYPTO_CONFIG, PIPE_SET_SELECT));
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_CONFIG), data);

  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_SEG_SIZE), (uint32_t) seg_sz);

  ret = CE_HAL_install_wrapped_keys(me, me->pipe_set_num);
  UC_GUARD(UCLIB_SUCCESS == ret, ret, env);

  return UCLIB_SUCCESS;
}

static int CE_BAM_set_ce_go(ce_hal_ctx_t *me, bool last)
{
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  uint32_t data = 0;
  uintptr_t addr = 0;
  bool use_hwio = ENV_cmds_over_hwio(env);

  // Set up GO command
  data = CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC, CLR_CNTXT);

  // no result dump for va case
  if (CE_BAM_use_result_dump(me, last))
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC, RESULTS_DUMP);

  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_SHK_EN)) {
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC_OEM_KEY, GO);
    addr = CE_HWIO_ADDR(CRYPTO0_CRYPTO_GOPROC_OEM_KEY);
  } else if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_PHK_EN)) {
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC_QC_KEY, GO);
    addr = CE_HWIO_ADDR(CRYPTO0_CRYPTO_GOPROC_QC_KEY);
  } else {
    data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC, GO);
    addr = CE_HWIO_ADDR(CRYPTO0_CRYPTO_GOPROC);
  }
  CE_CMD_WRITE(addr, data);
  return UCLIB_SUCCESS;
}

/**
 * @brief Set up CE context
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param auth_start    [in]    Auth. seg. start offset
 * @param auth_sz       [in]    Auth. seg. size
 * @param cipher_start  [in]    Cipher seg. start offset
 * @param cipher_sz     [in]    Cipher seg. size
 * @param seg_sz        [in]    Total seg. size
 * @param sw_key        [in]    Pointer to SW Key
 * @param sw_key_sz     [in]    SW Key size
 * @param first         [in]    first seg.
 * @param last          [in]    last seg.
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_set_ce_ctx(ce_hal_ctx_t *me,
                      size_t  auth_start,
                      size_t  auth_sz,
                      size_t  cipher_start,
                      size_t  cipher_sz,
                      size_t  seg_sz,
                      uint8_t *sw_key,
                      size_t  sw_key_sz,
                      bool    first,
                      bool    last)
{
  int ret = UCLIB_SUCCESS;
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);

  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);

  /* pipe set number configuration */
  CE_BAM_pipe_init(me);

  /* Clear command list */
  clear_cmd_regs(env, ctx);

  /* Set up CE CFG register */
  UC_GUARD(UCLIB_SUCCESS == (ret =
    CE_BAM_set_common_ctx(me, seg_sz)),
    UCLIB_ERR_INV_BAM_CTX,
    env);

  /* Auth engine set up */
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_AUTH_ENG_EN))
  {
    UC_GUARD(UCLIB_SUCCESS == (ret =
      CE_BAM_set_auth_ctx(
        me,
        auth_start,
        auth_sz,
        seg_sz,
        sw_key,
        sw_key_sz,
        first,
        last)),
     UCLIB_ERR_INV_BAM_CTX,
     env);
  }

  /* Cipher engine set up */
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_CIPHER_ENG_EN))
  {
    UC_GUARD(UCLIB_SUCCESS == (ret =
      CE_BAM_set_cipher_ctx(
        me,
        cipher_start,
        cipher_sz,
        seg_sz,
        sw_key,
        sw_key_sz,
        first,
        last)),
      UCLIB_ERR_INV_BAM_CTX,
      env);
  }

  /* Set up GOPROC register */
  UC_GUARD(UCLIB_SUCCESS == (ret =
    CE_BAM_set_ce_go(me, last)),
    UCLIB_ERR_INV_BAM_CTX,
    env);

  ENV_mem_barrier(env);

  return CE_BAM_set_up_xfer(me);
}

/**
 * @brief Save CE ctx to internal HAL context
 *
 * @param me        [in]    Pointer to CE HAL context
 * @param blocking  [in]    Boolean indicating whether to poll for completion
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_get_ce_ctx(ce_hal_ctx_t *me, bool blocking, bool last)
{
  int ret;
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);

  /* Get CE Result */
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_queue_result(me, last)), ret, me->env);

  blocking =
    blocking ||
    (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_NON_BLOCKING));

  ret = CE_BAM_wait_for_xfer(me, blocking);
  if (!blocking && CE_BAM_is_busy_err_code(ret))
  {
    me->xfer_pending = true;
    return UCLIB_SUCCESS;
  }

  UC_GUARD(!CE_BAM_is_fatal_err_code(ret, blocking), ret, me->env);
  int ce_status = ret;

  /* Update output buffer if valid */
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_buf_cpy_align_buf(me)), ret, me->env);

  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_NON_BLOCKING) && !last)
  {
    return ce_status;
  }

  UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_save_ctx(me, last, true)), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_toggle_engine_lock(me, false)), ret, me->env);
  UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_save_ctx(me, last, false)), ret, me->env);
  return ce_status;
}

/**
 * Configure a BAM pipe with a key, key usage rules
 * This API configures the legacy pipe key registers not HWKM key slots.
 *
 * @param me         [in] Pointer to CE HAL context. Should have all configuration options desired for the pipe.
 * @param pipe_id    [in] The pipe to configure. Should be in range [0,7] since there are only 8 pipes.
 * @param sw_key     [in] Pointer to the key to configure the pipe to use.
 * @param sw_key_sz  [in] Size of the key in bytes.
 *
 * @return UCLIB_SUCCESS if successful.
 */
int CE_HAL_set_pipe_key_legacy
(
  ce_hal_ctx_t* me,
  uint32_t pipe,
  const uint8_t* key,
  size_t key_sz
)
{
/* This is legacy function which is not applicable for GPCE ver 5.8 and above */
  #if defined CE_VER_0507_N_BELOW || defined CE_050502
  int ret;
  env_t* env = me->env;

  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  bool use_hwio = false;
  bool use_odd_keys = CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_USE_ODD_KEYS);

  /* VA usecase - uses HWIO to write to pipe config regs.
   * We are performing check based on VA OP and not based on pipeset here
   * because for use cases like in UEFI, where pipeset 2 is used, performing
   * HWIO operation to write to pipe config regs would lead to crash
   *
   * For non-VA mode, configuring pipe config registers for pipeset > 0 using
   * BAM causes xpu violation
   */
  if (CE_HAL_is_va_op(me))
  {
    use_hwio = true;
  }
  else if (0 == pipe && CE_HAL_are_CE_550_features_supported(me))
  {
    use_hwio = ENV_cmds_over_hwio(env);
  }
  else
  {
    return UCLIB_ERR_NOT_SUPPORTED;
  }

  /* If this is standalone mode, clear any other register values that might
   * be in the command set
   */
  if (!use_hwio)
  {
    clear_cmd_regs(env, ctx);
  }

  uint32_t usage_rules = 0;
  usage_rules |= CE_BAM_get_encr_seg_cfg_settings(me, false);
  #if defined HWIO_CRYPTO0_CRYPTO_ENCR_PIPEm_ODD_KEY_USAGE_RULES_ADDR && defined HWIO_CRYPTO0_CRYPTO_ENCR_PIPEm_EVEN_KEY_USAGE_RULES_ADDR
    if(use_odd_keys)
    {
      CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_ENCR_PIPEm_ODD_KEY_USAGE_RULES, pipe), usage_rules);
    }
    else {
      CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_ENCR_PIPEm_EVEN_KEY_USAGE_RULES, pipe), usage_rules);
    }
  #elif defined HWIO_CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_USAGE_RULES_ADDR
    usage_rules |= UINT32_C(1) << CE_HWIO_SHFT(CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_USAGE_RULES, ENABLE);
    CE_CMD_WRITE(CE_HWIO_ADDRI(CRYPTO0_CRYPTO_ENCR_PIPEm_KEY_USAGE_RULES, pipe), usage_rules);
  #endif
  uint32_t words_in_key = MIN(key_sz / sizeof(uint32_t), ENCR_KEY_COUNT);
  for (uint32_t i = 0; i < words_in_key; i++)
  {
    uintptr_t addr = 0;
    uint32_t data = 0;

    if (key)
      COPY_WORD(key, i, data);
#ifdef HWIO_CRYPTO0_CRYPTO_ENCR_PIPEm_ODD_KEYn_ADDR
    addr =
      use_odd_keys ?
      CE_HWIO_ADDRI2(CRYPTO0_CRYPTO_ENCR_PIPEm_ODD_KEYn, pipe, i) :
      CE_HWIO_ADDRI2(CRYPTO0_CRYPTO_ENCR_PIPEm_KEYn, pipe, i);
    CE_CMD_WRITE(addr, data);
#endif
    if (CE_HAL_XTS == me->mode)
    {
      if (IS_SIZE_ALIGNED(me->xts_key, 4))
      {
        data = (CAST_PTR_TYPE(me->xts_key, uint32_t))[i];
      }
      else {
        ENV_sec_memcpy(env, &data, sizeof(data), (me->xts_key + i * sizeof(data)), sizeof(data));
      }
#ifdef HWIO_CRYPTO0_CRYPTO_ENCR_PIPEm_ODD_KEYn_ADDR
      addr =
        use_odd_keys ?
        CE_HWIO_ADDRI2(CRYPTO0_CRYPTO_ENCR_PIPEm_XTS_ODD_KEYn, pipe, i) :
        CE_HWIO_ADDRI2(CRYPTO0_CRYPTO_ENCR_PIPEm_XTS_KEYn, pipe, i);
      CE_CMD_WRITE(addr, data);
#endif
    }
  }

  if (!use_hwio)
  {
    // Send the configuration commands
    CE_HAL_SET_FLAGS(me, CE_HAL_FLAGS_IS_CFG_OP);
    ENV_mem_barrier(env);
    UC_GUARD(CE_BAM_set_up_xfer(me) == UCLIB_SUCCESS, UCLIB_ERR_FAILURE, env);
    if (ENV_is_bam_polling_mode(env)) {
      UC_GUARD(UCLIB_SUCCESS == CE_BAM_polling(me, true, true), UCLIB_ERR_BAM_TX_PIPE_POLL, me->env);
    }
    else {
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_isr_wait(env)), ret, env);
    }
    CE_HAL_CLR_FLAGS(me, CE_HAL_FLAGS_IS_CFG_OP);
  }
  /** we are setting this flag to enable the pipe key encr for legacy pipe keys in ENCR_SEG_CFG **/
  me->legacy_pipe_key_enable = 1;
  #endif

  return UCLIB_SUCCESS;
}

/**
 * @brief Transfer data to/from CE engine
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param ibuf          [in]    Pointer to input buffer
 * @param isz           [in]    Input buffer size
 * @param obuf          [in]    Pointer to output buffer
 * @param osz           [in]    Output buffer size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_xfer_data(ce_hal_ctx_t *me, const uint8_t *ibuf, size_t isz, uint8_t *obuf, size_t osz)
{
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);

  // Queue input buffer to BAM TX pipe
  if (ibuf && isz) {
    size_t xfer_cnt = isz/CE_BAM_MAX_XFER_SIZE;
    xfer_cnt = (isz % CE_BAM_MAX_XFER_SIZE)?(xfer_cnt + 1):xfer_cnt;
    while (xfer_cnt) {
      int ret;
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_queue_xfer(me,
                                                         ibuf,
                                                         (isz > CE_BAM_MAX_XFER_SIZE)?CE_BAM_MAX_XFER_SIZE:isz,
                                                         (isz > CE_BAM_MAX_XFER_SIZE)?NEXT_PIPE_XFER_REQ:LAST_PIPE_XFER_REQ,
                                                         QUEUE_TX_PIPE_XFER)),
               ret, me->env);
      if (isz > CE_BAM_MAX_XFER_SIZE) {
        isz -= CE_BAM_MAX_XFER_SIZE;
        ibuf += CE_BAM_MAX_XFER_SIZE;
      }
      xfer_cnt--;
    }
  }

  // Queue output buffer to BAM RX pipe
  if (obuf && osz) {
    int ret;
    if (me->va_op != VA_OP_INVALID) {
      size_t xfer_cnt = (osz + CE_BAM_MAX_XFER_SIZE - 1) /CE_BAM_MAX_XFER_SIZE;
      while (xfer_cnt) {
        UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_queue_xfer(me,
                                                           obuf,
                                                           (osz > CE_BAM_MAX_XFER_SIZE)?CE_BAM_MAX_XFER_SIZE:osz,
                                                           (osz > CE_BAM_MAX_XFER_SIZE)?NEXT_PIPE_XFER_REQ:LAST_PIPE_XFER_REQ,
                                                           QUEUE_RX_PIPE_XFER)),
                                   ret, me->env);
        if (osz > CE_BAM_MAX_XFER_SIZE) {
          osz -= CE_BAM_MAX_XFER_SIZE;
          obuf += CE_BAM_MAX_XFER_SIZE;
        }
        xfer_cnt--;
      }
    } else {
      UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_aligned_buf_init(me, obuf, osz)), ret, me->env);
      bool last_buf = false;
      for (align_buf_t i=BLK_BUF_HEADER; !last_buf && i<=BLK_BUF_TAILER; i++) {
        uint8_t *ca_obuf = NULL; // Pointer to Cache line aligned buffer
        size_t ca_osz = 0;       // Size of Cache line aligned buffer

        UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_get_align_buf(me, i, &ca_obuf, &ca_osz, &last_buf)), ret, me->env);
        if (!ca_osz)
          continue;

        size_t xfer_cnt = ca_osz/CE_BAM_MAX_XFER_SIZE;
        xfer_cnt = (ca_osz % CE_BAM_MAX_XFER_SIZE)?(xfer_cnt + 1):xfer_cnt;
        while (xfer_cnt) {
          UC_GUARD(UCLIB_SUCCESS == (ret = CE_BAM_queue_xfer(me,
                                                             ca_obuf,
                                                             (ca_osz > CE_BAM_MAX_XFER_SIZE)?CE_BAM_MAX_XFER_SIZE:ca_osz,
                                                             (ca_osz <= CE_BAM_MAX_XFER_SIZE && (last_buf || BLK_BUF_TAILER == i))? LAST_PIPE_XFER_REQ : NEXT_PIPE_XFER_REQ,
                                                             QUEUE_RX_PIPE_XFER)),
                   ret, me->env);
          if (ca_osz >= CE_BAM_MAX_XFER_SIZE) {
            ca_osz -= CE_BAM_MAX_XFER_SIZE;
            ca_obuf += CE_BAM_MAX_XFER_SIZE;
          }
          xfer_cnt--;
        }
      }
    }
  }

  return UCLIB_SUCCESS;
}

/**
 * @brief Set up CE context for SHAKE Squeeze
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param sqz_cnt       [in]    number of squeeze needed
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_set_ce_squeeze_ctx(ce_hal_ctx_t *me, size_t sqz_cnt)
{
  int ret = UCLIB_SUCCESS;
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD((me->hw_ver >= CE_HW_VERSION(5,9,0) && (me->alg >= CE_HAL_SHAKE128 && me->alg <= CE_HAL_KMAC256)), UCLIB_ERR_NOT_SUPPORTED, NULL);
  me->sqz_cnt = sqz_cnt;
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);

  /* pipe set number configuration */
  CE_BAM_pipe_init(me);

  /* Clear command list */
  clear_cmd_regs(env, ctx);

  /* Set up CE CFG register with seg_sz = 0 */
  UC_GUARD(UCLIB_SUCCESS == (ret =
    CE_BAM_set_common_ctx(me, 0)),
    UCLIB_ERR_INV_BAM_CTX,
    env);

  /* Auth engine set up */
  if (CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_AUTH_ENG_EN))
  {
    UC_GUARD(UCLIB_SUCCESS == (ret =
      CE_BAM_set_auth_ctx(
        me,
        0,
        0,
        0,
        NULL,
        0,
        false,
        false)),
     UCLIB_ERR_INV_BAM_CTX,
     env);
  }

  // Set up GO command
  bool use_hwio = ENV_cmds_over_hwio(env);
  uint32_t data = 0;
  data = CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC, CLR_CNTXT);
  data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC, RESULTS_DUMP);
#ifdef HWIO_CRYPTO0_CRYPTO_GOPROC_RESULTS_DUMP_USE_OUT_SID_BMSK
  data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC, RESULTS_DUMP_USE_OUT_SID);
#endif
  data |= CE_HWIO_FMSK(CRYPTO0_CRYPTO_GOPROC, GO);
  CE_CMD_WRITE(CE_HWIO_ADDR(CRYPTO0_CRYPTO_GOPROC), data);

  ENV_mem_barrier(env);

  return CE_BAM_set_up_xfer(me);
}

/**
 * @brief Reqest RX to save the squeeze data to the output buff
 *
 * @param me       [in]    Pointer to CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_get_ce_squeeze_data_toRsltDump(ce_hal_ctx_t *me)
{
  int ret = UCLIB_SUCCESS;
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD((me->hw_ver >= CE_HW_VERSION(5,9,0) && (me->alg >= CE_HAL_SHAKE128 && me->alg <= CE_HAL_KMAC256)), UCLIB_ERR_NOT_SUPPORTED, NULL);

  //Flush result buffer if cached
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  if (!CE_HAL_IS_FLAG_SET(me, CE_HAL_FLAGS_IS_RSLT_BUF_UNCACHED)) {
    ENV_dcache_clean_and_invalidate_region(env, (void *)ctx->ce_rslt_dump_ptr.va, ENV_BAM_get_max_rlst_dump_cnt(env) * sizeof(ce_result_shake_t));
  }

  UC_GUARD(BAM_SUCCESS == CE_BAM_pipe_transfer(me,
                                               ctx->ce_rslt_dump_ptr.pa,
                                               ENV_BAM_get_max_rlst_dump_cnt(env) * sizeof(ce_result_shake_t),
                                               (BAM_IOVEC_FLAG_INT|BAM_IOVEC_FLAG_EOT),
                                               RX_PIPE_FLG,
                                               ctx),
                                               UCLIB_ERR_BAM_XFR_RSLT_DSC, env);

  ret = CE_BAM_wait_for_xfer(me, true);
  UC_GUARD(!CE_BAM_is_fatal_err_code(ret, true), ret, env);

  /* Release the lock */
  return CE_BAM_toggle_engine_lock(me, false);
}

/**
 * @brief Save the result dump data to the output buf and save CE ctx to internal HAL context.
 *
 * @param me       [in]    Pointer to CE HAL context
 * @param md       [in]    Pointer to output digest buffer
 * @param md_sz    [in]    Output digest buffer size
 * @param out_sz   [out]   Pointer to Output data size
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_process_rslt_dump_data(ce_hal_ctx_t *me, uint8_t *md, size_t md_sz, size_t *out_sz)
{
  int ret = UCLIB_SUCCESS;
  UC_GUARD(CE_HAL_IS_CTX_VALID(me), UCLIB_ERR_CE_HAL_CTX_INVALID, NULL);
  UC_GUARD((me->hw_ver >= CE_HW_VERSION(5,9,0) && (me->alg >= CE_HAL_SHAKE128 && me->alg <= CE_HAL_KMAC256)), UCLIB_ERR_NOT_SUPPORTED, NULL);
  size_t rate_sz = (CE_HAL_SHAKE128 == me->alg || CE_HAL_CSHAKE128 == me->alg || CE_HAL_KMAC128 == me->alg) ? CE_HW_SHAKE128_BLOCK_SIZE : CE_HW_SHAKE256_BLOCK_SIZE;
  env_t *env = me->env;
  void *bam_drv_ctx = ENV_get_driver_ctx(env, GET, ENV_INIT_BAM_DRV_CTX, NULL);
  UC_GUARD(bam_drv_ctx, UCLIB_ERR_INV_BAM_CTX, env);
  BAM_DRIVER_CTX_t *ctx = CAST_PTR_TYPE(bam_drv_ctx, BAM_DRIVER_CTX_t);
  ce_result_shake_t *rslt = (ce_result_shake_t *)ctx->ce_rslt_dump_ptr.va;
  UC_GUARD(rslt, UCLIB_ERR_INV_BAM_CTX, env);
  uint8_t *tmp_rslt = (uint8_t*)rslt;
  uint8_t *tmp_md = md;
  size_t tmp_sz = md_sz;
  size_t sqz_cnt = 0;
  size_t cp_sz = 0;
  *out_sz = 0;

  do {
    /* copy data from the result dump */
    tmp_rslt += (sqz_cnt * RESULT_SIZE_PER_SQZ);
    cp_sz = ENV_sec_memcpy(env, tmp_md, tmp_sz, tmp_rslt, rate_sz);
    tmp_md += cp_sz;
    tmp_sz -= cp_sz;
    *out_sz += cp_sz;
    sqz_cnt++;
  } while (sqz_cnt < me->sqz_cnt && *out_sz < md_sz);

  /* update the IV from the last run in the result dump */
  int i = 0;
  rslt = (ce_result_shake_t*)tmp_rslt;

  for (i = 0; i < MAX_AUTH_IV_COUNT; i++) {
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_iv(me, i, rslt->CRYPTO_AUTH_IV[i])), ret, env);
  }

  for (i = 0; i < MAX_AUTH_IVM_COUNT; i++) {
    UC_GUARD(UCLIB_SUCCESS == (ret = CE_HAL_set_auth_ivm(me, i, rslt->CRYPTO_AUTH_IV[MAX_AUTH_IV_COUNT + i])), ret, env);
  }

  return UCLIB_SUCCESS;
}
