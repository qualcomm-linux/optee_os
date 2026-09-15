// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 *
 * QCE HWIO register definitions.
 *
 * Auto-generated register definitions for the Qualcomm
 * Crypto Engine (QCE) and BAM hardware blocks.
 *
 * This file is generated from IPCAT and should not be
 * modified manually.
 */

#ifndef HWIO_H
#define HWIO_H
/*
===========================================================================
*/
/**
    @file hwio.h
    @brief Auto-generated HWIO interface include file.

    Reference chip release:
        SA8797P (NordAU) [nordschleife_v1.1_p3q3r98_MTO_SOCCP]
 
    This file contains HWIO register definitions for the following modules:
        CRYPTO0_CRYPTO
        CRYPTO0_CRYPTO_BAM


    Generation parameters: 
    { 'base-references': False,
      'filename': 'hwio.h',
      'modules': ['CRYPTO0_CRYPTO', 'CRYPTO0_CRYPTO_BAM']}
*/

/*----------------------------------------------------------------------------
 * MODULE: CRYPTO0_CRYPTO
 *--------------------------------------------------------------------------*/

#define CRYPTO0_CRYPTO_REG_BASE                                                              0x01dfa000
#define CRYPTO0_CRYPTO_REG_BASE_SIZE                                                         0x6000
#define CRYPTO0_CRYPTO_REG_BASE_USED                                                         0x5200

#define HWIO_CRYPTO0_CRYPTO_VERSION_ADDR                                                     (CRYPTO0_CRYPTO_REG_BASE      + 0x0)
#define HWIO_CRYPTO0_CRYPTO_VERSION_RMSK                                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_VERSION_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_VERSION_ADDR)
#define HWIO_CRYPTO0_CRYPTO_VERSION_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_VERSION_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_VERSION_MAJ_VER_BMSK                                             0xff000000
#define HWIO_CRYPTO0_CRYPTO_VERSION_MAJ_VER_SHFT                                                     24
#define HWIO_CRYPTO0_CRYPTO_VERSION_MIN_VER_BMSK                                               0xff0000
#define HWIO_CRYPTO0_CRYPTO_VERSION_MIN_VER_SHFT                                                     16
#define HWIO_CRYPTO0_CRYPTO_VERSION_STEP_VER_BMSK                                                0xffff
#define HWIO_CRYPTO0_CRYPTO_VERSION_STEP_VER_SHFT                                                     0

#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_ADDR                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0x4)
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_RMSK                                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_LIB_VERSION_ADDR)
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_LIB_VERSION_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_MAJ_VER_BMSK                                         0xff000000
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_MAJ_VER_SHFT                                                 24
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_MIN_VER_BMSK                                           0xff0000
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_MIN_VER_SHFT                                                 16
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_STEP_VER_BMSK                                            0xffff
#define HWIO_CRYPTO0_CRYPTO_LIB_VERSION_STEP_VER_SHFT                                                 0

#define HWIO_CRYPTO0_CRYPTO_FIPS_ID_ADDR                                                     (CRYPTO0_CRYPTO_REG_BASE      + 0xc)
#define HWIO_CRYPTO0_CRYPTO_FIPS_ID_RMSK                                                         0xffff
#define HWIO_CRYPTO0_CRYPTO_FIPS_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_FIPS_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_FIPS_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_FIPS_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_FIPS_ID_FIPS_ID_BMSK                                                 0xffff
#define HWIO_CRYPTO0_CRYPTO_FIPS_ID_FIPS_ID_SHFT                                                      0

#define HWIO_CRYPTO0_CRYPTO_DATA_INn_ADDR(n)                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0X10 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_DATA_INn_RMSK                                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_DATA_INn_MAXn                                                             3
#define HWIO_CRYPTO0_CRYPTO_DATA_INn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_DATA_INn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_DATA_INn_DATA_IN_BMSK                                            0xffffffff
#define HWIO_CRYPTO0_CRYPTO_DATA_INn_DATA_IN_SHFT                                                     0

#define HWIO_CRYPTO0_CRYPTO_DATA_OUTn_ADDR(n)                                                (CRYPTO0_CRYPTO_REG_BASE      + 0X20 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_DATA_OUTn_RMSK                                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_DATA_OUTn_MAXn                                                            3
#define HWIO_CRYPTO0_CRYPTO_DATA_OUTn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_DATA_OUTn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_DATA_OUTn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_DATA_OUTn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_DATA_OUTn_DATA_OUT_BMSK                                          0xffffffff
#define HWIO_CRYPTO0_CRYPTO_DATA_OUTn_DATA_OUT_SHFT                                                   0

#define HWIO_CRYPTO0_CRYPTO_STATUS_ADDR                                                      (CRYPTO0_CRYPTO_REG_BASE      + 0x100)
#define HWIO_CRYPTO0_CRYPTO_STATUS_RMSK                                                      0xffffffff
#define HWIO_CRYPTO0_CRYPTO_STATUS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_STATUS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_STATUS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_STATUS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_STATUS_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_STATUS_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_STATUS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_STATUS_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_STATUS_IN)
#define HWIO_CRYPTO0_CRYPTO_STATUS_MAC_FAILED_BMSK                                           0x80000000
#define HWIO_CRYPTO0_CRYPTO_STATUS_MAC_FAILED_SHFT                                                   31
#define HWIO_CRYPTO0_CRYPTO_STATUS_DOUT_SIZE_AVAIL_BMSK                                      0x7c000000
#define HWIO_CRYPTO0_CRYPTO_STATUS_DOUT_SIZE_AVAIL_SHFT                                              26
#define HWIO_CRYPTO0_CRYPTO_STATUS_DIN_SIZE_AVAIL_BMSK                                        0x3e00000
#define HWIO_CRYPTO0_CRYPTO_STATUS_DIN_SIZE_AVAIL_SHFT                                               21
#define HWIO_CRYPTO0_CRYPTO_STATUS_HSD_ERR_BMSK                                                0x100000
#define HWIO_CRYPTO0_CRYPTO_STATUS_HSD_ERR_SHFT                                                      20
#define HWIO_CRYPTO0_CRYPTO_STATUS_ACCESS_VIOL_BMSK                                             0x80000
#define HWIO_CRYPTO0_CRYPTO_STATUS_ACCESS_VIOL_SHFT                                                  19
#define HWIO_CRYPTO0_CRYPTO_STATUS_PIPE_ACTIVE_ERR_BMSK                                         0x40000
#define HWIO_CRYPTO0_CRYPTO_STATUS_PIPE_ACTIVE_ERR_SHFT                                              18
#define HWIO_CRYPTO0_CRYPTO_STATUS_CFG_CHNG_ERR_BMSK                                            0x20000
#define HWIO_CRYPTO0_CRYPTO_STATUS_CFG_CHNG_ERR_SHFT                                                 17
#define HWIO_CRYPTO0_CRYPTO_STATUS_DOUT_ERR_BMSK                                                0x10000
#define HWIO_CRYPTO0_CRYPTO_STATUS_DOUT_ERR_SHFT                                                     16
#define HWIO_CRYPTO0_CRYPTO_STATUS_DIN_ERR_BMSK                                                  0x8000
#define HWIO_CRYPTO0_CRYPTO_STATUS_DIN_ERR_SHFT                                                      15
#define HWIO_CRYPTO0_CRYPTO_STATUS_AXI_ERR_BMSK                                                  0x4000
#define HWIO_CRYPTO0_CRYPTO_STATUS_AXI_ERR_SHFT                                                      14
#define HWIO_CRYPTO0_CRYPTO_STATUS_CRYPTO_STATE_BMSK                                             0x3c00
#define HWIO_CRYPTO0_CRYPTO_STATUS_CRYPTO_STATE_SHFT                                                 10
#define HWIO_CRYPTO0_CRYPTO_STATUS_ENCR_BUSY_BMSK                                                 0x200
#define HWIO_CRYPTO0_CRYPTO_STATUS_ENCR_BUSY_SHFT                                                     9
#define HWIO_CRYPTO0_CRYPTO_STATUS_AUTH_BUSY_BMSK                                                 0x100
#define HWIO_CRYPTO0_CRYPTO_STATUS_AUTH_BUSY_SHFT                                                     8
#define HWIO_CRYPTO0_CRYPTO_STATUS_DOUT_INTR_BMSK                                                  0x80
#define HWIO_CRYPTO0_CRYPTO_STATUS_DOUT_INTR_SHFT                                                     7
#define HWIO_CRYPTO0_CRYPTO_STATUS_DIN_INTR_BMSK                                                   0x40
#define HWIO_CRYPTO0_CRYPTO_STATUS_DIN_INTR_SHFT                                                      6
#define HWIO_CRYPTO0_CRYPTO_STATUS_OP_DONE_INTR_BMSK                                               0x20
#define HWIO_CRYPTO0_CRYPTO_STATUS_OP_DONE_INTR_SHFT                                                  5
#define HWIO_CRYPTO0_CRYPTO_STATUS_ERR_INTR_BMSK                                                   0x10
#define HWIO_CRYPTO0_CRYPTO_STATUS_ERR_INTR_SHFT                                                      4
#define HWIO_CRYPTO0_CRYPTO_STATUS_DOUT_RDY_BMSK                                                    0x8
#define HWIO_CRYPTO0_CRYPTO_STATUS_DOUT_RDY_SHFT                                                      3
#define HWIO_CRYPTO0_CRYPTO_STATUS_DIN_RDY_BMSK                                                     0x4
#define HWIO_CRYPTO0_CRYPTO_STATUS_DIN_RDY_SHFT                                                       2
#define HWIO_CRYPTO0_CRYPTO_STATUS_OPERATION_DONE_BMSK                                              0x2
#define HWIO_CRYPTO0_CRYPTO_STATUS_OPERATION_DONE_SHFT                                                1
#define HWIO_CRYPTO0_CRYPTO_STATUS_SW_ERR_BMSK                                                      0x1
#define HWIO_CRYPTO0_CRYPTO_STATUS_SW_ERR_SHFT                                                        0

#define HWIO_CRYPTO0_CRYPTO_STATUS2_ADDR                                                     (CRYPTO0_CRYPTO_REG_BASE      + 0x104)
#define HWIO_CRYPTO0_CRYPTO_STATUS2_RMSK                                                     0xf901000f
#define HWIO_CRYPTO0_CRYPTO_STATUS2_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_STATUS2_ADDR)
#define HWIO_CRYPTO0_CRYPTO_STATUS2_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_STATUS2_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_STATUS2_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_STATUS2_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_STATUS2_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_STATUS2_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_STATUS2_IN)
#define HWIO_CRYPTO0_CRYPTO_STATUS2_BIST_ERROR_BMSK                                          0x80000000
#define HWIO_CRYPTO0_CRYPTO_STATUS2_BIST_ERROR_SHFT                                                  31
#define HWIO_CRYPTO0_CRYPTO_STATUS2_BIST_BUSY_BMSK                                           0x40000000
#define HWIO_CRYPTO0_CRYPTO_STATUS2_BIST_BUSY_SHFT                                                   30
#define HWIO_CRYPTO0_CRYPTO_STATUS2_KEY_ERR_BMSK                                             0x20000000
#define HWIO_CRYPTO0_CRYPTO_STATUS2_KEY_ERR_SHFT                                                     29
#define HWIO_CRYPTO0_CRYPTO_STATUS2_PIPE_KEY_USAGE_ERR_BMSK                                  0x10000000
#define HWIO_CRYPTO0_CRYPTO_STATUS2_PIPE_KEY_USAGE_ERR_SHFT                                          28
#define HWIO_CRYPTO0_CRYPTO_STATUS2_HW_KEY_USAGE_ERR_BMSK                                     0x8000000
#define HWIO_CRYPTO0_CRYPTO_STATUS2_HW_KEY_USAGE_ERR_SHFT                                            27
#define HWIO_CRYPTO0_CRYPTO_STATUS2_PIPE_KEY_TIMER_ERR_BMSK                                   0x1000000
#define HWIO_CRYPTO0_CRYPTO_STATUS2_PIPE_KEY_TIMER_ERR_SHFT                                          24
#define HWIO_CRYPTO0_CRYPTO_STATUS2_PIPE_KEY_PAUSE_ERR_BMSK                                     0x10000
#define HWIO_CRYPTO0_CRYPTO_STATUS2_PIPE_KEY_PAUSE_ERR_SHFT                                          16
#define HWIO_CRYPTO0_CRYPTO_STATUS2_HMAC_KEY_ERR_BMSK                                               0x8
#define HWIO_CRYPTO0_CRYPTO_STATUS2_HMAC_KEY_ERR_SHFT                                                 3
#define HWIO_CRYPTO0_CRYPTO_STATUS2_INVALID_ALGO_CFG_BMSK                                           0x4
#define HWIO_CRYPTO0_CRYPTO_STATUS2_INVALID_ALGO_CFG_SHFT                                             2
#define HWIO_CRYPTO0_CRYPTO_STATUS2_AXI_EXTRA_BMSK                                                  0x2
#define HWIO_CRYPTO0_CRYPTO_STATUS2_AXI_EXTRA_SHFT                                                    1
#define HWIO_CRYPTO0_CRYPTO_STATUS2_LOCKED_BMSK                                                     0x1
#define HWIO_CRYPTO0_CRYPTO_STATUS2_LOCKED_SHFT                                                       0

#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x108)
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_RMSK                                               0xfc07ffff
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_AES_NUM_DP_BMSK                               0xe0000000
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_AES_NUM_DP_SHFT                                       29
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ZUC_ENABLE_BMSK                                    0x10000000
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ZUC_ENABLE_SHFT                                            28
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_ZUC_SEL_BMSK                                   0x8000000
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_ZUC_SEL_SHFT                                          27
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_ZUC_SEL_BMSK                                   0x4000000
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_ZUC_SEL_SHFT                                          26
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_MAX_AXI_WR_BEATS_BMSK                                 0x7e000
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_MAX_AXI_WR_BEATS_SHFT                                      13
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_NUM_BAM_PIPE_SETS_BMSK                                 0x1e00
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_NUM_BAM_PIPE_SETS_SHFT                                      9
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_KASUMI_SEL_BMSK                                    0x100
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_KASUMI_SEL_SHFT                                        8
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_SNOW3G_SEL_BMSK                                     0x80
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_SNOW3G_SEL_SHFT                                        7
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_AES_SEL_BMSK                                        0x40
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_AES_SEL_SHFT                                           6
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_SHA512_SEL_BMSK                                     0x20
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_SHA512_SEL_SHFT                                        5
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_SHA_SEL_BMSK                                        0x10
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_AUTH_SHA_SEL_SHFT                                           4
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_KASUMI_SEL_BMSK                                      0x8
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_KASUMI_SEL_SHFT                                        3
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_SNOW3G_SEL_BMSK                                      0x4
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_SNOW3G_SEL_SHFT                                        2
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_DES_SEL_BMSK                                         0x2
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_DES_SEL_SHFT                                           1
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_AES_SEL_BMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL_ENCR_AES_SEL_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_ADDR                                                  (CRYPTO0_CRYPTO_REG_BASE      + 0x10c)
#define HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_RMSK                                                      0xffff
#define HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_ADDR)
#define HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_ENG_DOUT_FIFO_DEPTH_BMSK                                  0xff00
#define HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_ENG_DOUT_FIFO_DEPTH_SHFT                                       8
#define HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_ENG_DIN_FIFO_DEPTH_BMSK                                     0xff
#define HWIO_CRYPTO0_CRYPTO_FIFO_SIZES_ENG_DIN_FIFO_DEPTH_SHFT                                        0

