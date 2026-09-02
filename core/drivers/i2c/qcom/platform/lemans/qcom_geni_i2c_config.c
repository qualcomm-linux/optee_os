/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/qcom/tlmm/tlmm.h>
#include <drivers/qcom_geni_i2c.h>
#include <mm/core_mmu.h>
#include <util.h>

#define CFG_QUP2_SE2_I2C_EN

const uint8_t i2c_qup_fw[] =
{
    0x53, 0x45, 0x46, 0x57, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x30, 0x03, 0x00, 0x03, 0x01,
    0x09, 0x00, 0x2f, 0x01, 0x1c, 0x00, 0x45, 0x00, 0xec, 0x05, 0xd8, 0x04, 0xde, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x0a, 0x00,
    0x20, 0x6d, 0x4a, 0x00, 0x00, 0x48, 0x0a, 0x00, 0x10, 0x6d, 0x4a, 0x00, 0x00, 0x68, 0x0a, 0x00,
    0x10, 0x6d, 0x4a, 0x00, 0x00, 0x08, 0x0a, 0x00, 0x28, 0x6d, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x90, 0x00, 0x08, 0x6d, 0x4a, 0x00, 0x00, 0x08, 0x87, 0x00,
    0x08, 0x42, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe7, 0x6f, 0x7f, 0x00,
    0x20, 0x08, 0x7f, 0x00, 0xb1, 0x11, 0x9b, 0x00, 0x31, 0x41, 0x9a, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x04, 0x20, 0x7f, 0x00, 0x3d, 0x20, 0x9b, 0x00, 0x3d, 0x08, 0x9b, 0x00, 0x3c, 0x38, 0x9f, 0x00,
    0x97, 0x79, 0x9f, 0x00, 0x61, 0x01, 0x9c, 0x00, 0x00, 0x00, 0x09, 0x00, 0x81, 0x38, 0x7f, 0x00,
    0x67, 0x09, 0x9c, 0x00, 0x00, 0x00, 0x09, 0x00, 0xa0, 0x08, 0x7f, 0x00, 0x40, 0x10, 0x7f, 0x00,
    0x51, 0x30, 0x9f, 0x00, 0x50, 0x38, 0x9f, 0x00, 0x47, 0x78, 0x9f, 0x00, 0x28, 0x08, 0x7f, 0x00,
    0x00, 0x10, 0x7f, 0x00, 0x54, 0x18, 0x9f, 0x00, 0xf0, 0x33, 0x3f, 0x00, 0x07, 0x04, 0x65, 0x00,
    0x00, 0x30, 0x1f, 0x00, 0x06, 0x39, 0x7f, 0x00, 0x71, 0x79, 0x9d, 0x00, 0x00, 0x00, 0x0a, 0x00,
    0x63, 0x60, 0x9f, 0x00, 0x00, 0x00, 0x07, 0x00, 0x06, 0x39, 0x7f, 0x00, 0x66, 0x18, 0x9f, 0x00,
    0x28, 0x89, 0x9f, 0x00, 0x5f, 0x50, 0x9f, 0x00, 0xb5, 0x11, 0x9b, 0x00, 0x74, 0x38, 0x9f, 0x00,
    0xb5, 0x19, 0x9b, 0x00, 0x00, 0x00, 0x09, 0x00, 0x76, 0x28, 0x9f, 0x00, 0x04, 0x3a, 0x67, 0x00,
    0x05, 0x39, 0x7f, 0x00, 0x80, 0x01, 0x3f, 0x00, 0x01, 0x20, 0x7f, 0x00, 0x81, 0x60, 0x9f, 0x00,
    0x00, 0x00, 0x07, 0x00, 0x06, 0x39, 0x7f, 0x00, 0x84, 0x18, 0x9f, 0x00, 0x28, 0x09, 0x9f, 0x00,
    0x8a, 0x28, 0x9f, 0x00, 0x04, 0x3a, 0x67, 0x00, 0x05, 0x39, 0x7f, 0x00, 0x99, 0x30, 0x9a, 0x00,
    0x97, 0x48, 0x9f, 0x00, 0x99, 0x78, 0x9f, 0x00, 0x02, 0x24, 0x79, 0x00, 0x00, 0x00, 0x12, 0x00,
    0x4b, 0x21, 0x9b, 0x00, 0x6b, 0x79, 0x9d, 0x00, 0xa3, 0x79, 0x9d, 0x00, 0xa5, 0x30, 0x9a, 0x00,
    0x37, 0x49, 0x82, 0x00, 0x08, 0x04, 0x7f, 0x00, 0xa6, 0x28, 0x9f, 0x00, 0x04, 0x3a, 0x67, 0x00,
    0x05, 0x39, 0x7f, 0x00, 0x60, 0x02, 0x3f, 0x00, 0xaf, 0x60, 0x9f, 0x00, 0x00, 0x80, 0x07, 0x00,
    0x06, 0x39, 0x7f, 0x00, 0xb2, 0x18, 0x9f, 0x00, 0x28, 0x09, 0x9f, 0x00, 0xa7, 0x50, 0x9f, 0x00,
    0x08, 0x04, 0x7f, 0x00, 0x80, 0x1c, 0x7f, 0x00, 0x71, 0x79, 0x9d, 0x00, 0x28, 0x08, 0x72, 0x00,
    0x91, 0x71, 0x9f, 0x00, 0x02, 0x1c, 0x7f, 0x00, 0x93, 0x59, 0x9f, 0x00, 0xd1, 0x78, 0x83, 0x00,
    0x4a, 0x21, 0x9f, 0x00, 0xd0, 0x10, 0x9f, 0x00, 0x43, 0x79, 0x9f, 0x00, 0x6b, 0x79, 0x9d, 0x00,
    0xa3, 0x79, 0x9d, 0x00, 0xd9, 0x30, 0x9a, 0x00, 0x37, 0x49, 0x9f, 0x00, 0x71, 0x79, 0x9d, 0x00,
    0x21, 0x08, 0x7f, 0x00, 0x7b, 0x01, 0x9f, 0x00, 0x20, 0x08, 0x7f, 0x00, 0xe9, 0x38, 0x9a, 0x00,
    0xf2, 0x38, 0x9f, 0x00, 0xf3, 0x08, 0x9b, 0x00, 0x97, 0x79, 0x9f, 0x00, 0x00, 0x00, 0x1f, 0x00,
    0x06, 0x39, 0x7f, 0x00, 0xea, 0x18, 0x9f, 0x00, 0x71, 0x79, 0x9d, 0x00, 0x00, 0x00, 0x12, 0x00,
    0x00, 0x00, 0x09, 0x00, 0xfd, 0x10, 0x9a, 0x00, 0x00, 0x00, 0x05, 0x00, 0x6b, 0x79, 0x9d, 0x00,
    0xa9, 0x79, 0x9d, 0x00, 0xf8, 0x13, 0x3f, 0x00, 0x00, 0x00, 0x12, 0x00, 0x67, 0x19, 0x9c, 0x00,
    0x61, 0x21, 0x9c, 0x00, 0x14, 0x39, 0x9f, 0x00, 0x11, 0x41, 0x9f, 0x00, 0x17, 0x21, 0x9b, 0x00,
    0x21, 0x08, 0x7f, 0x00, 0x1d, 0x01, 0x9f, 0x00, 0x20, 0x1c, 0x7f, 0x00, 0x15, 0x19, 0x9b, 0x00,
    0x17, 0x79, 0x9f, 0x00, 0x40, 0x1c, 0x7f, 0x00, 0x25, 0x11, 0x9a, 0x00, 0x05, 0x20, 0x7f, 0x00,
    0x00, 0x40, 0x1f, 0x00, 0x01, 0x60, 0x7f, 0x00, 0x17, 0x09, 0x9b, 0x00, 0x01, 0x5c, 0x7f, 0x00,
    0x15, 0x79, 0x9f, 0x00, 0x04, 0x20, 0x7f, 0x00, 0x1b, 0x11, 0x9a, 0x00, 0x10, 0x24, 0x79, 0x00,
    0x15, 0x11, 0x9b, 0x00, 0x15, 0x19, 0x9b, 0x00, 0x17, 0x79, 0x9f, 0x00, 0x82, 0x38, 0x7f, 0x00,
    0x08, 0x04, 0x7f, 0x00, 0xe1, 0x79, 0x9f, 0x00, 0xd9, 0x38, 0x9a, 0x00, 0x5d, 0x21, 0x9b, 0x00,
    0x5d, 0x29, 0x9a, 0x00, 0x02, 0x24, 0x79, 0x00, 0x03, 0x20, 0x7f, 0x00, 0x81, 0x79, 0x9f, 0x00,
    0x21, 0x08, 0x7f, 0x00, 0x4b, 0x01, 0x9f, 0x00, 0x20, 0x08, 0x7f, 0x00, 0x42, 0x11, 0x9f, 0x00,
    0x6b, 0x79, 0x9d, 0x00, 0xa3, 0x79, 0x9d, 0x00, 0x21, 0x08, 0x72, 0x00, 0xd9, 0x00, 0x9f, 0x00,
    0x28, 0x08, 0x72, 0x00, 0x59, 0x31, 0x9a, 0x00, 0x37, 0x49, 0x9f, 0x00, 0x81, 0x21, 0x9b, 0x00,
    0xa5, 0x78, 0x9f, 0x00, 0x02, 0x24, 0x7f, 0x00, 0x3f, 0x79, 0x9f, 0x00, 0x03, 0x39, 0x7f, 0x00,
    0x63, 0x61, 0x9f, 0x00, 0x01, 0x00, 0x1f, 0x00, 0x00, 0x1e, 0x7f, 0x00, 0x01, 0x00, 0x1f, 0x00,
    0x6b, 0x61, 0x9f, 0x00, 0x00, 0x00, 0x07, 0x00, 0x01, 0x00, 0x1f, 0x00, 0x70, 0x29, 0x9f, 0x00,
    0x04, 0x3a, 0x67, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x05, 0x39, 0x7f, 0x00, 0x01, 0x00, 0x1f, 0x00,
    0x01, 0x60, 0x7f, 0x00, 0x29, 0x08, 0x7f, 0x00, 0xf7, 0x78, 0x89, 0x00, 0x29, 0x08, 0x7f, 0x00,
    0x71, 0x79, 0x9d, 0x00, 0xf7, 0x78, 0x89, 0x00, 0x8b, 0x31, 0x9f, 0x00, 0x43, 0x79, 0x9f, 0x00,
    0x42, 0x21, 0x9f, 0x00, 0xd0, 0x10, 0x9f, 0x00, 0x43, 0x79, 0x9f, 0x00, 0x00, 0x08, 0x1f, 0x00,
    0x87, 0x79, 0x9f, 0x00, 0x95, 0x01, 0x9f, 0x00, 0x05, 0x5c, 0x7f, 0x00, 0x20, 0x1c, 0x7f, 0x00,
    0xb4, 0x3c, 0x72, 0x00, 0xe0, 0x10, 0x3f, 0x00, 0x24, 0x09, 0x3f, 0x00, 0x47, 0x78, 0x9f, 0x00,
    0x06, 0x39, 0x7f, 0x00, 0xa2, 0x19, 0x9f, 0x00, 0x01, 0x00, 0x1f, 0x00, 0x06, 0x39, 0x7f, 0x00,
    0xa8, 0x19, 0x9f, 0x00, 0x28, 0x09, 0x9f, 0x00, 0x01, 0x00, 0x1f, 0x00, 0x05, 0x5c, 0x7f, 0x00,
    0x2f, 0x78, 0x9f, 0x00, 0x00, 0x00, 0x09, 0x00, 0x71, 0x79, 0x9d, 0x00, 0x00, 0x00, 0x12, 0x00,
    0x6b, 0x79, 0x9d, 0x00, 0xa3, 0x79, 0x9d, 0x00, 0xc5, 0x49, 0x89, 0x00, 0xc5, 0x31, 0x9a, 0x00,
    0xef, 0x49, 0x9f, 0x00, 0x82, 0x38, 0x7f, 0x00, 0x08, 0x04, 0x7f, 0x00, 0x71, 0x79, 0x9d, 0x00,
    0x6b, 0x79, 0x9d, 0x00, 0xa3, 0x79, 0x9d, 0x00, 0xd3, 0x31, 0x9a, 0x00, 0x37, 0x49, 0x9f, 0x00,
    0xd9, 0x59, 0x9f, 0x00, 0x03, 0x20, 0x7f, 0x00, 0xe9, 0x78, 0x9f, 0x00, 0xd8, 0x29, 0x9f, 0x00,
    0x04, 0x3a, 0x67, 0x00, 0x05, 0x39, 0x7f, 0x00, 0x40, 0x02, 0x3f, 0x00, 0xe1, 0x61, 0x9f, 0x00,
    0x00, 0x80, 0x07, 0x00, 0x06, 0x39, 0x7f, 0x00, 0xe4, 0x19, 0x9f, 0x00, 0xd9, 0x51, 0x88, 0x00,
    0x08, 0x04, 0x7f, 0x00, 0x71, 0x79, 0x9d, 0x00, 0x02, 0x1c, 0x7f, 0x00, 0x23, 0x08, 0x7f, 0x00,
    0xfb, 0x01, 0x9f, 0x00, 0x28, 0x08, 0x7f, 0x00, 0x0d, 0x5a, 0x9f, 0x00, 0x01, 0x3a, 0x9a, 0x00,
    0x28, 0x08, 0x7f, 0x00, 0x00, 0x00, 0x12, 0x00, 0x05, 0xfa, 0x9f, 0x00, 0x00, 0x00, 0x05, 0x00,
    0xd9, 0xf8, 0x9f, 0x00, 0x6b, 0x79, 0x9d, 0x00, 0xa9, 0x79, 0x9d, 0x00, 0x02, 0x20, 0x7f, 0x00,
    0xd9, 0x48, 0x82, 0x00, 0x23, 0x08, 0x65, 0x00, 0x0e, 0x0a, 0x9f, 0x00, 0x00, 0x08, 0x1f, 0x00,
    0x28, 0x08, 0x7f, 0x00, 0x6b, 0x79, 0x9d, 0x00, 0x06, 0x39, 0x7f, 0x00, 0x16, 0x1a, 0x9f, 0x00,
    0xd9, 0x79, 0x9f, 0x00, 0x21, 0x08, 0x7f, 0x00, 0x22, 0x1a, 0x9f, 0x00, 0x2d, 0x7a, 0x9f, 0x00,
    0xec, 0x23, 0x25, 0x00, 0x6b, 0x79, 0x9d, 0x00, 0x26, 0x1a, 0x9f, 0x00, 0xe8, 0x33, 0x3f, 0x00,
    0xf8, 0x03, 0x32, 0x00, 0x00, 0x18, 0x1f, 0x00, 0x33, 0x4a, 0x9f, 0x00, 0x3b, 0x7a, 0x9d, 0x00,
    0x00, 0x10, 0x1f, 0x00, 0x05, 0x20, 0x7f, 0x00, 0x3f, 0x02, 0x9f, 0x00, 0x00, 0x40, 0x1f, 0x00,
    0x08, 0x24, 0x79, 0x00, 0x01, 0x00, 0x1f, 0x00, 0x01, 0x60, 0x7f, 0x00, 0x00, 0x40, 0x1f, 0x00,
    0x20, 0x08, 0x7f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x0a, 0x04, 0x7f, 0x00, 0x5a, 0x1a, 0x9f, 0x00,
    0x6b, 0x79, 0x9d, 0x00, 0x00, 0x00, 0x09, 0x00, 0x4e, 0x1a, 0x9f, 0x00, 0x06, 0x39, 0x7f, 0x00,
    0x71, 0x79, 0x9d, 0x00, 0x00, 0x80, 0x02, 0x00, 0x4b, 0x52, 0x9f, 0x00, 0x00, 0x40, 0x1f, 0x00,
    0x09, 0x04, 0x7f, 0x00, 0x4b, 0x7a, 0x9f, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x80, 0x03, 0x00, 0x80, 0x40, 0x08, 0x00,
    0x43, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x80, 0x80, 0x00, 0x20, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0xfc, 0x01, 0x00, 0x22, 0x62, 0x03, 0x00,
    0xfc, 0x1f, 0xc0, 0x09, 0x20, 0x01, 0x10, 0x00, 0x00, 0x00, 0xc0, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xfe, 0xf8, 0x07, 0x00, 0xfe, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x07, 0x00, 0xfe, 0xfe, 0x0f, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x40,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71
};

