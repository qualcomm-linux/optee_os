// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * Crypto Engine BAM HAL definitions.
 *
 * Defines the interfaces used to configure the Qualcomm
 * Crypto Engine through the BAM transport layer, including
 * context programming, data transfer, and result handling.
 */

#ifndef __UCLIB_PRV_CE_BAM_HAL
#define __UCLIB_PRV_CE_BAM_HAL

#include "ce_hal.h"

/*===========================================================================
                 DEFINITIONS AND TYPE DECLARATIONS
 ===========================================================================*/
/**
 * @brief Initializes CE BAM driver.
 *
 * @param me            [in]    Pointer to CE HAL context
 *
 * @return 0 if successful, nagative value otherwise.
 */
int CE_BAM_init(ce_hal_ctx_t *me);

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
 * @param first         [in]    fisrt seg.
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
                      bool    last);

/**
 * @brief Save CE ctx to internal HAL context
 *
 * @param me        [in]    Pointer to CE HAL context
 * @param blocking  [in]    Boolean indicating whether to poll for completion
 * @param last      [in]    Boolean indicating the last data segment
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_get_ce_ctx(ce_hal_ctx_t *me, bool blocking, bool last);

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
int CE_BAM_xfer_data(ce_hal_ctx_t *me, const uint8_t *ibuf, size_t isz, uint8_t *obuf, size_t osz);

/**
 * @brief Set up CE context for SHAKE Squeeze
 *
 * @param me            [in]    Pointer to CE HAL context
 * @param sqz_cnt       [in]    number of squeeze needed
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_set_ce_squeeze_ctx(ce_hal_ctx_t *me, size_t sqz_cnt);

/**
 * @brief Reqest RX to save the squeeze data to the output buff
 *
 * @param me       [in]    Pointer to CE HAL context
 *
 * @return 0 if successful. negative value otherwise.
 */
int CE_BAM_get_ce_squeeze_data_toRsltDump(ce_hal_ctx_t *me);

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
int CE_BAM_process_rslt_dump_data(ce_hal_ctx_t *me, uint8_t *md, size_t md_sz, size_t *out_sz);

#endif /* __UCLIB_PRV_CE_BAM_HAL */