#define HWIO_CRYPTO0_CRYPTO_SEG_SIZE_ADDR                                                    (CRYPTO0_CRYPTO_REG_BASE      + 0x110)
#define HWIO_CRYPTO0_CRYPTO_SEG_SIZE_RMSK                                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_SEG_SIZE_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_SEG_SIZE_ADDR)
#define HWIO_CRYPTO0_CRYPTO_SEG_SIZE_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_SEG_SIZE_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_SEG_SIZE_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_SEG_SIZE_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_SEG_SIZE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_SEG_SIZE_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_SEG_SIZE_IN)
#define HWIO_CRYPTO0_CRYPTO_SEG_SIZE_SEG_SIZE_BMSK                                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_SEG_SIZE_SEG_SIZE_SHFT                                                    0

#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_ADDR                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0x114)
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_RMSK                                                 0xfffcffff
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BIST_STATUS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BIST_STATUS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CBC_TDES_DEC_ERR_BMSK                           0x80000000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CBC_TDES_DEC_ERR_SHFT                                   31
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CBC_TDES_ENC_ERR_BMSK                           0x40000000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CBC_TDES_ENC_ERR_SHFT                                   30
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_XTS_AES_DEC_ERR_BMSK                            0x20000000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_XTS_AES_DEC_ERR_SHFT                                    29
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_XTS_AES_ENC_ERR_BMSK                            0x10000000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_XTS_AES_ENC_ERR_SHFT                                    28
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CTR_AES_DEC_ERR_BMSK                             0x8000000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CTR_AES_DEC_ERR_SHFT                                    27
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CTR_AES_ENC_ERR_BMSK                             0x4000000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CTR_AES_ENC_ERR_SHFT                                    26
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CBC_AES_DEC_ERR_BMSK                             0x2000000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CBC_AES_DEC_ERR_SHFT                                    25
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CBC_AES_ENC_ERR_BMSK                             0x1000000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CBC_AES_ENC_ERR_SHFT                                    24
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_SHAKE_256_ERR_BMSK                                0x800000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_SHAKE_256_ERR_SHFT                                      23
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_SHAKE_128_ERR_BMSK                                0x400000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_SHAKE_128_ERR_SHFT                                      22
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA3_384_ERR_BMSK                            0x200000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA3_384_ERR_SHFT                                  21
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA3_256_ERR_BMSK                            0x100000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA3_256_ERR_SHFT                                  20
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA3_224_ERR_BMSK                             0x80000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA3_224_ERR_SHFT                                  19
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA3_512_ERR_BMSK                             0x40000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA3_512_ERR_SHFT                                  18
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA_512_ERR_BMSK                               0x8000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA_512_ERR_SHFT                                   15
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA_384_ERR_BMSK                               0x4000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA_384_ERR_SHFT                                   14
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA_2_ERR_BMSK                                 0x2000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA_2_ERR_SHFT                                     13
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA_1_ERR_BMSK                                 0x1000
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_HMAC_SHA_1_ERR_SHFT                                     12
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_GCM_AES_256_DEC_ERR_BMSK                             0x800
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_GCM_AES_256_DEC_ERR_SHFT                                11
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_GCM_AES_256_ENC_ERR_BMSK                             0x400
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_GCM_AES_256_ENC_ERR_SHFT                                10
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CCM_AES_256_DEC_ERR_BMSK                             0x200
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CCM_AES_256_DEC_ERR_SHFT                                 9
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CCM_AES_256_ENC_ERR_BMSK                             0x100
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CCM_AES_256_ENC_ERR_SHFT                                 8
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CMAC_AES_256_ERR_BMSK                                 0x80
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CMAC_AES_256_ERR_SHFT                                    7
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CMAC_AES_128_ERR_BMSK                                 0x40
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_CMAC_AES_128_ERR_SHFT                                    6
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_ECB_TDES_DEC_ERR_BMSK                                 0x20
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_ECB_TDES_DEC_ERR_SHFT                                    5
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_ECB_TDES_ENC_ERR_BMSK                                 0x10
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_ECB_TDES_ENC_ERR_SHFT                                    4
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_AES_256_DEC_ERR_BMSK                                   0x8
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_AES_256_DEC_ERR_SHFT                                     3
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_AES_256_ENC_ERR_BMSK                                   0x4
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_AES_256_ENC_ERR_SHFT                                     2
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_AES_128_DEC_ERR_BMSK                                   0x2
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_AES_128_DEC_ERR_SHFT                                     1
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_AES_128_ENC_ERR_BMSK                                   0x1
#define HWIO_CRYPTO0_CRYPTO_BIST_STATUS_BIST_AES_128_ENC_ERR_SHFT                                     0

#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_ADDR                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0x118)
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_RMSK                                                 0xfffcffff
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BIST_FINISH_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BIST_FINISH_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CBC_TDES_DEC_FIN_BMSK                           0x80000000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CBC_TDES_DEC_FIN_SHFT                                   31
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CBC_TDES_ENC_FIN_BMSK                           0x40000000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CBC_TDES_ENC_FIN_SHFT                                   30
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_XTS_AES_DEC_FIN_BMSK                            0x20000000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_XTS_AES_DEC_FIN_SHFT                                    29
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_XTS_AES_ENC_FIN_BMSK                            0x10000000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_XTS_AES_ENC_FIN_SHFT                                    28
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CTR_AES_DEC_FIN_BMSK                             0x8000000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CTR_AES_DEC_FIN_SHFT                                    27
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CTR_AES_ENC_FIN_BMSK                             0x4000000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CTR_AES_ENC_FIN_SHFT                                    26
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CBC_AES_DEC_FIN_BMSK                             0x2000000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CBC_AES_DEC_FIN_SHFT                                    25
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CBC_AES_ENC_FIN_BMSK                             0x1000000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CBC_AES_ENC_FIN_SHFT                                    24
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_SHAKE_256_FIN_BMSK                                0x800000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_SHAKE_256_FIN_SHFT                                      23
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_SHAKE_128_FIN_BMSK                                0x400000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_SHAKE_128_FIN_SHFT                                      22
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA3_384_FIN_BMSK                            0x200000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA3_384_FIN_SHFT                                  21
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA3_256_FIN_BMSK                            0x100000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA3_256_FIN_SHFT                                  20
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA3_224_FIN_BMSK                             0x80000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA3_224_FIN_SHFT                                  19
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA3_512_FIN_BMSK                             0x40000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA3_512_FIN_SHFT                                  18
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA_512_FIN_BMSK                               0x8000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA_512_FIN_SHFT                                   15
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA_384_FIN_BMSK                               0x4000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA_384_FIN_SHFT                                   14
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA_2_FIN_BMSK                                 0x2000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA_2_FIN_SHFT                                     13
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA_1_FIN_BMSK                                 0x1000
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_HMAC_SHA_1_FIN_SHFT                                     12
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_GCM_AES_256_DEC_FIN_BMSK                             0x800
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_GCM_AES_256_DEC_FIN_SHFT                                11
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_GCM_AES_256_ENC_FIN_BMSK                             0x400
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_GCM_AES_256_ENC_FIN_SHFT                                10
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CCM_AES_256_DEC_FIN_BMSK                             0x200
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CCM_AES_256_DEC_FIN_SHFT                                 9
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CCM_AES_256_ENC_FIN_BMSK                             0x100
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CCM_AES_256_ENC_FIN_SHFT                                 8
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CMAC_AES_256_FIN_BMSK                                 0x80
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CMAC_AES_256_FIN_SHFT                                    7
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CMAC_AES_128_FIN_BMSK                                 0x40
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_CMAC_AES_128_FIN_SHFT                                    6
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_ECB_TDES_DEC_FIN_BMSK                                 0x20
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_ECB_TDES_DEC_FIN_SHFT                                    5
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_ECB_TDES_ENC_FIN_BMSK                                 0x10
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_ECB_TDES_ENC_FIN_SHFT                                    4
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_AES_256_DEC_FIN_BMSK                                   0x8
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_AES_256_DEC_FIN_SHFT                                     3
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_AES_256_ENC_FIN_BMSK                                   0x4
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_AES_256_ENC_FIN_SHFT                                     2
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_AES_128_DEC_FIN_BMSK                                   0x2
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_AES_128_DEC_FIN_SHFT                                     1
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_AES_128_ENC_FIN_BMSK                                   0x1
#define HWIO_CRYPTO0_CRYPTO_BIST_FINISH_BIST_AES_128_ENC_FIN_SHFT                                     0

#define HWIO_CRYPTO0_CRYPTO_STATUS3_ADDR                                                     (CRYPTO0_CRYPTO_REG_BASE      + 0x11c)
#define HWIO_CRYPTO0_CRYPTO_STATUS3_RMSK                                                       0xff1111
#define HWIO_CRYPTO0_CRYPTO_STATUS3_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_STATUS3_ADDR)
#define HWIO_CRYPTO0_CRYPTO_STATUS3_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_STATUS3_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_STATUS3_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_STATUS3_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_STATUS3_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_STATUS3_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_STATUS3_IN)
#define HWIO_CRYPTO0_CRYPTO_STATUS3_KEY_INDEX_BMSK                                             0xff0000
#define HWIO_CRYPTO0_CRYPTO_STATUS3_KEY_INDEX_SHFT                                                   16
#define HWIO_CRYPTO0_CRYPTO_STATUS3_PIPE_KEY_PAUSE_ERR_BMSK                                      0x1000
#define HWIO_CRYPTO0_CRYPTO_STATUS3_PIPE_KEY_PAUSE_ERR_SHFT                                          12
#define HWIO_CRYPTO0_CRYPTO_STATUS3_PIPE_KEY_PAUSE_DET_MIRROR_BMSK                                0x100
#define HWIO_CRYPTO0_CRYPTO_STATUS3_PIPE_KEY_PAUSE_DET_MIRROR_SHFT                                    8
#define HWIO_CRYPTO0_CRYPTO_STATUS3_PIPE_KEY_TIMER_ERR_BMSK                                        0x10
#define HWIO_CRYPTO0_CRYPTO_STATUS3_PIPE_KEY_TIMER_ERR_SHFT                                           4
#define HWIO_CRYPTO0_CRYPTO_STATUS3_PIPE_KEY_TIMER_EXPIRED_BMSK                                     0x1
#define HWIO_CRYPTO0_CRYPTO_STATUS3_PIPE_KEY_TIMER_EXPIRED_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_GOPROC_ADDR                                                      (CRYPTO0_CRYPTO_REG_BASE      + 0x120)
#define HWIO_CRYPTO0_CRYPTO_GOPROC_RMSK                                                             0xf
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_GOPROC_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_GOPROC_RESULTS_DUMP_USE_OUT_SID_BMSK                                    0x8
#define HWIO_CRYPTO0_CRYPTO_GOPROC_RESULTS_DUMP_USE_OUT_SID_SHFT                                      3
#define HWIO_CRYPTO0_CRYPTO_GOPROC_RESULTS_DUMP_BMSK                                                0x4
#define HWIO_CRYPTO0_CRYPTO_GOPROC_RESULTS_DUMP_SHFT                                                  2
#define HWIO_CRYPTO0_CRYPTO_GOPROC_CLR_CNTXT_BMSK                                                   0x2
#define HWIO_CRYPTO0_CRYPTO_GOPROC_CLR_CNTXT_SHFT                                                     1
#define HWIO_CRYPTO0_CRYPTO_GOPROC_GO_BMSK                                                          0x1
#define HWIO_CRYPTO0_CRYPTO_GOPROC_GO_SHFT                                                            0

#define HWIO_CRYPTO0_CRYPTO_STATUS4_ADDR                                                     (CRYPTO0_CRYPTO_REG_BASE      + 0x124)
#define HWIO_CRYPTO0_CRYPTO_STATUS4_RMSK                                                         0xffff
#define HWIO_CRYPTO0_CRYPTO_STATUS4_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_STATUS4_ADDR)
#define HWIO_CRYPTO0_CRYPTO_STATUS4_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_STATUS4_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_STATUS4_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_STATUS4_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_STATUS4_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_STATUS4_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_STATUS4_IN)
#define HWIO_CRYPTO0_CRYPTO_STATUS4_COMMON_KEY_USAGE_ERR_BMSK                                    0x8000
#define HWIO_CRYPTO0_CRYPTO_STATUS4_COMMON_KEY_USAGE_ERR_SHFT                                        15
#define HWIO_CRYPTO0_CRYPTO_STATUS4_MODE_ERR_BMSK                                                0x4000
#define HWIO_CRYPTO0_CRYPTO_STATUS4_MODE_ERR_SHFT                                                    14
#define HWIO_CRYPTO0_CRYPTO_STATUS4_SYNC_CLR_INTR_BMSK                                           0x2000
#define HWIO_CRYPTO0_CRYPTO_STATUS4_SYNC_CLR_INTR_SHFT                                               13
#define HWIO_CRYPTO0_CRYPTO_STATUS4_KEY_INDEX_ERR_BMSK                                           0x1000
#define HWIO_CRYPTO0_CRYPTO_STATUS4_KEY_INDEX_ERR_SHFT                                               12
#define HWIO_CRYPTO0_CRYPTO_STATUS4_PIPE_KEY_USAGE_ERR2_VEC_BMSK                                  0xc00
#define HWIO_CRYPTO0_CRYPTO_STATUS4_PIPE_KEY_USAGE_ERR2_VEC_SHFT                                     10
#define HWIO_CRYPTO0_CRYPTO_STATUS4_SHK_USAGE_ERR_BMSK                                            0x200
#define HWIO_CRYPTO0_CRYPTO_STATUS4_SHK_USAGE_ERR_SHFT                                                9
#define HWIO_CRYPTO0_CRYPTO_STATUS4_PHK_USAGE_ERR_BMSK                                            0x100
#define HWIO_CRYPTO0_CRYPTO_STATUS4_PHK_USAGE_ERR_SHFT                                                8
#define HWIO_CRYPTO0_CRYPTO_STATUS4_PIPE_KEY_USAGE_ERR_VEC_BMSK                                    0xff
#define HWIO_CRYPTO0_CRYPTO_STATUS4_PIPE_KEY_USAGE_ERR_VEC_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_STATUS5_ADDR                                                     (CRYPTO0_CRYPTO_REG_BASE      + 0x128)
#define HWIO_CRYPTO0_CRYPTO_STATUS5_RMSK                                                     0x57f00000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_STATUS5_ADDR)
#define HWIO_CRYPTO0_CRYPTO_STATUS5_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_STATUS5_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_STATUS5_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_STATUS5_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_STATUS5_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_STATUS5_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_STATUS5_IN)
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KT_HW_PARAM_RD_BMSK                                      0x40000000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KT_HW_PARAM_RD_SHFT                                              30
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KT_CSR_PARAM_RD_BMSK                                     0x10000000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KT_CSR_PARAM_RD_SHFT                                             28
#define HWIO_CRYPTO0_CRYPTO_STATUS5_READ_FAILED_KEYPOLICY_MISMATCH_BMSK                       0x4000000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_READ_FAILED_KEYPOLICY_MISMATCH_SHFT                              26
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KEY_SLOT_EMPTY_BMSK                                       0x2000000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KEY_SLOT_EMPTY_SHFT                                              25
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KEYTABLE_INTEGRITY_ERROR_BMSK                             0x1000000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KEYTABLE_INTEGRITY_ERROR_SHFT                                    24
#define HWIO_CRYPTO0_CRYPTO_STATUS5_DOUBLE_KEY_SLOT_NOT_EVEN_BMSK                              0x800000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_DOUBLE_KEY_SLOT_NOT_EVEN_SHFT                                    23
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KEY_SLOT_ID_OUT_OF_RANGE_BMSK                              0x400000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KEY_SLOT_ID_OUT_OF_RANGE_SHFT                                    22
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KEY_SIZE_ERROR_BMSK                                        0x200000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_KEY_SIZE_ERROR_SHFT                                              21
#define HWIO_CRYPTO0_CRYPTO_STATUS5_USER_UNAUTHORIZED_BMSK                                     0x100000
#define HWIO_CRYPTO0_CRYPTO_STATUS5_USER_UNAUTHORIZED_SHFT                                           20