#if defined(CFG_QUP0_SE0_I2C_EN) || \
    defined(CFG_QUP0_SE1_I2C_EN) || \
    defined(CFG_QUP0_SE2_I2C_EN) || \
    defined(CFG_QUP0_SE3_I2C_EN) || \
    defined(CFG_QUP0_SE4_I2C_EN) || \
    defined(CFG_QUP0_SE5_I2C_EN)
#define QUP0_COMMON_BASE		UL(0x009C0000)

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_COMMON_BASE,
			 QUP_I2C_COMMON_REG_SIZE);

static const char *common_clocks_qup0[] = {
	"gcc_qupv3_wrap0_core_2x_clk",
	"gcc_qupv3_wrap0_core_clk",
	"gcc_qupv3_wrap_0_s_ahb_clk",
	"gcc_qupv3_wrap_0_m_ahb_clk",
	NULL,
};
#endif

#if defined(CFG_QUP1_SE0_I2C_EN) || \
    defined(CFG_QUP1_SE1_I2C_EN) || \
    defined(CFG_QUP1_SE2_I2C_EN) || \
    defined(CFG_QUP1_SE3_I2C_EN) || \
    defined(CFG_QUP1_SE4_I2C_EN) || \
    defined(CFG_QUP1_SE5_I2C_EN) || \
    defined(CFG_QUP1_SE6_I2C_EN)
#define QUP1_COMMON_BASE		UL(0x00AC0000)

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_COMMON_BASE,
			 QUP_I2C_COMMON_REG_SIZE);

static const char *common_clocks_qup1[] = {
	"gcc_qupv3_wrap1_core_2x_clk",
	"gcc_qupv3_wrap1_core_clk",
	"gcc_qupv3_wrap_1_s_ahb_clk",
	"gcc_qupv3_wrap_1_m_ahb_clk",
	NULL,
};
#endif

#if defined(CFG_QUP2_SE0_I2C_EN) || \
    defined(CFG_QUP2_SE1_I2C_EN) || \
    defined(CFG_QUP2_SE2_I2C_EN) || \
    defined(CFG_QUP2_SE3_I2C_EN) || \
    defined(CFG_QUP2_SE4_I2C_EN) || \
    defined(CFG_QUP2_SE5_I2C_EN) || \
    defined(CFG_QUP2_SE6_I2C_EN)
#define QUP2_COMMON_BASE		UL(0x008C0000)

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_COMMON_BASE,
			 QUP_I2C_COMMON_REG_SIZE);

static const char *common_clocks_qup2[] = {
	"gcc_qupv3_wrap2_core_2x_clk",
	"gcc_qupv3_wrap2_core_clk",
	"gcc_qupv3_wrap_2_s_ahb_clk",
	"gcc_qupv3_wrap_2_m_ahb_clk",
	NULL,
};
#endif