#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_MIRROR_ADDR                                 (CRYPTO0_CRYPTO_REG_BASE      + 0x130)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_MIRROR_RMSK                                        0x1
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_MIRROR_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_MIRROR_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_MIRROR_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_MIRROR_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_MIRROR_TIMER_ENABLE_BMSK                           0x1
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_MIRROR_TIMER_ENABLE_SHFT                             0

#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR_ADDR                                 (CRYPTO0_CRYPTO_REG_BASE      + 0x134)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR_GLOBAL_TIMER_VAL_LO_BMSK             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_MIRROR_GLOBAL_TIMER_VAL_LO_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR_ADDR                                 (CRYPTO0_CRYPTO_REG_BASE      + 0x138)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR_RMSK                                   0xffffff
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR_GLOBAL_TIMER_VAL_HI_BMSK               0xffffff
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_MIRROR_GLOBAL_TIMER_VAL_HI_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_STATUS6_ADDR                                                     (CRYPTO0_CRYPTO_REG_BASE      + 0x13c)
#define HWIO_CRYPTO0_CRYPTO_STATUS6_RMSK                                                         0xff03
#define HWIO_CRYPTO0_CRYPTO_STATUS6_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_STATUS6_ADDR)
#define HWIO_CRYPTO0_CRYPTO_STATUS6_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_STATUS6_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_STATUS6_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_STATUS6_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_STATUS6_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_STATUS6_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_STATUS6_IN)
#define HWIO_CRYPTO0_CRYPTO_STATUS6_KEY_INDEX_BMSK                                               0xff00
#define HWIO_CRYPTO0_CRYPTO_STATUS6_KEY_INDEX_SHFT                                                    8
#define HWIO_CRYPTO0_CRYPTO_STATUS6_AUTH_KEY_USAGE_ERR_BMSK                                         0x2
#define HWIO_CRYPTO0_CRYPTO_STATUS6_AUTH_KEY_USAGE_ERR_SHFT                                           1
#define HWIO_CRYPTO0_CRYPTO_STATUS6_ENCR_KEY_USAGE_ERR_BMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_STATUS6_ENCR_KEY_USAGE_ERR_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x140)
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_RMSK                                                0x1fffff
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_MAX_AXI_RD_BEATS_BMSK                               0x1ff000
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_MAX_AXI_RD_BEATS_SHFT                                     12
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_NUM_SHA3_ROUNDS_BMSK                                   0xc00
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_NUM_SHA3_ROUNDS_SHFT                                      10
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_NUM_SHA_ROUNDS_BMSK                                    0x300
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_NUM_SHA_ROUNDS_SHFT                                        8
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_NUM_AES_ROUNDS_BMSK                                     0xc0
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_NUM_AES_ROUNDS_SHFT                                        6
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_AUTH_SHA3_SEL_BMSK                                      0x20
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_AUTH_SHA3_SEL_SHFT                                         5
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_CRYPTO_WRAPPER_CONFIGURATION_BMSK                       0x1c
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_CRYPTO_WRAPPER_CONFIGURATION_SHFT                          2
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_ENCR_SM4_SEL_BMSK                                        0x2
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_ENCR_SM4_SEL_SHFT                                          1
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_AUTH_SM3_SEL_BMSK                                        0x1
#define HWIO_CRYPTO0_CRYPTO_ENGINES_AVAIL2_AUTH_SM3_SEL_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_COMMON_KEY_SEL_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x144)
#define HWIO_CRYPTO0_CRYPTO_COMMON_KEY_SEL_RMSK                                                    0xff
#define HWIO_CRYPTO0_CRYPTO_COMMON_KEY_SEL_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_COMMON_KEY_SEL_ADDR)
#define HWIO_CRYPTO0_CRYPTO_COMMON_KEY_SEL_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_COMMON_KEY_SEL_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_COMMON_KEY_SEL_COMMON_POOL_KEY_SEL_BMSK                                0xff
#define HWIO_CRYPTO0_CRYPTO_COMMON_KEY_SEL_COMMON_POOL_KEY_SEL_SHFT                                   0

#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x180)
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_RMSK                                                   0x7fff
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_IN)
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_FAULT_MASK_BMSK                                        0x4000
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_FAULT_MASK_SHFT                                            14
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_VPCNT_BMSK                                             0x3f00
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_VPCNT_SHFT                                                  8
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_HPCNT_BMSK                                               0xfc
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_HPCNT_SHFT                                                  2
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_PARITY_DISABLE_BMSK                                       0x2
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_PARITY_DISABLE_SHFT                                         1
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_FAULT_BMSK                                                0x1
#define HWIO_CRYPTO0_CRYPTO_PARITY_STATUS_FAULT_SHFT                                                  0

#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_ADDR                                                (CRYPTO0_CRYPTO_REG_BASE      + 0x184)
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_RMSK                                                     0x333
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_IN)
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_FAULT_UNMASKED_BMSK                                      0x300
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_FAULT_UNMASKED_SHFT                                          8
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_FAULT_MASK_BMSK                                           0x20
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_FAULT_MASK_SHFT                                              5
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_LS_COMP_DISABLE_BMSK                                      0x10
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_LS_COMP_DISABLE_SHFT                                         4
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_FAULT_BMSK                                                 0x3
#define HWIO_CRYPTO0_CRYPTO_KP_LS_STATUS_FAULT_SHFT                                                   0

#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x188)
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_RMSK                                                   0x333
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_IN)
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_FAULT_UNMASKED_BMSK                                    0x300
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_FAULT_UNMASKED_SHFT                                        8
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_FAULT_MASK_BMSK                                         0x20
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_FAULT_MASK_SHFT                                            5
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_LS_COMP_DISABLE_BMSK                                    0x10
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_LS_COMP_DISABLE_SHFT                                       4
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_FAULT_BMSK                                               0x3
#define HWIO_CRYPTO0_CRYPTO_CLIB_LS_STATUS_FAULT_SHFT                                                 0

#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x1c0)
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_RMSK                                                0x1ffffff
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_ADDR)
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_KMAC_128_256_BMSK                                   0x1000000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_KMAC_128_256_SHFT                                          24
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_CSHAKE_128_256_BMSK                                  0x800000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_CSHAKE_128_256_SHFT                                        23
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHAKE_128_256_BMSK                                   0x400000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHAKE_128_256_SHFT                                         22
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA3_512_BMSK                                   0x200000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA3_512_SHFT                                         21
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA3_384_BMSK                                   0x100000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA3_384_SHFT                                         20
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA3_256_BMSK                                    0x80000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA3_256_SHFT                                         19
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA3_224_BMSK                                    0x40000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA3_224_SHFT                                         18
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA3_512_BMSK                                         0x20000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA3_512_SHFT                                              17
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA3_384_BMSK                                         0x10000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA3_384_SHFT                                              16
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA3_256_BMSK                                          0x8000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA3_256_SHFT                                              15
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA3_224_BMSK                                          0x4000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA3_224_SHFT                                              14
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_AES_128_256_CMAC_BMSK                                  0x2000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_AES_128_256_CMAC_SHFT                                      13
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA512_BMSK                                       0x1000
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA512_SHFT                                           12
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA384_BMSK                                        0x800
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA384_SHFT                                           11
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA256_BMSK                                        0x400
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA256_SHFT                                           10
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA1_BMSK                                          0x200
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_HMAC_SHA1_SHFT                                              9
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA512_BMSK                                             0x100
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA512_SHFT                                                 8
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA384_BMSK                                              0x80
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA384_SHFT                                                 7
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA256_BMSK                                              0x40
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA256_SHFT                                                 6
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA1_BMSK                                                0x20
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_SHA1_SHFT                                                   5
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_TDES_2KEY_ECB_CBC_BMSK                                   0x10
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_TDES_2KEY_ECB_CBC_SHFT                                      4
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_TDES_ECB_CBC_BMSK                                         0x8
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_TDES_ECB_CBC_SHFT                                           3
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_AES_128_256_XTS_BMSK                                      0x4
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_AES_128_256_XTS_SHFT                                        2
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_AES_128_256_CCM_BMSK                                      0x2
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_AES_128_256_CCM_SHFT                                        1
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_AES_128_256_ECB_CBC_CTR_BMSK                              0x1
#define HWIO_CRYPTO0_CRYPTO_FIPS_APPROVED_AES_128_256_ECB_CBC_CTR_SHFT                                0

#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_ADDR                                           (CRYPTO0_CRYPTO_REG_BASE      + 0x1e0)
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_RMSK                                                0x1ff
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_ADDR)
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_KMAC_128_256_LT_112_BMSK                            0x100
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_KMAC_128_256_LT_112_SHFT                                8
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_HMAC_SHA3_224_256_384_512_LT_112_BMSK                0x80
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_HMAC_SHA3_224_256_384_512_LT_112_SHFT                   7
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_SM4_ECB_CBC_CTR_BMSK                                 0x40
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_SM4_ECB_CBC_CTR_SHFT                                    6
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_SM3_HMAC_SM3_BMSK                                    0x20
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_SM3_HMAC_SM3_SHFT                                       5
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_AES_SIV_BMSK                                         0x10
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_AES_SIV_SHFT                                            4
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_AES_128_256_GCM_BMSK                                  0x8
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_AES_128_256_GCM_SHFT                                    3
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_AEAD_SHA1_BMSK                                        0x4
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_AEAD_SHA1_SHFT                                          2
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_HMAC_SHA_256_384_512_LT_112_BMSK                      0x2
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_HMAC_SHA_256_384_512_LT_112_SHFT                        1
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_DES_CBC_ECB_BMSK                                      0x1
#define HWIO_CRYPTO0_CRYPTO_FIPS_NON_APPROVED_DES_CBC_ECB_SHFT                                        0

#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_ADDR                                                (CRYPTO0_CRYPTO_REG_BASE      + 0x1f0)
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_RMSK                                                   0x70001
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_IN)
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_KM_STATUS_IRQ_BMSK                                     0x40000
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_KM_STATUS_IRQ_SHFT                                          18
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_KM_ERR_VEC_IRQ_BMSK                                    0x20000
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_KM_ERR_VEC_IRQ_SHFT                                         17
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_CORE_IRQ_BMSK                                          0x10000
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_CORE_IRQ_SHFT                                               16
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_SEC_IRQ_MASK_BMSK                                          0x1
#define HWIO_CRYPTO0_CRYPTO_SEC_IRQ_STTS_SEC_IRQ_MASK_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x1f4)
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_RMSK                                                  0x70001
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_IN)
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_KM_STATUS_IRQ_BMSK                                    0x40000
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_KM_STATUS_IRQ_SHFT                                         18
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_KM_ERR_VEC_IRQ_BMSK                                   0x20000
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_KM_ERR_VEC_IRQ_SHFT                                        17
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_CORE_IRQ_BMSK                                         0x10000
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_CORE_IRQ_SHFT                                              16
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_NSEC_IRQ_MASK_BMSK                                        0x1
#define HWIO_CRYPTO0_CRYPTO_NSEC_IRQ_STTS_NSEC_IRQ_MASK_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ADDR                                                (CRYPTO0_CRYPTO_REG_BASE      + 0x200)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_RMSK                                                  0x1fffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_USE_COMMON_KEY_ENCR_BMSK                              0x100000
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_USE_COMMON_KEY_ENCR_SHFT                                    20
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_RESERVED_19_BMSK                                       0x80000
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_RESERVED_19_SHFT                                            19
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ODD_KEY_SEL_BMSK                                       0x40000
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ODD_KEY_SEL_SHFT                                            18
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_KEYSTREAM_ENABLE_BMSK                                  0x20000
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_KEYSTREAM_ENABLE_SHFT                                       17
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_F8_DIRECTION_BMSK                                      0x10000
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_F8_DIRECTION_SHFT                                           16
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_USE_PIPE_KEY_ENCR_BMSK                                  0x8000
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_USE_PIPE_KEY_ENCR_SHFT                                      15
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_USE_HW_KEY_ENCR_BMSK                                    0x4000
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_USE_HW_KEY_ENCR_SHFT                                        14
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_LAST_BMSK                                               0x2000
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_LAST_SHFT                                                   13
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_CNTR_ALG_BMSK                                           0x1800
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_CNTR_ALG_SHFT                                               11
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ENCODE_BMSK                                              0x400
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ENCODE_SHFT                                                 10
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ENCR_MODE_BMSK                                           0x3c0
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ENCR_MODE_SHFT                                               6
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ENCR_KEY_SZ_BMSK                                          0x38
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ENCR_KEY_SZ_SHFT                                             3
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ENCR_ALG_BMSK                                              0x7
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_CFG_ENCR_ALG_SHFT                                                0

#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x204)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_ENCR_SIZE_BMSK                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_SIZE_ENCR_SIZE_SHFT                                              0

#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x208)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_ENCR_START_BMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_SEG_START_ENCR_START_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x20c)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_CRYPTO_CNTR0_IV0_BMSK                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR0_IV0_CRYPTO_CNTR0_IV0_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x210)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_CRYPTO_CNTR1_IV1_BMSK                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR1_IV1_CRYPTO_CNTR1_IV1_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x214)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_CRYPTO_CNTR2_IV2_BMSK                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR2_IV2_CRYPTO_CNTR2_IV2_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x218)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_CRYPTO_CNTR3_IV3_BMSK                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR3_IV3_CRYPTO_CNTR3_IV3_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x21c)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_CNTR_MASK_BMSK                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK_CNTR_MASK_SHFT                                             0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_ADDR(n)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X220 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_RMSK                                         0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_MAXn                                                  3
#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_CCM_INIT_CNTR_BMSK                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CCM_INIT_CNTRn_CCM_INIT_CNTR_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_ADDR                                            (CRYPTO0_CRYPTO_REG_BASE      + 0x230)
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_RMSK                                               0xfffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_DU_SIZE_BMSK                                       0xfffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_DU_SIZE_DU_SIZE_SHFT                                             0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_ADDR                                             (CRYPTO0_CRYPTO_REG_BASE      + 0x234)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_RMSK                                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_CNTR_MASK_BMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK2_CNTR_MASK_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_ADDR                                             (CRYPTO0_CRYPTO_REG_BASE      + 0x238)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_RMSK                                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_CNTR_MASK_BMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK1_CNTR_MASK_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_ADDR                                             (CRYPTO0_CRYPTO_REG_BASE      + 0x23c)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_RMSK                                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_IN)
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_CNTR_MASK_BMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_CNTR_MASK0_CNTR_MASK_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x240)
#define HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_RMSK                                                     0xff
#define HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_ADDR)
#define HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_IN)
#define HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_GCM_IV_LENGTH_BMSK                                       0xff
#define HWIO_CRYPTO0_CRYPTO_GCM_IV_LENGTH_GCM_IV_LENGTH_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_ADDR                                                (CRYPTO0_CRYPTO_REG_BASE      + 0x300)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_RMSK                                                0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_IN)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_USE_COMMON_KEY_AUTH_BMSK                            0x40000000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_USE_COMMON_KEY_AUTH_SHFT                                    30
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_PARTIAL_BIT_SIZE_BMSK                          0x3c000000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_PARTIAL_BIT_SIZE_SHFT                                  26
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_RESERVED_25_BMSK                                     0x2000000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_RESERVED_25_SHFT                                            25
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_COMP_EXP_MAC_BMSK                                    0x1000000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_COMP_EXP_MAC_SHFT                                           24
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_F9_DIRECTION_BMSK                                     0x800000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_F9_DIRECTION_SHFT                                           23
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_NONCE_NUM_WORDS_BMSK                             0x700000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_NONCE_NUM_WORDS_SHFT                                   20
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_USE_PIPE_KEY_AUTH_BMSK                                 0x80000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_USE_PIPE_KEY_AUTH_SHFT                                      19
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_USE_HW_KEY_AUTH_BMSK                                   0x40000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_USE_HW_KEY_AUTH_SHFT                                        18
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_FIRST_BMSK                                             0x20000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_FIRST_SHFT                                                  17
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_LAST_BMSK                                              0x10000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_LAST_SHFT                                                   16
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_POS_BMSK                                           0xc000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_POS_SHFT                                               14
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_SIZE_BMSK                                          0x3e00
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_SIZE_SHFT                                               9
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_MODE_BMSK                                           0x1c0
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_MODE_SHFT                                               6
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_KEY_SZ_BMSK                                          0x38
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_KEY_SZ_SHFT                                             3
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_ALG_BMSK                                              0x7
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_CFG_AUTH_ALG_SHFT                                                0

#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x304)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_IN)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_AUTH_SIZE_BMSK                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_SIZE_AUTH_SIZE_SHFT                                              0

#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x308)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_IN)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_AUTH_START_BMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_SEG_START_AUTH_START_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_ADDR                                      (CRYPTO0_CRYPTO_REG_BASE      + 0x30c)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_RMSK                                      0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_IN)
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_RESTORE_BMSK                              0x80000000
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_RESTORE_SHFT                                      31
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_CNTR_BMSK                                 0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_SHA3_SQUEEZE_CNTR_CNTR_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_ADDR(n)                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0X310 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_RMSK                                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_MAXn                                                            15
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_IVn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_IVn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_IVn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_IVn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_AUTH_IVn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_AUTH_IVN_BMSK                                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVn_AUTH_IVN_SHFT                                                    0

#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_ADDR(n)                                         (CRYPTO0_CRYPTO_REG_BASE      + 0X350 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_RMSK                                            0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_MAXn                                                     3
#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_CTRL_INFO_NONCE_BMSK                            0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_INFO_NONCEn_CTRL_INFO_NONCE_SHFT                                     0

#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x390)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_IN)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_AUTH_BYTECNT0_BMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT0_AUTH_BYTECNT0_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x394)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_IN)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_AUTH_BYTECNT1_BMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT1_AUTH_BYTECNT1_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x398)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_IN)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_AUTH_BYTECNT2_BMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT2_AUTH_BYTECNT2_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x39c)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_IN)
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_AUTH_BYTECNT3_BMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_BYTECNT3_AUTH_BYTECNT3_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_ADDR(n)                                            (CRYPTO0_CRYPTO_REG_BASE      + 0X3A0 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_MAXn                                                       15
#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_EXP_MAC_BMSK                                       0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_EXP_MACn_EXP_MAC_SHFT                                                0

#define HWIO_CRYPTO0_CRYPTO_CONFIG_ADDR                                                      (CRYPTO0_CRYPTO_REG_BASE      + 0x400)
#define HWIO_CRYPTO0_CRYPTO_CONFIG_RMSK                                                        0x1fffff
#define HWIO_CRYPTO0_CRYPTO_CONFIG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_CONFIG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_CONFIG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_CONFIG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_CONFIG_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_CONFIG_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_CONFIG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_CONFIG_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_CONFIG_IN)
#define HWIO_CRYPTO0_CRYPTO_CONFIG_REQ_SIZE_BMSK                                               0x1e0000
#define HWIO_CRYPTO0_CRYPTO_CONFIG_REQ_SIZE_SHFT                                                     17
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MAX_QUEUED_REQS_BMSK                                         0x1c000
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MAX_QUEUED_REQS_SHFT                                              14
#define HWIO_CRYPTO0_CRYPTO_CONFIG_IRQ_ENABLE_BMSK                                               0x3c00
#define HWIO_CRYPTO0_CRYPTO_CONFIG_IRQ_ENABLE_SHFT                                                   10
#define HWIO_CRYPTO0_CRYPTO_CONFIG_LITTLE_ENDIAN_MODE_BMSK                                        0x200
#define HWIO_CRYPTO0_CRYPTO_CONFIG_LITTLE_ENDIAN_MODE_SHFT                                            9
#define HWIO_CRYPTO0_CRYPTO_CONFIG_PIPE_SET_SELECT_BMSK                                           0x1e0
#define HWIO_CRYPTO0_CRYPTO_CONFIG_PIPE_SET_SELECT_SHFT                                               5
#define HWIO_CRYPTO0_CRYPTO_CONFIG_HIGH_SPD_DATA_EN_N_BMSK                                         0x10
#define HWIO_CRYPTO0_CRYPTO_CONFIG_HIGH_SPD_DATA_EN_N_SHFT                                            4
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MASK_DOUT_INTR_BMSK                                              0x8
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MASK_DOUT_INTR_SHFT                                                3
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MASK_DIN_INTR_BMSK                                               0x4
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MASK_DIN_INTR_SHFT                                                 2
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MASK_OP_DONE_INTR_BMSK                                           0x2
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MASK_OP_DONE_INTR_SHFT                                             1
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MASK_ERR_INTR_BMSK                                               0x1
#define HWIO_CRYPTO0_CRYPTO_CONFIG_MASK_ERR_INTR_SHFT                                                 0

#define HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_ADDR                                                (CRYPTO0_CRYPTO_REG_BASE      + 0x404)
#define HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_RMSK                                                      0xff
#define HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_ADDR)
#define HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_IN)
#define HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_KEY_INDEX_BMSK                                            0xff
#define HWIO_CRYPTO0_CRYPTO_KM_KEY_INDEX_KEY_INDEX_SHFT                                               0

#define HWIO_CRYPTO0_CRYPTO_PWR_CTRL_ADDR                                                    (CRYPTO0_CRYPTO_REG_BASE      + 0x408)
#define HWIO_CRYPTO0_CRYPTO_PWR_CTRL_RMSK                                                           0x1
#define HWIO_CRYPTO0_CRYPTO_PWR_CTRL_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PWR_CTRL_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PWR_CTRL_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PWR_CTRL_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PWR_CTRL_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_PWR_CTRL_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_PWR_CTRL_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_PWR_CTRL_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_PWR_CTRL_IN)
#define HWIO_CRYPTO0_CRYPTO_PWR_CTRL_AUTO_SHUTDOWN_EN_BMSK                                          0x1
#define HWIO_CRYPTO0_CRYPTO_PWR_CTRL_AUTO_SHUTDOWN_EN_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_ADDR                                            (CRYPTO0_CRYPTO_REG_BASE      + 0x410)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_RMSK                                                   0x3
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_ADDR)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_IN)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_OVERRIDE_VALUE_BMSK                                    0x2
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_OVERRIDE_VALUE_SHFT                                      1
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_OVERRIDE_ENABLE_BMSK                                   0x1
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_OVERRIDE_OVERRIDE_ENABLE_SHFT                                     0

#define HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_ADDR                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0x414)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_RMSK                                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_IN)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_CYCLE_COUNT_BMSK                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_CFG_CYCLE_COUNT_SHFT                                              0

#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_ADDR(n)                                          (CRYPTO0_CRYPTO_REG_BASE      + 0X420 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_RMSK                                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_MAXn                                                      3
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_COUNT_BMSK                                       0xffffff00
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_COUNT_SHFT                                                8
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_THRESHOLD_BMSK                                         0xff
#define HWIO_CRYPTO0_CRYPTO_QACTIVE_STATUSn_THRESHOLD_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_ADDR(m)                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0X400 + (0x4*(m)))
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_RMSK                                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_MAXm                                                            49
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_IVm_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_IVm_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_IVm_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_AUTH_IVm_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_AUTH_IVm_INI(m))
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_AUTH_IVN_BMSK                                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_IVm_AUTH_IVN_SHFT                                                    0