#ifdef CFG_QUP0_SE0_I2C_EN
#define QUP0_SE0_I2C_ID			1
#define QUP0_SE0_I2C_BASE		UL(0x00980000)
#define QUP0_SE0_I2C_IRQ		582

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE0_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP0_SE0_I2C_SDA_PIN		20
#define QUP0_SE0_I2C_SCL_PIN		21

static const unsigned int qup0_se0_i2c_g0_pins[] = {
	QUP0_SE0_I2C_SDA_PIN,
	QUP0_SE0_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup0_se0_i2c_pin_groups[] = {
	{
		.pins        = qup0_se0_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se0_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE1_I2C_EN
#define QUP0_SE1_I2C_ID			2
#define QUP0_SE1_I2C_BASE		UL(0x00984000)
#define QUP0_SE1_I2C_IRQ		583

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE1_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP0_SE1_I2C_SDA_PIN		24
#define QUP0_SE1_I2C_SCL_PIN		25

static const unsigned int qup0_se1_i2c_g0_pins[] = {
	QUP0_SE1_I2C_SDA_PIN,
	QUP0_SE1_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup0_se1_i2c_pin_groups[] = {
	{
		.pins        = qup0_se1_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se1_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE2_I2C_EN
#define QUP0_SE2_I2C_ID			3
#define QUP0_SE2_I2C_BASE		UL(0x00988000)
#define QUP0_SE2_I2C_IRQ		561

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE2_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP0_SE2_I2C_SDA_PIN		36
#define QUP0_SE2_I2C_SCL_PIN		37

static const unsigned int qup0_se2_i2c_g0_pins[] = {
	QUP0_SE2_I2C_SDA_PIN,
	QUP0_SE2_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup0_se2_i2c_pin_groups[] = {
	{
		.pins        = qup0_se2_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se2_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE3_I2C_EN
#define QUP0_SE3_I2C_ID			4
#define QUP0_SE3_I2C_BASE		UL(0x0098C000)
#define QUP0_SE3_I2C_IRQ		562

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE3_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP0_SE3_I2C_SDA_PIN		28
#define QUP0_SE3_I2C_SCL_PIN		29

static const unsigned int qup0_se3_i2c_g0_pins[] = {
	QUP0_SE3_I2C_SDA_PIN,
	QUP0_SE3_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup0_se3_i2c_pin_groups[] = {
	{
		.pins        = qup0_se3_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se3_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE4_I2C_EN
#define QUP0_SE4_I2C_ID			5
#define QUP0_SE4_I2C_BASE		UL(0x00990000)
#define QUP0_SE4_I2C_IRQ		563

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE4_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP0_SE4_I2C_SDA_PIN		32
#define QUP0_SE4_I2C_SCL_PIN		33

static const unsigned int qup0_se4_i2c_g0_pins[] = {
	QUP0_SE4_I2C_SDA_PIN,
	QUP0_SE4_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup0_se4_i2c_pin_groups[] = {
	{
		.pins        = qup0_se4_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se4_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE5_I2C_EN
#define QUP0_SE5_I2C_ID			6
#define QUP0_SE5_I2C_BASE		UL(0x00994000)
#define QUP0_SE5_I2C_IRQ		567

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE5_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP0_SE5_I2C_SDA_PIN		36
#define QUP0_SE5_I2C_SCL_PIN		37

static const unsigned int qup0_se5_i2c_g0_pins[] = {
	QUP0_SE5_I2C_SDA_PIN,
	QUP0_SE5_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup0_se5_i2c_pin_groups[] = {
	{
		.pins        = qup0_se5_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se5_i2c_g0_pins),
		.func        = 2,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE0_I2C_EN
#define QUP1_SE0_I2C_ID			7
#define QUP1_SE0_I2C_BASE		UL(0x00A80000)
#define QUP1_SE0_I2C_IRQ		385

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE0_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP1_SE0_I2C_SDA_PIN		40
#define QUP1_SE0_I2C_SCL_PIN		41

static const unsigned int qup1_se0_i2c_g0_pins[] = {
	QUP1_SE0_I2C_SDA_PIN,
	QUP1_SE0_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup1_se0_i2c_pin_groups[] = {
	{
		.pins        = qup1_se0_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se0_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE1_I2C_EN
#define QUP1_SE1_I2C_ID			8
#define QUP1_SE1_I2C_BASE		UL(0x00A84000)
#define QUP1_SE1_I2C_IRQ		386

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE1_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP1_SE1_I2C_SDA_PIN		42
#define QUP1_SE1_I2C_SCL_PIN		43

static const unsigned int qup1_se1_i2c_g0_pins[] = {
	QUP1_SE1_I2C_SDA_PIN,
	QUP1_SE1_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup1_se1_i2c_pin_groups[] = {
	{
		.pins        = qup1_se1_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se1_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE2_I2C_EN
#define QUP1_SE2_I2C_ID			9
#define QUP1_SE2_I2C_BASE		UL(0x00A88000)
#define QUP1_SE2_I2C_IRQ		387

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE2_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP1_SE2_I2C_SDA_PIN		46
#define QUP1_SE2_I2C_SCL_PIN		47

static const unsigned int qup1_se2_i2c_g0_pins[] = {
	QUP1_SE2_I2C_SDA_PIN,
	QUP1_SE2_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup1_se2_i2c_pin_groups[] = {
	{
		.pins        = qup1_se2_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se2_i2c_g0_pins),
		.func        = 2,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE3_I2C_EN
#define QUP1_SE3_I2C_ID			10
#define QUP1_SE3_I2C_BASE		UL(0x00A8C000)
#define QUP1_SE3_I2C_IRQ		388

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE3_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP1_SE3_I2C_SDA_PIN		44
#define QUP1_SE3_I2C_SCL_PIN		45

static const unsigned int qup1_se3_i2c_g0_pins[] = {
	QUP1_SE3_I2C_SDA_PIN,
	QUP1_SE3_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup1_se3_i2c_pin_groups[] = {
	{
		.pins        = qup1_se3_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se3_i2c_g0_pins),
		.func        = 2,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE4_I2C_EN
#define QUP1_SE4_I2C_ID			11
#define QUP1_SE4_I2C_BASE		UL(0x00A90000)
#define QUP1_SE4_I2C_IRQ		389

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE4_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP1_SE4_I2C_SDA_PIN		48
#define QUP1_SE4_I2C_SCL_PIN		49

static const unsigned int qup1_se4_i2c_g0_pins[] = {
	QUP1_SE4_I2C_SDA_PIN,
	QUP1_SE4_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup1_se4_i2c_pin_groups[] = {
	{
		.pins        = qup1_se4_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se4_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE5_I2C_EN
#define QUP1_SE5_I2C_ID			12
#define QUP1_SE5_I2C_BASE		UL(0x00A94000)
#define QUP1_SE5_I2C_IRQ		390

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE5_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP1_SE5_I2C_SDA_PIN		52
#define QUP1_SE5_I2C_SCL_PIN		53

static const unsigned int qup1_se5_i2c_g0_pins[] = {
	QUP1_SE5_I2C_SDA_PIN,
	QUP1_SE5_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup1_se5_i2c_pin_groups[] = {
	{
		.pins        = qup1_se5_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se5_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE6_I2C_EN
#define QUP1_SE6_I2C_ID			13
#define QUP1_SE6_I2C_BASE		UL(0x00A98000)
#define QUP1_SE6_I2C_IRQ		867

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE6_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP1_SE6_I2C_SDA_PIN		56
#define QUP1_SE6_I2C_SCL_PIN		57

static const unsigned int qup1_se6_i2c_g0_pins[] = {
	QUP1_SE6_I2C_SDA_PIN,
	QUP1_SE6_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup1_se6_i2c_pin_groups[] = {
	{
		.pins        = qup1_se6_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se6_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE0_I2C_EN
#define QUP2_SE0_I2C_ID			14
#define QUP2_SE0_I2C_BASE		UL(0x00880000)
#define QUP2_SE0_I2C_IRQ		405

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE0_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP2_SE0_I2C_SDA_PIN		80
#define QUP2_SE0_I2C_SCL_PIN		81

static const unsigned int qup2_se0_i2c_g0_pins[] = {
	QUP2_SE0_I2C_SDA_PIN,
	QUP2_SE0_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup2_se0_i2c_pin_groups[] = {
	{
		.pins        = qup2_se0_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se0_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE1_I2C_EN
#define QUP2_SE1_I2C_ID			15
#define QUP2_SE1_I2C_BASE		UL(0x00884000)
#define QUP2_SE1_I2C_IRQ		615

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE1_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP2_SE1_I2C_SDA_PIN		84
#define QUP2_SE1_I2C_SCL_PIN		85

static const unsigned int qup2_se1_i2c_g0_pins[] = {
	QUP2_SE1_I2C_SDA_PIN,
	QUP2_SE1_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup2_se1_i2c_pin_groups[] = {
	{
		.pins        = qup2_se1_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se1_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE2_I2C_EN
#define QUP2_SE2_I2C_ID			16
#define QUP2_SE2_I2C_BASE		UL(0x00888000)
#define QUP2_SE2_I2C_IRQ		616

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE2_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP2_SE2_I2C_SDA_PIN		86
#define QUP2_SE2_I2C_SCL_PIN		87

static const unsigned int qup2_se2_i2c_g0_pins[] = {
	QUP2_SE2_I2C_SDA_PIN,
	QUP2_SE2_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup2_se2_i2c_pin_groups[] = {
	{
		.pins        = qup2_se2_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se2_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE3_I2C_EN
#define QUP2_SE3_I2C_ID			17
#define QUP2_SE3_I2C_BASE		UL(0x0088C000)
#define QUP2_SE3_I2C_IRQ		617

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE3_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP2_SE3_I2C_SDA_PIN		91
#define QUP2_SE3_I2C_SCL_PIN		92

static const unsigned int qup2_se3_i2c_g0_pins[] = {
	QUP2_SE3_I2C_SDA_PIN,
	QUP2_SE3_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup2_se3_i2c_pin_groups[] = {
	{
		.pins        = qup2_se3_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se3_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE4_I2C_EN
#define QUP2_SE4_I2C_ID			18
#define QUP2_SE4_I2C_BASE		UL(0x00890000)
#define QUP2_SE4_I2C_IRQ		618

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE4_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP2_SE4_I2C_SDA_PIN		95
#define QUP2_SE4_I2C_SCL_PIN		96

static const unsigned int qup2_se4_i2c_g0_pins[] = {
	QUP2_SE4_I2C_SDA_PIN,
	QUP2_SE4_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup2_se4_i2c_pin_groups[] = {
	{
		.pins        = qup2_se4_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se4_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE5_I2C_EN
#define QUP2_SE5_I2C_ID			19
#define QUP2_SE5_I2C_BASE		UL(0x00894000)
#define QUP2_SE5_I2C_IRQ		619

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE5_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP2_SE5_I2C_SDA_PIN		99
#define QUP2_SE5_I2C_SCL_PIN		100

static const unsigned int qup2_se5_i2c_g0_pins[] = {
	QUP2_SE5_I2C_SDA_PIN,
	QUP2_SE5_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup2_se5_i2c_pin_groups[] = {
	{
		.pins        = qup2_se5_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se5_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE6_I2C_EN
#define QUP2_SE6_I2C_ID			20
#define QUP2_SE6_I2C_BASE		UL(0x00898000)
#define QUP2_SE6_I2C_IRQ		865

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE6_I2C_BASE, QUP_I2C_REG_SIZE);

#define QUP2_SE6_I2C_SDA_PIN		97
#define QUP2_SE6_I2C_SCL_PIN		98

static const unsigned int qup2_se6_i2c_g0_pins[] = {
	QUP2_SE6_I2C_SDA_PIN,
	QUP2_SE6_I2C_SCL_PIN,
};

static const struct tlmm_pin_group qup2_se6_i2c_pin_groups[] = {
	{
		.pins        = qup2_se6_i2c_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se6_i2c_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_UP,
		.drive_ma    = 2,
		.strong_pull = false,
	},
};
#endif

const struct qup_i2c_platform_cfg qup_i2c_config[] = {
#ifdef CFG_QUP0_SE0_I2C_EN
	{
		.id                 = QUP0_SE0_I2C_ID,
		.base               = QUP0_SE0_I2C_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE0_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s0_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se0_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se0_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE1_I2C_EN
	{
		.id                 = QUP0_SE1_I2C_ID,
		.base               = QUP0_SE1_I2C_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE1_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s1_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se1_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se1_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE2_I2C_EN
	{
		.id                 = QUP0_SE2_I2C_ID,
		.base               = QUP0_SE2_I2C_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE2_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s2_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se2_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se2_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE3_I2C_EN
	{
		.id                 = QUP0_SE3_I2C_ID,
		.base               = QUP0_SE3_I2C_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE3_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s3_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se3_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se3_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE4_I2C_EN
	{
		.id                 = QUP0_SE4_I2C_ID,
		.base               = QUP0_SE4_I2C_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE4_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s4_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se4_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se4_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE5_I2C_EN
	{
		.id                 = QUP0_SE5_I2C_ID,
		.base               = QUP0_SE5_I2C_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE5_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s5_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se5_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se5_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE0_I2C_EN
	{
		.id                 = QUP1_SE0_I2C_ID,
		.base               = QUP1_SE0_I2C_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE0_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s0_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se0_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se0_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE1_I2C_EN
	{
		.id                 = QUP1_SE1_I2C_ID,
		.base               = QUP1_SE1_I2C_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE1_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s1_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se1_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se1_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE2_I2C_EN
	{
		.id                 = QUP1_SE2_I2C_ID,
		.base               = QUP1_SE2_I2C_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE2_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s2_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se2_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se2_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE3_I2C_EN
	{
		.id                 = QUP1_SE3_I2C_ID,
		.base               = QUP1_SE3_I2C_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE3_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s3_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se3_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se3_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE4_I2C_EN
	{
		.id                 = QUP1_SE4_I2C_ID,
		.base               = QUP1_SE4_I2C_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE4_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s4_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se4_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se4_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE5_I2C_EN
	{
		.id                 = QUP1_SE5_I2C_ID,
		.base               = QUP1_SE5_I2C_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE5_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s5_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se5_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se5_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE6_I2C_EN
	{
		.id                 = QUP1_SE6_I2C_ID,
		.base               = QUP1_SE6_I2C_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE6_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s6_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se6_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se6_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE0_I2C_EN
	{
		.id                 = QUP2_SE0_I2C_ID,
		.base               = QUP2_SE0_I2C_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE0_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s0_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se0_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se0_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE1_I2C_EN
	{
		.id                 = QUP2_SE1_I2C_ID,
		.base               = QUP2_SE1_I2C_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE1_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s1_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se1_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se1_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE2_I2C_EN
	{
		.id                 = QUP2_SE2_I2C_ID,
		.base               = QUP2_SE2_I2C_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE2_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s2_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se2_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se2_i2c_pin_groups),
		.fw_image           = i2c_qup_fw,
		.fw_image_size      = sizeof(i2c_qup_fw),
	},
#endif
#ifdef CFG_QUP2_SE3_I2C_EN
	{
		.id                 = QUP2_SE3_I2C_ID,
		.base               = QUP2_SE3_I2C_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE3_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s3_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se3_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se3_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE4_I2C_EN
	{
		.id                 = QUP2_SE4_I2C_ID,
		.base               = QUP2_SE4_I2C_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE4_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s4_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se4_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se4_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE5_I2C_EN
	{
		.id                 = QUP2_SE5_I2C_ID,
		.base               = QUP2_SE5_I2C_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE5_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s5_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se5_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se5_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE6_I2C_EN
	{
		.id                 = QUP2_SE6_I2C_ID,
		.base               = QUP2_SE6_I2C_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE6_I2C_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s6_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se6_i2c_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se6_i2c_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
};

const size_t qup_i2c_config_count = ARRAY_SIZE(qup_i2c_config);