#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_ADDR                                          (CRYPTO0_CRYPTO_REG_BASE      + 0x500)
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_RMSK                                              0xffff
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_IN)
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_PATT_OFFSET_BMSK                                  0xf000
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_PATT_OFFSET_SHFT                                      12
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_PROC_DATA_SZ_BMSK                                  0xf00
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_PROC_DATA_SZ_SHFT                                      8
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_PATT_SZ_BMSK                                        0xf0
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_PATT_SZ_SHFT                                           4
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_RESERVED_3_1_BMSK                                    0xe
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_RESERVED_3_1_SHFT                                      1
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_PATT_EN_BMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_DATA_PATT_PROC_CFG_PATT_EN_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_ADDR                                 (CRYPTO0_CRYPTO_REG_BASE      + 0x504)
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_RMSK                                       0xff
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_IN)
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_PARTIAL_BLOCK_OFFSET_BMSK                  0xf0
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_PARTIAL_BLOCK_OFFSET_SHFT                     4
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_RESERVED_3_1_BMSK                           0xe
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_RESERVED_3_1_SHFT                             1
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_PARTIAL_EN_BMSK                             0x1
#define HWIO_CRYPTO0_CRYPTO_DATA_PARTIAL_BLOCK_PROC_CFG_PARTIAL_EN_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_ADDR(m)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0X604 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_RMSK                                               0x1
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_MAXm                                                 7
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_TIMER_ENABLE_BMSK                                  0x1
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MIRROR_TIMER_ENABLE_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_ADDR(m)                               (CRYPTO0_CRYPTO_REG_BASE      + 0X608 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_RMSK                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_MAXm                                           7
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_TIMER_LIMIT_VAL_LO_BMSK               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MIRROR_TIMER_LIMIT_VAL_LO_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_ADDR(m)                               (CRYPTO0_CRYPTO_REG_BASE      + 0X60C + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_RMSK                                    0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_MAXm                                           7
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_TIMER_LIMIT_VAL_HI_BMSK                 0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MIRROR_TIMER_LIMIT_VAL_HI_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_ADDR(m)                                    (CRYPTO0_CRYPTO_REG_BASE      + 0X610 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_RMSK                                          0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_MAXm                                                7
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_PROT_LVL_CHK_SEL_BMSK                         0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MIRROR_PROT_LVL_CHK_SEL_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_ADDR(m)                               (CRYPTO0_CRYPTO_REG_BASE      + 0X618 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_RMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_MAXm                                           7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_HDCP_ENCR_EN_BMSK                            0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MIRROR_HDCP_ENCR_EN_SHFT                              0

#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_ADDR(j)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0X604 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_RMSK                                               0x1
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_MAXj                                                15
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_TIMER_ENABLE_BMSK                                  0x1
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MIRROR_TIMER_ENABLE_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_ADDR(j)                               (CRYPTO0_CRYPTO_REG_BASE      + 0X608 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_RMSK                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_MAXj                                          15
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_TIMER_LIMIT_VAL_LO_BMSK               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MIRROR_TIMER_LIMIT_VAL_LO_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_ADDR(j)                               (CRYPTO0_CRYPTO_REG_BASE      + 0X60C + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_RMSK                                    0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_MAXj                                          15
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_TIMER_LIMIT_VAL_HI_BMSK                 0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MIRROR_TIMER_LIMIT_VAL_HI_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_ADDR(j)                                    (CRYPTO0_CRYPTO_REG_BASE      + 0X610 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_RMSK                                          0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_MAXj                                               15
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_PROT_LVL_CHK_SEL_BMSK                         0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MIRROR_PROT_LVL_CHK_SEL_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_ADDR(j)                               (CRYPTO0_CRYPTO_REG_BASE      + 0X618 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_RMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_MAXj                                          15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_HDCP_ENCR_EN_BMSK                            0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MIRROR_HDCP_ENCR_EN_SHFT                              0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_ADDR(m,n)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X800 + (0x10*(m)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_RMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_MAXm                                            7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_MAXn                                            3
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_INI2(m,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_ADDR(m,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_INMI2(m,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_ADDR(m,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_CNTR_IV_BMSK                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MIRROR_CNTR_IV_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_ADDR(j,n)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X800 + (0x10*(j)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_RMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_MAXj                                           15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_MAXn                                            3
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_INI2(j,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_ADDR(j,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_INMI2(j,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_ADDR(j,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_CNTR_IV_BMSK                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MIRROR_CNTR_IV_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_ADDR(m,n)                            (CRYPTO0_CRYPTO_REG_BASE      + 0X900 + (0x10*(m)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_MAXm                                          7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_MAXn                                          2
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_INI2(m,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_ADDR(m,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_INMI2(m,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_ADDR(m,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_CNTR_MASK_BMSK                       0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MIRROR_CNTR_MASK_SHFT                                0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_ADDR(m)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X90C + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_MAXm                                          7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_CNTR_MASK_BMSK                       0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MIRROR_CNTR_MASK_SHFT                                0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_ADDR(j,n)                            (CRYPTO0_CRYPTO_REG_BASE      + 0X900 + (0x10*(j)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_MAXj                                         15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_MAXn                                          2
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_INI2(j,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_ADDR(j,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_INMI2(j,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_ADDR(j,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_CNTR_MASK_BMSK                       0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MIRROR_CNTR_MASK_SHFT                                0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_ADDR(j)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X90C + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_MAXj                                         15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_CNTR_MASK_BMSK                       0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MIRROR_CNTR_MASK_SHFT                                0

#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_KEY_BASE_SLOT_ID_ADDR                                  (CRYPTO0_CRYPTO_REG_BASE      + 0xb00)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_KEY_BASE_SLOT_ID_RMSK                                        0xff
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_KEY_BASE_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_KEY_BASE_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_KEY_BASE_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_KEY_BASE_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_KEY_BASE_SLOT_ID_BASE_SLOT_ID_BMSK                           0xff
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_KEY_BASE_SLOT_ID_BASE_SLOT_ID_SHFT                              0

#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_KEY_BASE_SLOT_ID_ADDR                              (CRYPTO0_CRYPTO_REG_BASE      + 0xb04)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_KEY_BASE_SLOT_ID_RMSK                                    0xff
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_KEY_BASE_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_KEY_BASE_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_KEY_BASE_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_KEY_BASE_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_KEY_BASE_SLOT_ID_BASE_SLOT_ID_BMSK                       0xff
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_KEY_BASE_SLOT_ID_BASE_SLOT_ID_SHFT                          0

#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_ODD_KEY_BASE_SLOT_ID_ADDR                              (CRYPTO0_CRYPTO_REG_BASE      + 0xb08)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_ODD_KEY_BASE_SLOT_ID_RMSK                                    0xff
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_ODD_KEY_BASE_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_ODD_KEY_BASE_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_ODD_KEY_BASE_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_ODD_KEY_BASE_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_ODD_KEY_BASE_SLOT_ID_BASE_SLOT_ID_BMSK                       0xff
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_ODD_KEY_BASE_SLOT_ID_BASE_SLOT_ID_SHFT                          0

#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_ODD_KEY_BASE_SLOT_ID_ADDR                          (CRYPTO0_CRYPTO_REG_BASE      + 0xb0c)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_ODD_KEY_BASE_SLOT_ID_RMSK                                0xff
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_ODD_KEY_BASE_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_ODD_KEY_BASE_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_ODD_KEY_BASE_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_ODD_KEY_BASE_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_ODD_KEY_BASE_SLOT_ID_BASE_SLOT_ID_BMSK                   0xff
#define HWIO_CRYPTO0_CRYPTO_ENCR_PIPE_XTS_ODD_KEY_BASE_SLOT_ID_BASE_SLOT_ID_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_AUTH_PIPE_KEY_BASE_SLOT_ID_ADDR                                  (CRYPTO0_CRYPTO_REG_BASE      + 0xb10)
#define HWIO_CRYPTO0_CRYPTO_AUTH_PIPE_KEY_BASE_SLOT_ID_RMSK                                        0xff
#define HWIO_CRYPTO0_CRYPTO_AUTH_PIPE_KEY_BASE_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_AUTH_PIPE_KEY_BASE_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_AUTH_PIPE_KEY_BASE_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_AUTH_PIPE_KEY_BASE_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_AUTH_PIPE_KEY_BASE_SLOT_ID_BASE_SLOT_ID_BMSK                           0xff
#define HWIO_CRYPTO0_CRYPTO_AUTH_PIPE_KEY_BASE_SLOT_ID_BASE_SLOT_ID_SHFT                              0

#define HWIO_CRYPTO0_CRYPTO_PHK_SLOT_ID_ADDR                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0xb14)
#define HWIO_CRYPTO0_CRYPTO_PHK_SLOT_ID_RMSK                                                       0xff
#define HWIO_CRYPTO0_CRYPTO_PHK_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PHK_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PHK_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PHK_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PHK_SLOT_ID_SLOT_ID_BMSK                                               0xff
#define HWIO_CRYPTO0_CRYPTO_PHK_SLOT_ID_SLOT_ID_SHFT                                                  0

#define HWIO_CRYPTO0_CRYPTO_SHK_SLOT_ID_ADDR                                                 (CRYPTO0_CRYPTO_REG_BASE      + 0xb18)
#define HWIO_CRYPTO0_CRYPTO_SHK_SLOT_ID_RMSK                                                       0xff
#define HWIO_CRYPTO0_CRYPTO_SHK_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_SHK_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_SHK_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_SHK_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_SHK_SLOT_ID_SLOT_ID_BMSK                                               0xff
#define HWIO_CRYPTO0_CRYPTO_SHK_SLOT_ID_SLOT_ID_SHFT                                                  0

#define HWIO_CRYPTO0_CRYPTO_DRM_KEY_BASE_SLOT_ID_ADDR                                        (CRYPTO0_CRYPTO_REG_BASE      + 0xb1c)
#define HWIO_CRYPTO0_CRYPTO_DRM_KEY_BASE_SLOT_ID_RMSK                                              0xff
#define HWIO_CRYPTO0_CRYPTO_DRM_KEY_BASE_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_DRM_KEY_BASE_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_DRM_KEY_BASE_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_DRM_KEY_BASE_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_DRM_KEY_BASE_SLOT_ID_BASE_SLOT_ID_BMSK                                 0xff
#define HWIO_CRYPTO0_CRYPTO_DRM_KEY_BASE_SLOT_ID_BASE_SLOT_ID_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_GP_KEY_BASE_SLOT_ID_ADDR                                         (CRYPTO0_CRYPTO_REG_BASE      + 0xb20)
#define HWIO_CRYPTO0_CRYPTO_GP_KEY_BASE_SLOT_ID_RMSK                                               0xff
#define HWIO_CRYPTO0_CRYPTO_GP_KEY_BASE_SLOT_ID_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_GP_KEY_BASE_SLOT_ID_ADDR)
#define HWIO_CRYPTO0_CRYPTO_GP_KEY_BASE_SLOT_ID_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_GP_KEY_BASE_SLOT_ID_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_GP_KEY_BASE_SLOT_ID_BASE_SLOT_ID_BMSK                                  0xff
#define HWIO_CRYPTO0_CRYPTO_GP_KEY_BASE_SLOT_ID_BASE_SLOT_ID_SHFT                                     0

#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_ADDR(i)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0XA04 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_RMSK                                               0x1
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_MAXi                                                23
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_TIMER_ENABLE_BMSK                                  0x1
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MIRROR_TIMER_ENABLE_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_ADDR(i)                               (CRYPTO0_CRYPTO_REG_BASE      + 0XA08 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_RMSK                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_MAXi                                          23
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_TIMER_LIMIT_VAL_LO_BMSK               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MIRROR_TIMER_LIMIT_VAL_LO_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_ADDR(i)                               (CRYPTO0_CRYPTO_REG_BASE      + 0XA0C + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_RMSK                                    0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_MAXi                                          23
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_TIMER_LIMIT_VAL_HI_BMSK                 0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MIRROR_TIMER_LIMIT_VAL_HI_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_ADDR(i)                                    (CRYPTO0_CRYPTO_REG_BASE      + 0XA10 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_RMSK                                          0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_MAXi                                               23
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_PROT_LVL_CHK_SEL_BMSK                         0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MIRROR_PROT_LVL_CHK_SEL_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_ADDR(i)                               (CRYPTO0_CRYPTO_REG_BASE      + 0XA18 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_RMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_MAXi                                          23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_HDCP_ENCR_EN_BMSK                            0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MIRROR_HDCP_ENCR_EN_SHFT                              0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_ADDR(i,n)                              (CRYPTO0_CRYPTO_REG_BASE      + 0XD00 + (0x10*(i)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_RMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_MAXi                                           23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_MAXn                                            3
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_INI2(i,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_ADDR(i,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_INMI2(i,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_ADDR(i,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_CNTR_IV_BMSK                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MIRROR_CNTR_IV_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_ADDR(i,n)                            (CRYPTO0_CRYPTO_REG_BASE      + 0XE00 + (0x10*(i)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_MAXi                                         23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_MAXn                                          2
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_INI2(i,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_ADDR(i,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_INMI2(i,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_ADDR(i,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_CNTR_MASK_BMSK                       0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MIRROR_CNTR_MASK_SHFT                                0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_ADDR(i)                              (CRYPTO0_CRYPTO_REG_BASE      + 0XE0C + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_MAXi                                         23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_CNTR_MASK_BMSK                       0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MIRROR_CNTR_MASK_SHFT                                0

#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_ADDR                                               (CRYPTO0_CRYPTO_REG_BASE      + 0x1000)
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_RMSK                                                      0xf
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_RESULTS_DUMP_USE_OUT_SID_BMSK                             0x8
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_RESULTS_DUMP_USE_OUT_SID_SHFT                               3
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_RESULTS_DUMP_BMSK                                         0x4
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_RESULTS_DUMP_SHFT                                           2
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_CLR_CNTXT_BMSK                                            0x2
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_CLR_CNTXT_SHFT                                              1
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_GO_BMSK                                                   0x1
#define HWIO_CRYPTO0_CRYPTO_GOPROC_QC_KEY_GO_SHFT                                                     0

#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_ADDR                                              (CRYPTO0_CRYPTO_REG_BASE      + 0x2000)
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_RMSK                                                     0xf
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_RESULTS_DUMP_USE_OUT_SID_BMSK                            0x8
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_RESULTS_DUMP_USE_OUT_SID_SHFT                              3
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_RESULTS_DUMP_BMSK                                        0x4
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_RESULTS_DUMP_SHFT                                          2
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_CLR_CNTXT_BMSK                                           0x2
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_CLR_CNTXT_SHFT                                             1
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_GO_BMSK                                                  0x1
#define HWIO_CRYPTO0_CRYPTO_GOPROC_OEM_KEY_GO_SHFT                                                    0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYn_ADDR(n)                                                (CRYPTO0_CRYPTO_REG_BASE      + 0X3000 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYn_RMSK                                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYn_MAXn                                                            7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYn_CRYPTO_ENCR_KEY_BMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYn_CRYPTO_ENCR_KEY_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_KEYn_ADDR(n)                                            (CRYPTO0_CRYPTO_REG_BASE      + 0X3020 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_KEYn_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_KEYn_MAXn                                                        7
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_KEYn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_XTS_KEYn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_KEYn_CRYPTO_ENCR_XTS_KEY_BMSK                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_XTS_KEYn_CRYPTO_ENCR_XTS_KEY_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_ADDR(n)                                                (CRYPTO0_CRYPTO_REG_BASE      + 0X3040 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_RMSK                                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_MAXn                                                           15
#define HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_CRYPTO_AUTH_KEY_BMSK                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_AUTH_KEYn_CRYPTO_AUTH_KEY_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_ADDR(m)                                              (CRYPTO0_CRYPTO_REG_BASE      + 0X4004 + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_RMSK                                                        0x1
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_MAXm                                                          7
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_INI(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_DRM_EN_BMSK                                                 0x1
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_EN_DRM_EN_SHFT                                                   0

#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_ADDR(m)                                  (CRYPTO0_CRYPTO_REG_BASE      + 0X4008 + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_RMSK                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_MAXm                                              7
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_INI(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_RESERVED_31_6_BMSK                       0xffffffc0
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_RESERVED_31_6_SHFT                                6
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_DRM_DATA_INPUT_SID_BMSK                        0x3f
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_INPUT_SID_DRM_DATA_INPUT_SID_SHFT                           0

#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_ADDR(m)                                 (CRYPTO0_CRYPTO_REG_BASE      + 0X400C + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_RMSK                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_MAXm                                             7
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_INI(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_RESERVED_31_6_BMSK                      0xffffffc0
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_RESERVED_31_6_SHFT                               6
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_DRM_DATA_OUTPUT_SID_BMSK                      0x3f
#define HWIO_CRYPTO0_CRYPTO_KEYm_DRM_DATA_OUTPUT_SID_DRM_DATA_OUTPUT_SID_SHFT                         0

#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_ADDR(j)                                              (CRYPTO0_CRYPTO_REG_BASE      + 0X4004 + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_RMSK                                                        0x1
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_MAXj                                                         15
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_INI(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_DRM_EN_BMSK                                                 0x1
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_EN_DRM_EN_SHFT                                                   0

#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_ADDR(j)                                  (CRYPTO0_CRYPTO_REG_BASE      + 0X4008 + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_RMSK                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_MAXj                                             15
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_INI(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_RESERVED_31_6_BMSK                       0xffffffc0
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_RESERVED_31_6_SHFT                                6
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_DRM_DATA_INPUT_SID_BMSK                        0x3f
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_INPUT_SID_DRM_DATA_INPUT_SID_SHFT                           0

#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_ADDR(j)                                 (CRYPTO0_CRYPTO_REG_BASE      + 0X400C + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_RMSK                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_MAXj                                            15
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_INI(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_RESERVED_31_6_BMSK                      0xffffffc0
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_RESERVED_31_6_SHFT                               6
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_DRM_DATA_OUTPUT_SID_BMSK                      0x3f
#define HWIO_CRYPTO0_CRYPTO_KEYj_DRM_DATA_OUTPUT_SID_DRM_DATA_OUTPUT_SID_SHFT                         0

#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_ADDR(m)                                            (CRYPTO0_CRYPTO_REG_BASE      + 0X4104 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_RMSK                                                      0x1
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_MAXm                                                        7
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_INI(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_TIMER_ENABLE_BMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_EN_TIMER_ENABLE_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_ADDR(m)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X4108 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_RMSK                                         0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_MAXm                                                  7
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_INI(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_TIMER_LIMIT_VAL_LO_BMSK                      0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_LO_TIMER_LIMIT_VAL_LO_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_ADDR(m)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X410C + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_RMSK                                           0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_MAXm                                                  7
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_INI(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_TIMER_LIMIT_VAL_HI_BMSK                        0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_TIMER_LIMIT_HI_TIMER_LIMIT_VAL_HI_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_ADDR(m)                                           (CRYPTO0_CRYPTO_REG_BASE      + 0X4110 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_RMSK                                                 0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_MAXm                                                       7
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_INI(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_PROT_LVL_CHK_SEL_BMSK                                0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_CFG_PROT_LVL_CHK_SEL_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_ADDR(m)                                           (CRYPTO0_CRYPTO_REG_BASE      + 0X4114 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_RMSK                                                     0x1
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_MAXm                                                       7
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_INI(m))
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_DET_VAL_BMSK                                             0x1
#define HWIO_CRYPTO0_CRYPTO_KEYm_PAUSE_DET_DET_VAL_SHFT                                               0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_ADDR(m)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X4118 + (0x20*(m)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_RMSK                                                0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_MAXm                                                  7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_INI(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_HDCP_ENCR_EN_BMSK                                   0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_HDCP_ENCR_HDCP_ENCR_EN_SHFT                                     0

#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_ADDR(j)                                            (CRYPTO0_CRYPTO_REG_BASE      + 0X4104 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_RMSK                                                      0x1
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_MAXj                                                       15
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_INI(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_TIMER_ENABLE_BMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_EN_TIMER_ENABLE_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_ADDR(j)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X4108 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_RMSK                                         0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_MAXj                                                 15
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_INI(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_TIMER_LIMIT_VAL_LO_BMSK                      0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_LO_TIMER_LIMIT_VAL_LO_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_ADDR(j)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X410C + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_RMSK                                           0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_MAXj                                                 15
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_INI(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_TIMER_LIMIT_VAL_HI_BMSK                        0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_TIMER_LIMIT_HI_TIMER_LIMIT_VAL_HI_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_ADDR(j)                                           (CRYPTO0_CRYPTO_REG_BASE      + 0X4110 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_RMSK                                                 0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_MAXj                                                      15
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_INI(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_PROT_LVL_CHK_SEL_BMSK                                0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_CFG_PROT_LVL_CHK_SEL_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_ADDR(j)                                           (CRYPTO0_CRYPTO_REG_BASE      + 0X4114 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_RMSK                                                     0x1
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_MAXj                                                      15
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_INI(j))
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_DET_VAL_BMSK                                             0x1
#define HWIO_CRYPTO0_CRYPTO_KEYj_PAUSE_DET_DET_VAL_SHFT                                               0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_ADDR(j)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X4118 + (0x20*(j)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_RMSK                                                0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_MAXj                                                 15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_INI(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_HDCP_ENCR_EN_BMSK                                   0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_HDCP_ENCR_HDCP_ENCR_EN_SHFT                                     0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_ADDR(m,n)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0X4300 + (0x10*(m)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_RMSK                                          0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MAXm                                                   7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_MAXn                                                   3
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_INI2(m,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_ADDR(m,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_INMI2(m,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_ADDR(m,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_OUTI2(m,n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_ADDR(m,n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_OUTMI2(m,n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_ADDR(m,n),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_INI2(m,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_CNTR_IV_BMSK                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_IVn_CNTR_IV_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_ADDR(j,n)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0X4300 + (0x10*(j)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_RMSK                                          0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MAXj                                                  15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_MAXn                                                   3
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_INI2(j,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_ADDR(j,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_INMI2(j,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_ADDR(j,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_OUTI2(j,n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_ADDR(j,n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_OUTMI2(j,n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_ADDR(j,n),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_INI2(j,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_CNTR_IV_BMSK                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_IVn_CNTR_IV_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_ADDR(m)                             (CRYPTO0_CRYPTO_REG_BASE      + 0X4400 + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_RMSK                                0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_MAXm                                         7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_INI(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_FIRST_USE_TIMER_LO_BMSK             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_LO_FIRST_USE_TIMER_LO_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_ADDR(m)                             (CRYPTO0_CRYPTO_REG_BASE      + 0X4404 + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_RMSK                                  0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_MAXm                                         7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_INI(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_FIRST_USE_TIMER_HI_BMSK               0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_FIRST_USE_TIMER_HI_FIRST_USE_TIMER_HI_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_ADDR(m)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X4408 + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_MAXm                                          7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_LAST_USE_TIMER_LO_BMSK               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_LO_LAST_USE_TIMER_LO_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_ADDR(m)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X440C + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_RMSK                                   0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_MAXm                                          7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_LAST_USE_TIMER_HI_BMSK                 0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_LAST_USE_TIMER_HI_LAST_USE_TIMER_HI_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_ADDR(j)                             (CRYPTO0_CRYPTO_REG_BASE      + 0X4400 + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_RMSK                                0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_MAXj                                        15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_INI(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_FIRST_USE_TIMER_LO_BMSK             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_LO_FIRST_USE_TIMER_LO_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_ADDR(j)                             (CRYPTO0_CRYPTO_REG_BASE      + 0X4404 + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_RMSK                                  0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_MAXj                                        15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_INI(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_FIRST_USE_TIMER_HI_BMSK               0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_FIRST_USE_TIMER_HI_FIRST_USE_TIMER_HI_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_ADDR(j)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X4408 + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_MAXj                                         15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_LAST_USE_TIMER_LO_BMSK               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_LO_LAST_USE_TIMER_LO_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_ADDR(j)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X440C + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_RMSK                                   0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_MAXj                                         15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_LAST_USE_TIMER_HI_BMSK                 0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_LAST_USE_TIMER_HI_LAST_USE_TIMER_HI_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_ADDR(m,n)                                   (CRYPTO0_CRYPTO_REG_BASE      + 0X4500 + (0x10*(m)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_RMSK                                        0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MAXm                                                 7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_MAXn                                                 2
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_INI2(m,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_ADDR(m,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_INMI2(m,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_ADDR(m,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_OUTI2(m,n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_ADDR(m,n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_OUTMI2(m,n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_ADDR(m,n),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_INI2(m,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_CNTR_MASK_BMSK                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASKn_CNTR_MASK_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_ADDR(m)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0X450C + (0x10*(m)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_RMSK                                        0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_MAXm                                                 7
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_INI(m))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_CNTR_MASK_BMSK                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYm_CNTR_MASK3_CNTR_MASK_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_ADDR(j,n)                                   (CRYPTO0_CRYPTO_REG_BASE      + 0X4500 + (0x10*(j)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_RMSK                                        0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MAXj                                                15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_MAXn                                                 2
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_INI2(j,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_ADDR(j,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_INMI2(j,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_ADDR(j,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_OUTI2(j,n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_ADDR(j,n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_OUTMI2(j,n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_ADDR(j,n),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_INI2(j,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_CNTR_MASK_BMSK                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASKn_CNTR_MASK_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_ADDR(j)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0X450C + (0x10*(j)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_RMSK                                        0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_MAXj                                                15
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_INI(j)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_ADDR(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_INMI(j,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_ADDR(j), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_OUTI(j,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_ADDR(j),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_OUTMI(j,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_ADDR(j),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_INI(j))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_CNTR_MASK_BMSK                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYj_CNTR_MASK3_CNTR_MASK_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_ADDR(i)                                              (CRYPTO0_CRYPTO_REG_BASE      + 0X4704 + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_RMSK                                                        0x1
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_MAXi                                                         23
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_INI(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_DRM_EN_BMSK                                                 0x1
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_EN_DRM_EN_SHFT                                                   0

#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_ADDR(i)                                  (CRYPTO0_CRYPTO_REG_BASE      + 0X4708 + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_RMSK                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_MAXi                                             23
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_INI(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_RESERVED_31_6_BMSK                       0xffffffc0
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_RESERVED_31_6_SHFT                                6
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_DRM_DATA_INPUT_SID_BMSK                        0x3f
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_INPUT_SID_DRM_DATA_INPUT_SID_SHFT                           0

#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_ADDR(i)                                 (CRYPTO0_CRYPTO_REG_BASE      + 0X470C + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_RMSK                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_MAXi                                            23
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_INI(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_RESERVED_31_6_BMSK                      0xffffffc0
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_RESERVED_31_6_SHFT                               6
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_DRM_DATA_OUTPUT_SID_BMSK                      0x3f
#define HWIO_CRYPTO0_CRYPTO_KEYi_DRM_DATA_OUTPUT_SID_DRM_DATA_OUTPUT_SID_SHFT                         0

#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_ADDR(i)                                            (CRYPTO0_CRYPTO_REG_BASE      + 0X4704 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_RMSK                                                      0x1
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_MAXi                                                       23
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_INI(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_TIMER_ENABLE_BMSK                                         0x1
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_EN_TIMER_ENABLE_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_ADDR(i)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X4708 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_RMSK                                         0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_MAXi                                                 23
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_INI(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_TIMER_LIMIT_VAL_LO_BMSK                      0xffffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_LO_TIMER_LIMIT_VAL_LO_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_ADDR(i)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X470C + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_RMSK                                           0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_MAXi                                                 23
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_INI(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_TIMER_LIMIT_VAL_HI_BMSK                        0xffffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_TIMER_LIMIT_HI_TIMER_LIMIT_VAL_HI_SHFT                               0

#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_ADDR(i)                                           (CRYPTO0_CRYPTO_REG_BASE      + 0X4710 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_RMSK                                                 0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_MAXi                                                      23
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_INI(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_PROT_LVL_CHK_SEL_BMSK                                0x1ffff
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_CFG_PROT_LVL_CHK_SEL_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_ADDR(i)                                           (CRYPTO0_CRYPTO_REG_BASE      + 0X4714 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_RMSK                                                     0x1
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_MAXi                                                      23
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_INI(i))
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_DET_VAL_BMSK                                             0x1
#define HWIO_CRYPTO0_CRYPTO_KEYi_PAUSE_DET_DET_VAL_SHFT                                               0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_ADDR(i)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X4718 + (0x20*(i)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_RMSK                                                0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_MAXi                                                 23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_INI(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_HDCP_ENCR_EN_BMSK                                   0x1
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_HDCP_ENCR_HDCP_ENCR_EN_SHFT                                     0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_ADDR(i,n)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0X4A00 + (0x10*(i)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_RMSK                                          0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MAXi                                                  23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_MAXn                                                   3
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_INI2(i,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_ADDR(i,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_INMI2(i,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_ADDR(i,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_OUTI2(i,n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_ADDR(i,n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_OUTMI2(i,n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_ADDR(i,n),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_INI2(i,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_CNTR_IV_BMSK                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_IVn_CNTR_IV_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_ADDR(i)                             (CRYPTO0_CRYPTO_REG_BASE      + 0X4B00 + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_RMSK                                0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_MAXi                                        23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_INI(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_FIRST_USE_TIMER_LO_BMSK             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_LO_FIRST_USE_TIMER_LO_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_ADDR(i)                             (CRYPTO0_CRYPTO_REG_BASE      + 0X4B04 + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_RMSK                                  0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_MAXi                                        23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_INI(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_FIRST_USE_TIMER_HI_BMSK               0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_FIRST_USE_TIMER_HI_FIRST_USE_TIMER_HI_SHFT                      0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_ADDR(i)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X4B08 + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_RMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_MAXi                                         23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_LAST_USE_TIMER_LO_BMSK               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_LO_LAST_USE_TIMER_LO_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_ADDR(i)                              (CRYPTO0_CRYPTO_REG_BASE      + 0X4B0C + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_RMSK                                   0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_MAXi                                         23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_LAST_USE_TIMER_HI_BMSK                 0xffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_LAST_USE_TIMER_HI_LAST_USE_TIMER_HI_SHFT                        0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_ADDR(i,n)                                   (CRYPTO0_CRYPTO_REG_BASE      + 0X4C00 + (0x10*(i)) + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_RMSK                                        0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MAXi                                                23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_MAXn                                                 2
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_INI2(i,n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_ADDR(i,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_INMI2(i,n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_ADDR(i,n), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_OUTI2(i,n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_ADDR(i,n),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_OUTMI2(i,n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_ADDR(i,n),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_INI2(i,n))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_CNTR_MASK_BMSK                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASKn_CNTR_MASK_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_ADDR(i)                                     (CRYPTO0_CRYPTO_REG_BASE      + 0X4C0C + (0x10*(i)))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_RMSK                                        0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_MAXi                                                23
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_INI(i)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_ADDR(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_INMI(i,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_ADDR(i), mask)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_OUTI(i,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_ADDR(i),val)
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_OUTMI(i,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_ADDR(i),mask,val,HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_INI(i))
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_CNTR_MASK_BMSK                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_ENCR_KEYi_CNTR_MASK3_CNTR_MASK_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_ADDR                                                (CRYPTO0_CRYPTO_REG_BASE      + 0x5000)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_RMSK                                                     0x3ff
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_ADDR)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_IN)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_DBG_EN_BMSK                                              0x200
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_DBG_EN_SHFT                                                  9
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_MAXI2AXI_DBG_SEL_BMSK                                    0x1c0
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_MAXI2AXI_DBG_SEL_SHFT                                        6
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_DBG_SEL_BMSK                                              0x3f
#define HWIO_CRYPTO0_CRYPTO_DEBUG_ENABLE_DBG_SEL_SHFT                                                 0

#define HWIO_CRYPTO0_CRYPTO_DEBUG_ADDR                                                       (CRYPTO0_CRYPTO_REG_BASE      + 0x5004)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_RMSK                                                              0x0
#define HWIO_CRYPTO0_CRYPTO_DEBUG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_DEBUG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_DEBUG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_DEBUG_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_DEBUG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_DEBUG_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_DEBUG_IN)

#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_ADDR                                        (CRYPTO0_CRYPTO_REG_BASE      + 0x5008)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_RMSK                                               0x1
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_IN)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_TIMER_ENABLE_BMSK                                  0x1
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_EN_TIMER_ENABLE_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_ADDR                                        (CRYPTO0_CRYPTO_REG_BASE      + 0x500c)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_RMSK                                        0xffffffff
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_GLOBAL_TIMER_VAL_LO_BMSK                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_LO_GLOBAL_TIMER_VAL_LO_SHFT                             0

#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_ADDR                                        (CRYPTO0_CRYPTO_REG_BASE      + 0x5010)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_RMSK                                          0xffffff
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_ADDR)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_GLOBAL_TIMER_VAL_HI_BMSK                      0xffffff
#define HWIO_CRYPTO0_CRYPTO_PIPE_GLOBAL_TIMER_HI_GLOBAL_TIMER_VAL_HI_SHFT                             0

#define HWIO_CRYPTO0_CRYPTO_PIPE_KEY_PAUSE_SW_SRC_ADDR                                       (CRYPTO0_CRYPTO_REG_BASE      + 0x5014)
#define HWIO_CRYPTO0_CRYPTO_PIPE_KEY_PAUSE_SW_SRC_RMSK                                              0x1
#define HWIO_CRYPTO0_CRYPTO_PIPE_KEY_PAUSE_SW_SRC_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_PIPE_KEY_PAUSE_SW_SRC_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_PIPE_KEY_PAUSE_SW_SRC_KEY_PAUSE_SW_INPUT_BMSK                           0x1
#define HWIO_CRYPTO0_CRYPTO_PIPE_KEY_PAUSE_SW_SRC_KEY_PAUSE_SW_INPUT_SHFT                             0

#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_ADDR(m)                                      (CRYPTO0_CRYPTO_REG_BASE      + 0X5100 + (0x4*(m)))
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_RMSK                                               0x3f
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_MAXm                                                  9
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_INI(m)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_ADDR(m))
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_INMI(m,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_ADDR(m), mask)
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_OUTI(m,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_ADDR(m),val)
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_OUTMI(m,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_ADDR(m),mask,val,HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_INI(m))
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_NON_SECURE_VM_BMSK                                 0x3c
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_NON_SECURE_VM_SHFT                                    2
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_NON_SECURE_EE_BMSK                                  0x2
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_NON_SECURE_EE_SHFT                                    1
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_SECURE_EE_BMSK                                      0x1
#define HWIO_CRYPTO0_CRYPTO_PIPE_SETm_EE_POLICY_SECURE_EE_SHFT                                        0

#define HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_ADDR                                         (CRYPTO0_CRYPTO_REG_BASE      + 0x5200)
#define HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_RMSK                                                0x1
#define HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_ADDR)
#define HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_IN)
#define HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_INDEX_BASED_KEY_SEL_BMSK                            0x1
#define HWIO_CRYPTO0_CRYPTO_INDEX_BASED_KEY_SEL_INDEX_BASED_KEY_SEL_SHFT                              0

/*----------------------------------------------------------------------------
 * MODULE: CRYPTO0_CRYPTO_BAM
 *--------------------------------------------------------------------------*/

#define CRYPTO0_CRYPTO_BAM_REG_BASE                                                                   0x01dc4000
#define CRYPTO0_CRYPTO_BAM_REG_BASE_SIZE                                                              0x28000
#define CRYPTO0_CRYPTO_BAM_REG_BASE_USED                                                              0x27004

#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_ADDR                                                             (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x0)
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_RMSK                                                               0x1feff3
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_CTRL_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_CTRL_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_CTRL_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_CTRL_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_CTRL_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_MESS_ONLY_CANCEL_WB_BMSK                                       0x100000
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_MESS_ONLY_CANCEL_WB_SHFT                                             20
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_CACHE_MISS_ERR_RESP_EN_BMSK                                         0x80000
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_CACHE_MISS_ERR_RESP_EN_SHFT                                              19
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_LOCAL_CLK_GATING_BMSK                                               0x60000
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_LOCAL_CLK_GATING_SHFT                                                    17
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_IBC_DISABLE_BMSK                                                    0x10000
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_IBC_DISABLE_SHFT                                                         16
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_CACHED_DESC_STORE_BMSK                                           0x8000
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_CACHED_DESC_STORE_SHFT                                               15
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_DESC_CACHE_SEL_BMSK                                              0x6000
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_DESC_CACHE_SEL_SHFT                                                  13
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_TESTBUS_SEL_BMSK                                                  0xfe0
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_TESTBUS_SEL_SHFT                                                      5
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_EN_ACCUM_BMSK                                                      0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_EN_ACCUM_SHFT                                                         4
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_EN_BMSK                                                             0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_EN_SHFT                                                               1
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_SW_RST_BMSK                                                         0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_CTRL_BAM_SW_RST_SHFT                                                           0

#define HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_ADDR                                                  (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x8)
#define HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_RMSK                                                      0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_CNT_TRSHLD_BMSK                                           0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_DESC_CNT_TRSHLD_CNT_TRSHLD_SHFT                                                0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_ADDR                                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x14)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_RMSK                                                               0x1f
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_BAM_TIMER_IRQ_BMSK                                                 0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_BAM_TIMER_IRQ_SHFT                                                    4
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_BAM_EMPTY_IRQ_BMSK                                                  0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_BAM_EMPTY_IRQ_SHFT                                                    3
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_BAM_ERROR_IRQ_BMSK                                                  0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_BAM_ERROR_IRQ_SHFT                                                    2
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_BAM_HRESP_ERR_IRQ_BMSK                                              0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_BAM_HRESP_ERR_IRQ_SHFT                                                1
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_RESERVED_BITS0_BMSK                                                 0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_STTS_RESERVED_BITS0_SHFT                                                   0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_ADDR                                                          (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x18)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_RMSK                                                                0x1f
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_BAM_TIMER_CLR_BMSK                                                  0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_BAM_TIMER_CLR_SHFT                                                     4
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_BAM_EMPTY_CLR_BMSK                                                   0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_BAM_EMPTY_CLR_SHFT                                                     3
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_BAM_ERROR_CLR_BMSK                                                   0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_BAM_ERROR_CLR_SHFT                                                     2
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_BAM_HRESP_ERR_CLR_BMSK                                               0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_BAM_HRESP_ERR_CLR_SHFT                                                 1
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_RESERVED_BITS0_BMSK                                                  0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_CLR_RESERVED_BITS0_SHFT                                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_ADDR                                                           (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1c)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_RMSK                                                                 0x1f
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_BAM_TIMER_EN_BMSK                                                    0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_BAM_TIMER_EN_SHFT                                                       4
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_BAM_EMPTY_EN_BMSK                                                     0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_BAM_EMPTY_EN_SHFT                                                       3
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_BAM_ERROR_EN_BMSK                                                     0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_BAM_ERROR_EN_SHFT                                                       2
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_BAM_HRESP_ERR_EN_BMSK                                                 0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_BAM_HRESP_ERR_EN_SHFT                                                   1
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_RESERVED_BITS0_BMSK                                                   0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_EN_RESERVED_BITS0_SHFT                                                     0

#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_ADDR                                                        (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x7c)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_RMSK                                                        0xfffff80f
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_AOS_OVERFLOW_PRVNT_BMSK                                     0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_AOS_OVERFLOW_PRVNT_SHFT                                             31
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_MULTIPLE_EVENTS_DESC_AVAIL_EN_BMSK                          0x40000000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_MULTIPLE_EVENTS_DESC_AVAIL_EN_SHFT                                  30
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_MULTIPLE_EVENTS_SIZE_EN_BMSK                                0x20000000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_MULTIPLE_EVENTS_SIZE_EN_SHFT                                        29
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_ZLT_W_CD_SUPPORT_BMSK                                   0x10000000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_ZLT_W_CD_SUPPORT_SHFT                                           28
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_CD_ENABLE_BMSK                                           0x8000000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_CD_ENABLE_SHFT                                                  27
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_AU_ACCUMED_BMSK                                          0x4000000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_AU_ACCUMED_SHFT                                                 26
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_PSM_P_HD_DATA_BMSK                                       0x2000000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_PSM_P_HD_DATA_SHFT                                              25
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_REG_P_EN_BMSK                                            0x1000000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_REG_P_EN_SHFT                                                   24
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_DSC_AVL_P_RST_BMSK                                     0x800000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_DSC_AVL_P_RST_SHFT                                           23
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_RETR_SVPNT_BMSK                                        0x400000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_RETR_SVPNT_SHFT                                              22
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_CSW_ACK_IDL_BMSK                                       0x200000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_CSW_ACK_IDL_SHFT                                             21
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_BLK_CSW_BMSK                                           0x100000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_BLK_CSW_SHFT                                                 20
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_P_RES_BMSK                                              0x80000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_WB_P_RES_SHFT                                                   19
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_SI_P_RES_BMSK                                              0x40000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_SI_P_RES_SHFT                                                   18
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_AU_P_RES_BMSK                                              0x20000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_AU_P_RES_SHFT                                                   17
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_PSM_P_RES_BMSK                                             0x10000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_PSM_P_RES_SHFT                                                  16
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_PSM_CSW_REQ_BMSK                                            0x8000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_PSM_CSW_REQ_SHFT                                                15
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_SB_CLK_REQ_BMSK                                             0x4000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_SB_CLK_REQ_SHFT                                                 14
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_IBC_DISABLE_BMSK                                            0x2000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_IBC_DISABLE_SHFT                                                13
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_NO_EXT_P_RST_BMSK                                           0x1000
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_NO_EXT_P_RST_SHFT                                               12
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_FULL_PIPE_BMSK                                               0x800
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_FULL_PIPE_SHFT                                                  11
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_ADML_SYNC_BRIDGE_BMSK                                          0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_ADML_SYNC_BRIDGE_SHFT                                            3
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_PIPE_CNFG_BMSK                                                 0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_PIPE_CNFG_SHFT                                                   2
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_ADML_DEEP_CONS_FIFO_BMSK                                       0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_ADML_DEEP_CONS_FIFO_SHFT                                         1
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_ADML_INCR4_EN_N_BMSK                                           0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_BAM_ADML_INCR4_EN_N_SHFT                                             0

#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_ADDR                                                      (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x84)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_RMSK                                                      0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_RESERVED_BITS31_4_BMSK                                    0xfffffff0
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_RESERVED_BITS31_4_SHFT                                             4
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_SUP_GRP_LOCKER_RST_SUPPORT_BMSK                                  0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_SUP_GRP_LOCKER_RST_SUPPORT_SHFT                                    3
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_ACTIVE_PIPE_RST_SUPPORT_BMSK                                     0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_ACTIVE_PIPE_RST_SUPPORT_SHFT                                       2
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_NO_SW_OFFSET_REVERT_BACK_BMSK                                    0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_NO_SW_OFFSET_REVERT_BACK_SHFT                                      1
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_CNFG_NO_ACCEPT_AT_FIFO_FULL_BMSK                                 0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_CNFG_BITS_2_CNFG_NO_ACCEPT_AT_FIFO_FULL_SHFT                                   0

#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_ADDR                                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1000)
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_RMSK                                                         0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_REVISION_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_REVISION_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_INACTIV_TMR_BASE_BMSK                                        0xff000000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_INACTIV_TMR_BASE_SHFT                                                24
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_CMD_DESC_EN_BMSK                                               0x800000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_CMD_DESC_EN_SHFT                                                     23
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_DESC_CACHE_DEPTH_BMSK                                          0x600000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_DESC_CACHE_DEPTH_SHFT                                                21
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_NUM_INACTIV_TMRS_BMSK                                          0x100000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_NUM_INACTIV_TMRS_SHFT                                                20
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_INACTIV_TMRS_EXST_BMSK                                          0x80000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_INACTIV_TMRS_EXST_SHFT                                               19
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_HIGH_FREQUENCY_BAM_BMSK                                         0x40000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_HIGH_FREQUENCY_BAM_SHFT                                              18
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_BAM_HAS_NO_BYPASS_BMSK                                          0x20000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_BAM_HAS_NO_BYPASS_SHFT                                               17
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_SECURED_BMSK                                                    0x10000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_SECURED_SHFT                                                         16
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_USE_VMIDMT_BMSK                                                  0x8000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_USE_VMIDMT_SHFT                                                      15
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_AXI_ACTIVE_BMSK                                                  0x4000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_AXI_ACTIVE_SHFT                                                      14
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_CE_BUFFER_SIZE_BMSK                                              0x3000
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_CE_BUFFER_SIZE_SHFT                                                  12
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_NUM_EES_BMSK                                                      0xf00
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_NUM_EES_SHFT                                                          8
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_REVISION_BMSK                                                      0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_REVISION_REVISION_SHFT                                                         0

#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_ADDR                                                       (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1004)
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_RMSK                                                       0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_MAJOR_BMSK                                                 0xf0000000
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_MAJOR_SHFT                                                         28
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_MINOR_BMSK                                                  0xfff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_MINOR_SHFT                                                         16
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_STEP_BMSK                                                      0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_SW_VERSION_STEP_SHFT                                                           0

#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_ADDR                                                        (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1008)
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_RMSK                                                        0xffffc0ff
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_BAM_NON_PIPE_GRP_BMSK                                       0xff000000
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_BAM_NON_PIPE_GRP_SHFT                                               24
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_PERIPH_NON_PIPE_GRP_BMSK                                      0xff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_PERIPH_NON_PIPE_GRP_SHFT                                            16
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_BAM_DATA_ADDR_BUS_WIDTH_BMSK                                    0xc000
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_BAM_DATA_ADDR_BUS_WIDTH_SHFT                                        14
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_BAM_NUM_PIPES_BMSK                                                0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_NUM_PIPES_BAM_NUM_PIPES_SHFT                                                   0

#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_ADDR                                                     (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1010)
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_RMSK                                                       0x3f007f
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_SW_EVENTS_ZERO_BMSK                                        0x200000
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_SW_EVENTS_ZERO_SHFT                                              21
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_SW_EVENTS_SEL_BMSK                                         0x180000
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_SW_EVENTS_SEL_SHFT                                               19
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_BAM_DATA_ERASE_BMSK                                         0x40000
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_BAM_DATA_ERASE_SHFT                                              18
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_BAM_DATA_FLUSH_BMSK                                         0x20000
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_BAM_DATA_FLUSH_SHFT                                              17
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_BAM_CLK_ALWAYS_ON_BMSK                                      0x10000
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_BAM_CLK_ALWAYS_ON_SHFT                                           16
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_BAM_TESTBUS_SEL_BMSK                                           0x7f
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_SEL_BAM_TESTBUS_SEL_SHFT                                              0

#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_REG_ADDR                                                     (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1014)
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_REG_RMSK                                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_REG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_REG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_REG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_REG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_REG_BAM_TESTBUS_REG_BMSK                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_TEST_BUS_REG_BAM_TESTBUS_REG_SHFT                                              0

#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_ADDR                                             (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1024)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_RMSK                                              0x7ffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_CONFIG_IDLE_BMSK                              0x4000000
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_CONFIG_IDLE_SHFT                                     26
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_RB_IDLE_BMSK                                  0x2000000
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_RB_IDLE_SHFT                                         25
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_OPB_IDLE_BMSK                                 0x1000000
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_OPB_IDLE_SHFT                                        24
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_IDLE_BMSK                                      0x800000
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_IDLE_SHFT                                            23
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HVMID_BMSK                                 0x7c0000
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HVMID_SHFT                                       18
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_DIRECT_MODE_BMSK                            0x20000
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_DIRECT_MODE_SHFT                                 17
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HCID_BMSK                                   0x1f000
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HCID_SHFT                                        12
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HPROT_BMSK                                    0xf00
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HPROT_SHFT                                        8
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HBURST_BMSK                                    0xe0
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HBURST_SHFT                                       5
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HSIZE_BMSK                                     0x18
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HSIZE_SHFT                                        3
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HWRITE_BMSK                                     0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HWRITE_SHFT                                       2
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HTRANS_BMSK                                     0x3
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_CTRLS_BAM_ERR_HTRANS_SHFT                                       0

#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_ADDR                                              (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1028)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_BAM_ERR_ADDR_BMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_BAM_ERR_ADDR_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_DATA_ADDR                                              (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x102c)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_DATA_RMSK                                              0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_DATA_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_DATA_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_DATA_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_DATA_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_DATA_BAM_ERR_DATA_BMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_DATA_BAM_ERR_DATA_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_LSB_ADDR                                          (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1100)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_LSB_RMSK                                          0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_LSB_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_LSB_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_LSB_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_LSB_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_LSB_BAM_ERR_ADDR_BMSK                             0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_LSB_BAM_ERR_ADDR_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_MSB_ADDR                                          (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x1104)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_MSB_RMSK                                                0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_MSB_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_MSB_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_MSB_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_MSB_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_MSB_BAM_ERR_ADDR_BMSK                                   0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_AHB_MASTER_ERR_ADDR_MSB_BAM_ERR_ADDR_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_ADDR                                                        (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x2000)
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_RMSK                                                            0x3f87
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_LOCK_EE_CTRL_BMSK                                               0x2000
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_LOCK_EE_CTRL_SHFT                                                   13
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_BAM_VMID_BMSK                                                   0x1f00
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_BAM_VMID_SHFT                                                        8
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_BAM_RST_BLOCK_BMSK                                                0x80
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_BAM_RST_BLOCK_SHFT                                                   7
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_BAM_EE_BMSK                                                        0x7
#define HWIO_CRYPTO0_CRYPTO_BAM_TRUST_REG_BAM_EE_SHFT                                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_ADDR(n)                                                  (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X2020 + (0x4*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_RMSK                                                         0x1fff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_MAXn                                                             19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_BAM_P_VMID_BMSK                                              0x1f00
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_BAM_P_VMID_SHFT                                                   8
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_BAM_P_SUP_GROUP_BMSK                                           0xf8
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_BAM_P_SUP_GROUP_SHFT                                              3
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_BAM_P_EE_BMSK                                                   0x7
#define HWIO_CRYPTO0_CRYPTO_BAM_P_TRUST_REGn_BAM_P_EE_SHFT                                                     0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_ADDR(n)                                                  (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X3000 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_RMSK                                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_MAXn                                                              3
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_BAM_IRQ_BMSK                                             0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_BAM_IRQ_SHFT                                                     31
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_P_IRQ_BMSK                                               0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_EEn_P_IRQ_SHFT                                                        0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_ADDR(n)                                              (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X3004 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_RMSK                                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_MAXn                                                          3
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_BAM_IRQ_MSK_BMSK                                     0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_BAM_IRQ_MSK_SHFT                                             31
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_P_IRQ_MSK_BMSK                                       0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_EEn_P_IRQ_MSK_SHFT                                                0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_ADDR(n)                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X3008 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_RMSK                                            0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_MAXn                                                     3
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_BAM_IRQ_UNMASKED_BMSK                           0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_BAM_IRQ_UNMASKED_SHFT                                   31
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_P_IRQ_UNMASKED_BMSK                             0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_EEn_P_IRQ_UNMASKED_SHFT                                      0

#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_ADDR(n)                                                 (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X300C + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_RMSK                                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_MAXn                                                             3
#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_BAM_ENABLED_BMSK                                        0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_BAM_ENABLED_SHFT                                                31
#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_P_ATTR_BMSK                                             0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_PIPE_ATTR_EEn_P_ATTR_SHFT                                                      0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_ADDR                                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x3010)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_RMSK                                                         0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_BAM_IRQ_BMSK                                                 0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_BAM_IRQ_SHFT                                                         31
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_P_IRQ_BMSK                                                   0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_P_IRQ_SHFT                                                            0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_ADDR                                                     (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x3014)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_RMSK                                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_BAM_IRQ_MSK_BMSK                                         0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_BAM_IRQ_MSK_SHFT                                                 31
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_P_IRQ_MSK_BMSK                                           0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_MSK_P_IRQ_MSK_SHFT                                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_ADDR                                                (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x3018)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_RMSK                                                0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_BAM_IRQ_UNMASKED_BMSK                               0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_BAM_IRQ_UNMASKED_SHFT                                       31
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_P_IRQ_UNMASKED_BMSK                                 0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_IRQ_SRCS_UNMASKED_P_IRQ_UNMASKED_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_ADDR(n)                                                       (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13000 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_RMSK                                                            0x1f0ffa
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_MAXn                                                                  19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_LOCK_GROUP_BMSK                                               0x1f0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_LOCK_GROUP_SHFT                                                     16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_WRITE_NWD_BMSK                                                   0x800
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_WRITE_NWD_SHFT                                                      11
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_PREFETCH_LIMIT_BMSK                                              0x600
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_PREFETCH_LIMIT_SHFT                                                  9
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_AUTO_EOB_SEL_BMSK                                                0x180
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_AUTO_EOB_SEL_SHFT                                                    7
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_AUTO_EOB_BMSK                                                     0x40
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_AUTO_EOB_SHFT                                                        6
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_SYS_MODE_BMSK                                                     0x20
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_SYS_MODE_SHFT                                                        5
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_SYS_STRM_BMSK                                                     0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_SYS_STRM_SHFT                                                        4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_DIRECTION_BMSK                                                     0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_DIRECTION_SHFT                                                       3
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_EN_BMSK                                                            0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CTRLn_P_EN_SHFT                                                              1

#define HWIO_CRYPTO0_CRYPTO_BAM_P_RSTn_ADDR(n)                                                        (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13004 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RSTn_RMSK                                                                  0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RSTn_MAXn                                                                   19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RSTn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_RSTn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RSTn_P_SW_RST_BMSK                                                         0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RSTn_P_SW_RST_SHFT                                                           0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_ADDR(n)                                                       (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13008 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_RMSK                                                                0x1f
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_MAXn                                                                  19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_FORCE_DESC_FIFO_FULL_BMSK                                         0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_FORCE_DESC_FIFO_FULL_SHFT                                            4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_PIPE_EMPTY_BMSK                                                    0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_PIPE_EMPTY_SHFT                                                      3
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_LAST_DESC_ZLT_BMSK                                                 0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_LAST_DESC_ZLT_SHFT                                                   2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_PROD_HALTED_BMSK                                                   0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_PROD_HALTED_SHFT                                                     1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_HALT_BMSK                                                          0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_HALTn_P_HALT_SHFT                                                            0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_ADDR(n)                                                   (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13010 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_RMSK                                                            0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_MAXn                                                              19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_HRESP_ERR_IRQ_BMSK                                            0x80
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_HRESP_ERR_IRQ_SHFT                                               7
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_PIPE_RST_ERROR_IRQ_BMSK                                       0x40
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_PIPE_RST_ERROR_IRQ_SHFT                                          6
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_TRNSFR_END_IRQ_BMSK                                           0x20
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_TRNSFR_END_IRQ_SHFT                                              5
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_ERR_IRQ_BMSK                                                  0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_ERR_IRQ_SHFT                                                     4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_OUT_OF_DESC_IRQ_BMSK                                           0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_OUT_OF_DESC_IRQ_SHFT                                             3
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_WAKE_IRQ_BMSK                                                  0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_WAKE_IRQ_SHFT                                                    2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_TIMER_IRQ_BMSK                                                 0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_TIMER_IRQ_SHFT                                                   1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_PRCSD_DESC_IRQ_BMSK                                            0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_STTSn_P_PRCSD_DESC_IRQ_SHFT                                              0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_ADDR(n)                                                    (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13014 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_RMSK                                                             0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_MAXn                                                               19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_HRESP_ERR_CLR_BMSK                                             0x80
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_HRESP_ERR_CLR_SHFT                                                7
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_PIPE_RST_ERROR_CLR_BMSK                                        0x40
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_PIPE_RST_ERROR_CLR_SHFT                                           6
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_TRNSFR_END_CLR_BMSK                                            0x20
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_TRNSFR_END_CLR_SHFT                                               5
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_ERR_CLR_BMSK                                                   0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_ERR_CLR_SHFT                                                      4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_OUT_OF_DESC_CLR_BMSK                                            0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_OUT_OF_DESC_CLR_SHFT                                              3
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_WAKE_CLR_BMSK                                                   0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_WAKE_CLR_SHFT                                                     2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_TIMER_CLR_BMSK                                                  0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_TIMER_CLR_SHFT                                                    1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_PRCSD_DESC_CLR_BMSK                                             0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_CLRn_P_PRCSD_DESC_CLR_SHFT                                               0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_ADDR(n)                                                     (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13018 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_RMSK                                                              0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_MAXn                                                                19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_HRESP_ERR_EN_BMSK                                               0x80
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_HRESP_ERR_EN_SHFT                                                  7
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_PIPE_RST_ERROR_EN_BMSK                                          0x40
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_PIPE_RST_ERROR_EN_SHFT                                             6
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_TRNSFR_END_EN_BMSK                                              0x20
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_TRNSFR_END_EN_SHFT                                                 5
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_ERR_EN_BMSK                                                     0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_ERR_EN_SHFT                                                        4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_OUT_OF_DESC_EN_BMSK                                              0x8
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_OUT_OF_DESC_EN_SHFT                                                3
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_WAKE_EN_BMSK                                                     0x4
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_WAKE_EN_SHFT                                                       2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_TIMER_EN_BMSK                                                    0x2
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_TIMER_EN_SHFT                                                      1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_PRCSD_DESC_EN_BMSK                                               0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_P_IRQ_ENn_P_PRCSD_DESC_EN_SHFT                                                 0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_ADDR(n)                                                (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13024 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_RMSK                                                    0x11fffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_MAXn                                                           19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_BAM_P_SB_UPDATED_BMSK                                   0x1000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_BAM_P_SB_UPDATED_SHFT                                          24
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_BAM_P_TOGGLE_BMSK                                        0x100000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_BAM_P_TOGGLE_SHFT                                              20
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_BAM_P_CTRL_BMSK                                           0xf0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_BAM_P_CTRL_SHFT                                                16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_BAM_P_BYTES_FREE_BMSK                                      0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PRDCR_SDBNDn_BAM_P_BYTES_FREE_SHFT                                           0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_ADDR(n)                                                (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13028 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_RMSK                                                   0x7fffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_MAXn                                                           19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACCEPT_ACK_ON_SUCCESS_TOGGLE_BMSK                0x40000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACCEPT_ACK_ON_SUCCESS_TOGGLE_SHFT                        30
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACK_ON_SUCCESS_CTRL_BMSK                         0x30000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACK_ON_SUCCESS_CTRL_SHFT                                 28
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACK_ON_SUCCESS_TOGGLE_BMSK                        0x8000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACK_ON_SUCCESS_TOGGLE_SHFT                               27
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_SB_UPDATED_BMSK                                   0x4000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_SB_UPDATED_SHFT                                          26
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_NWD_TOGGLE_BMSK                                   0x2000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_NWD_TOGGLE_SHFT                                          25
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_NWD_TOGGLE_R_BMSK                                 0x1000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_NWD_TOGGLE_R_SHFT                                        24
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_WAIT_4_ACK_BMSK                                    0x800000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_WAIT_4_ACK_SHFT                                          23
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACK_TOGGLE_BMSK                                    0x400000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACK_TOGGLE_SHFT                                          22
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACK_TOGGLE_R_BMSK                                  0x200000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_ACK_TOGGLE_R_SHFT                                        21
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_TOGGLE_BMSK                                        0x100000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_TOGGLE_SHFT                                              20
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_CTRL_BMSK                                           0xf0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_CTRL_SHFT                                                16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_BYTES_AVAIL_BMSK                                     0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_CNSMR_SDBNDn_BAM_P_BYTES_AVAIL_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_ADDR(n)                                                   (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13800 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_RMSK                                                      0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_MAXn                                                              19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_SW_OFST_IN_DESC_BMSK                                      0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_SW_OFST_IN_DESC_SHFT                                              16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_SW_DESC_OFST_BMSK                                             0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SW_OFSTSn_SW_DESC_OFST_SHFT                                                  0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_ADDR(n)                                            (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13804 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_MAXn                                                       19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_AU_PSM_ACCUMED_BMSK                                0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_AU_PSM_ACCUMED_SHFT                                        16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_AU_ACKED_BMSK                                          0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_AU_PSM_CNTXT_1_n_AU_ACKED_SHFT                                               0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_ADDR(n)                                               (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13808 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_RMSK                                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_MAXn                                                          19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_DESC_VALID_BMSK                                   0x80000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_DESC_VALID_SHFT                                           31
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_DESC_IRQ_BMSK                                     0x40000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_DESC_IRQ_SHFT                                             30
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_DESC_IRQ_DONE_BMSK                                0x20000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_DESC_IRQ_DONE_SHFT                                        29
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_GENERAL_BITS_BMSK                                 0x1e000000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_GENERAL_BITS_SHFT                                         25
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_CONS_STATE_BMSK                                    0x1c00000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_CONS_STATE_SHFT                                           22
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_PROD_SYS_STATE_BMSK                                 0x380000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_PROD_SYS_STATE_SHFT                                       19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_PROD_B2B_STATE_BMSK                                  0x70000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_PROD_B2B_STATE_SHFT                                       16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_DESC_SIZE_BMSK                                        0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_2_n_PSM_DESC_SIZE_SHFT                                             0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_ADDR(n)                                               (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X1380C + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_RMSK                                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_MAXn                                                          19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_PSM_DESC_ADDR_BMSK                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_n_PSM_DESC_ADDR_SHFT                                             0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_ADDR(n)                                               (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13810 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_RMSK                                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_MAXn                                                          19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_PSM_DESC_OFST_BMSK                                    0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_PSM_DESC_OFST_SHFT                                            16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_PSM_SAVED_ACCUMED_SIZE_BMSK                               0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_4_n_PSM_SAVED_ACCUMED_SIZE_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_ADDR(n)                                               (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13814 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_RMSK                                                  0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_MAXn                                                          19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_PSM_BLOCK_BYTE_CNT_BMSK                               0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_PSM_BLOCK_BYTE_CNT_SHFT                                       16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_PSM_OFST_IN_DESC_BMSK                                     0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_5_n_PSM_OFST_IN_DESC_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_ADDR(n)                                                   (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13818 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_RMSK                                                      0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_MAXn                                                              19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_P_BYTES_CONSUMED_BMSK                                     0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_P_BYTES_CONSUMED_SHFT                                             16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_P_DESC_FIFO_PEER_OFST_BMSK                                    0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_REGn_P_DESC_FIFO_PEER_OFST_SHFT                                         0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_ADDR(n)                                             (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X1381C + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_RMSK                                                0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_MAXn                                                        19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_P_DESC_FIFO_ADDR_BMSK                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDRn_P_DESC_FIFO_ADDR_SHFT                                        0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_ADDR(n)                                                 (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13820 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_RMSK                                                    0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_MAXn                                                            19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_P_DATA_FIFO_SIZE_BMSK                                   0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_P_DATA_FIFO_SIZE_SHFT                                           16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_P_DESC_FIFO_SIZE_BMSK                                       0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_FIFO_SIZESn_P_DESC_FIFO_SIZE_SHFT                                            0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_ADDR(n)                                             (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13824 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_RMSK                                                0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_MAXn                                                        19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_P_DATA_FIFO_ADDR_BMSK                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDRn_P_DATA_FIFO_ADDR_SHFT                                        0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_ADDR(n)                                            (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13828 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_MAXn                                                       19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_RESERVED_BITS31_16_BMSK                            0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_RESERVED_BITS31_16_SHFT                                    16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_P_TRSHLD_BMSK                                          0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_GEN_TRSHLDn_P_TRSHLD_SHFT                                               0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_ADDR(n)                                             (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X1382C + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_RMSK                                                0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_MAXn                                                        19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_P_EVNT_DEST_ADDR_BMSK                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDRn_P_EVNT_DEST_ADDR_SHFT                                        0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_ADDR(n)                                                  (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13830 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_RMSK                                                     0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_MAXn                                                             19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_WB_ACCUMULATED_BMSK                                      0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_WB_ACCUMULATED_SHFT                                              16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_DF_DESC_OFST_BMSK                                            0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DF_CNTXT_n_DF_DESC_OFST_SHFT                                                 0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_ADDR(n)                                                (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13834 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_RMSK                                                   0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_MAXn                                                           19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_RETR_DESC_OFST_BMSK                                    0xffff0000
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_RETR_DESC_OFST_SHFT                                            16
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_RETR_OFST_IN_DESC_BMSK                                     0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_RETR_CNTXT_n_RETR_OFST_IN_DESC_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_ADDR(n)                                                  (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13838 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_RMSK                                                         0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_MAXn                                                             19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_SI_DESC_OFST_BMSK                                            0xffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_SI_CNTXT_n_SI_DESC_OFST_SHFT                                                 0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_ADDR(n)                                            (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13900 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_RMSK                                               0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_MAXn                                                       19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_PSM_DESC_ADDR_BMSK                                 0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_LSBn_PSM_DESC_ADDR_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_ADDR(n)                                            (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13904 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_RMSK                                                     0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_MAXn                                                       19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_PSM_DESC_ADDR_BMSK                                       0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_PSM_CNTXT_3_MSBn_PSM_DESC_ADDR_SHFT                                          0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_ADDR(n)                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13910 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_RMSK                                            0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_MAXn                                                    19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_P_DESC_FIFO_ADDR_BMSK                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_LSBn_P_DESC_FIFO_ADDR_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_ADDR(n)                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13914 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_RMSK                                                  0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_MAXn                                                    19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_P_DESC_FIFO_ADDR_BMSK                                 0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DESC_FIFO_ADDR_MSBn_P_DESC_FIFO_ADDR_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_ADDR(n)                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13920 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_RMSK                                            0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_MAXn                                                    19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_P_DATA_FIFO_ADDR_BMSK                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_LSBn_P_DATA_FIFO_ADDR_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_ADDR(n)                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13924 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_RMSK                                                  0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_MAXn                                                    19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_P_DATA_FIFO_ADDR_BMSK                                 0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_DATA_FIFO_ADDR_MSBn_P_DATA_FIFO_ADDR_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_ADDR(n)                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13930 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_RMSK                                            0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_MAXn                                                    19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_P_EVNT_DEST_ADDR_BMSK                           0xffffffff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_LSBn_P_EVNT_DEST_ADDR_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_ADDR(n)                                         (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0X13934 + (0x1000*(n)))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_RMSK                                                  0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_MAXn                                                    19
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_INI(n)                \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_ADDR(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_INMI(n,mask)        \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_ADDR(n), mask)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_OUTI(n,val)        \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_ADDR(n),val)
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_OUTMI(n,mask,val) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_ADDR(n),mask,val,HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_INI(n))
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_P_EVNT_DEST_ADDR_BMSK                                 0xff
#define HWIO_CRYPTO0_CRYPTO_BAM_P_EVNT_DEST_ADDR_MSBn_P_EVNT_DEST_ADDR_SHFT                                    0

#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_ADDR                                               (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x27000)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_RMSK                                                     0x3f
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_BAM_SEC_INTRA_XPU_STATIC_CFG_APSVIOE_BMSK                0x20
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_BAM_SEC_INTRA_XPU_STATIC_CFG_APSVIOE_SHFT                   5
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_BAM_SEC_INTRA_XPU_STATIC_CFG_SLE_BMSK                    0x10
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_BAM_SEC_INTRA_XPU_STATIC_CFG_SLE_SHFT                       4
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_BAM_SEC_INTRA_XPU_STATIC_CFG_BMSK                         0xe
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_BAM_SEC_INTRA_XPU_STATIC_CFG_SHFT                           1
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_BAM_SEC_INTRA_XPU_STATIC_CFG_CFGOWNS_BMSK                 0x1
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG0_BAM_SEC_INTRA_XPU_STATIC_CFG_CFGOWNS_SHFT                   0

#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_ADDR                                               (CRYPTO0_CRYPTO_BAM_REG_BASE      + 0x27004)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_RMSK                                                     0x3f
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_IN                    \
                in_dword(HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_ADDR)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_INM(m)            \
                in_dword_masked(HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_ADDR, m)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_OUT(v)            \
                out_dword(HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_ADDR,v)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_OUTM(m,v) \
                out_dword_masked_ns(HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_ADDR,m,v,HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_IN)
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_RSVD_BMSK                                                0x3f
#define HWIO_CRYPTO0_CRYPTO_BAM_SEC_EXTND_CFG_REG1_RSVD_SHFT                                                   0


#endif /* HWIO_H */
