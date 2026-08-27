/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/qcom/tlmm/tlmm.h>
#include <drivers/qcom_geni_spi.h>
#include <mm/core_mmu.h>
#include <util.h>

/*
 * Per-QUPv3 wrapper resources. The wrapper-common register window and the
 * wrapper gating clocks are shared by every SE in that wrapper, so they are
 * defined and mapped once here rather than per SE.
 */

const uint8_t spi_qup_fw[] =
{
    0x53, 0x45, 0x46, 0x57, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x30, 0x01, 0x00, 0x02, 0x0b,
    0x09, 0x00, 0xba, 0x01, 0x1c, 0x00, 0x45, 0x00, 0x18, 0x08, 0x04, 0x07, 0xce, 0x07, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x11, 0x00,
    0x20, 0xc0, 0x34, 0x00, 0x00, 0xc8, 0x3d, 0x00, 0x10, 0x40, 0x62, 0x00, 0x00, 0xe8, 0x67, 0x00,
    0x30, 0xc0, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0xa8, 0x00, 0x30, 0x40, 0xd7, 0x00, 0x00, 0xe8, 0x11, 0x00,
    0x10, 0xc0, 0x33, 0x00, 0x00, 0x88, 0x8a, 0x00, 0x08, 0x40, 0x00, 0x00, 0x00, 0x08, 0x8b, 0x00,
    0x08, 0x40, 0x00, 0x00, 0x00, 0x88, 0x8c, 0x00, 0x08, 0x40, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6b, 0x0b, 0x61, 0x00, 0x40, 0x11, 0x7f, 0x00,
    0x81, 0x38, 0x7f, 0x00, 0x83, 0x7a, 0xff, 0x00, 0x5f, 0x6f, 0x7f, 0x00, 0x07, 0x78, 0x7f, 0x00,
    0x53, 0x79, 0x7f, 0x00, 0x50, 0x78, 0x7f, 0x00, 0x9b, 0x30, 0x9f, 0x00, 0x58, 0x10, 0x9f, 0x00,
    0x61, 0x48, 0x86, 0x00, 0x04, 0x3a, 0x7f, 0x00, 0x90, 0x68, 0x9f, 0x00, 0x6f, 0x6f, 0x7f, 0x00,
    0x69, 0x48, 0x9f, 0x00, 0x1f, 0x78, 0x7f, 0x00, 0x83, 0x78, 0x9f, 0x00, 0x48, 0x78, 0x7f, 0x00,
    0x0b, 0x78, 0x7f, 0x00, 0x77, 0x78, 0x9f, 0x00, 0x6f, 0x60, 0x9f, 0x00, 0x04, 0x3a, 0x64, 0x00,
    0x72, 0x28, 0x3f, 0x01, 0x75, 0x60, 0xcc, 0x00, 0x28, 0xb8, 0x67, 0x00, 0x00, 0xba, 0x67, 0x00,
    0x7b, 0xe8, 0x87, 0x00, 0x6e, 0xb0, 0x25, 0x01, 0x9b, 0x78, 0x9f, 0x00, 0x80, 0x28, 0xc6, 0x00,
    0x28, 0xb8, 0x67, 0x00, 0x00, 0xb9, 0x67, 0x00, 0x87, 0xe0, 0x27, 0x01, 0x80, 0xb0, 0x85, 0x00,
    0x9b, 0x78, 0x9f, 0x00, 0x8c, 0x28, 0x9f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x28, 0xb8, 0x67, 0x00,
    0x00, 0xba, 0x67, 0x00, 0x95, 0xe8, 0x87, 0x00, 0x91, 0xa8, 0xc5, 0x00, 0x8c, 0x30, 0x9f, 0x00,
    0xbb, 0x38, 0x3b, 0x01, 0x0b, 0x0b, 0x63, 0x00, 0x01, 0x44, 0x7f, 0x00, 0x05, 0x39, 0x7f, 0x00,
    0xa3, 0x60, 0x9f, 0x00, 0xb8, 0x28, 0x83, 0x00, 0xaa, 0x28, 0x83, 0x00, 0x00, 0x1e, 0x7f, 0x00,
    0xb0, 0x28, 0x83, 0x00, 0x03, 0x39, 0x7f, 0x00, 0xaf, 0x60, 0x9f, 0x00, 0x20, 0x1c, 0x7f, 0x00,
    0xb6, 0x30, 0x9b, 0x00, 0x40, 0x1c, 0x7f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0xa7, 0x78, 0x84, 0x00,
    0xc2, 0x48, 0x9f, 0x00, 0xc2, 0x30, 0x9a, 0x00, 0xbf, 0x60, 0x9f, 0x00, 0x04, 0x3a, 0x64, 0x00,
    0x01, 0x44, 0x7f, 0x00, 0x20, 0x1c, 0x7f, 0x00, 0x08, 0x20, 0x7f, 0x00, 0xc9, 0x68, 0x9f, 0x00,
    0xca, 0x08, 0x9f, 0x00, 0xff, 0x78, 0x9f, 0x00, 0xfb, 0x0b, 0x7f, 0x00, 0x89, 0x11, 0x9f, 0x00,
    0xef, 0x50, 0x9f, 0x00, 0x01, 0x44, 0x7f, 0x00, 0x01, 0x60, 0x7f, 0x00, 0x05, 0x39, 0x7f, 0x00,
    0xdb, 0x60, 0x9f, 0x00, 0x4b, 0x0b, 0x72, 0x00, 0x00, 0x10, 0x7f, 0x00, 0xeb, 0x18, 0x9a, 0x00,
    0xb7, 0x08, 0x9b, 0x00, 0x01, 0x5c, 0x7f, 0x00, 0x40, 0x1c, 0x7f, 0x00, 0xb7, 0x78, 0x9f, 0x00,
    0x00, 0x1e, 0x7f, 0x00, 0xe3, 0x78, 0x9f, 0x00, 0x50, 0xf9, 0x7f, 0x00, 0xf4, 0xd0, 0x9f, 0x00,
    0xf3, 0xd0, 0x87, 0x00, 0xd5, 0x78, 0x85, 0x00, 0x00, 0x00, 0x01, 0x00, 0x83, 0x01, 0x9a, 0x00,
    0x7f, 0x09, 0x9a, 0x00, 0x05, 0x39, 0x7f, 0x00, 0x05, 0x5c, 0x61, 0x00, 0x82, 0x38, 0x7f, 0x00,
    0x09, 0x49, 0x9f, 0x00, 0x04, 0x3a, 0x7f, 0x00, 0x28, 0x69, 0x9f, 0x00, 0x08, 0x78, 0x7f, 0x00,
    0x56, 0x79, 0x7f, 0x00, 0xef, 0x6e, 0x7f, 0x00, 0x20, 0x38, 0x7f, 0x00, 0x01, 0x2c, 0x7f, 0x00,
    0x05, 0x2c, 0x7f, 0x00, 0x05, 0x2c, 0x7f, 0x00, 0x00, 0x04, 0x7f, 0x00, 0x01, 0x40, 0x7f, 0x00,
    0x50, 0x78, 0x7f, 0x00, 0x02, 0x1c, 0x7f, 0x00, 0x1f, 0x61, 0x9f, 0x00, 0x25, 0x49, 0x9f, 0x00,
    0x45, 0x79, 0x9f, 0x00, 0x48, 0x78, 0x7f, 0x00, 0x39, 0x79, 0x9f, 0x00, 0x52, 0x79, 0x7f, 0x00,
    0xbf, 0x6e, 0x7f, 0x00, 0x2d, 0x61, 0x9f, 0x00, 0x57, 0x79, 0x9f, 0x00, 0x31, 0x61, 0x9f, 0x00,
    0x04, 0x3a, 0x64, 0x00, 0x34, 0x29, 0x3f, 0x01, 0x37, 0x61, 0x8b, 0x00, 0x28, 0x38, 0x68, 0x00,
    0x00, 0xba, 0x68, 0x00, 0x3d, 0xe9, 0x88, 0x00, 0x31, 0xd9, 0x28, 0x01, 0x5f, 0x79, 0x9f, 0x00,
    0x42, 0x29, 0x9f, 0x00, 0x28, 0x38, 0x68, 0x00, 0x00, 0xb9, 0x68, 0x00, 0x49, 0xe1, 0x28, 0x01,
    0x43, 0xd9, 0x88, 0x00, 0x5f, 0x79, 0x9f, 0x00, 0x57, 0x29, 0x88, 0x00, 0x01, 0x40, 0x7f, 0x00,
    0x52, 0x29, 0x9f, 0x00, 0x01, 0x44, 0x7f, 0x00, 0x20, 0x38, 0x68, 0x00, 0x59, 0xd1, 0x88, 0x00,
    0x4f, 0x59, 0x88, 0x00, 0x5f, 0x79, 0x88, 0x00, 0x0b, 0x0b, 0x7f, 0x00, 0x01, 0x44, 0x7f, 0x00,
    0x05, 0x39, 0x7f, 0x00, 0x65, 0x61, 0x9f, 0x00, 0x7c, 0x29, 0x83, 0x00, 0x6c, 0x29, 0x83, 0x00,
    0x00, 0x1e, 0x7f, 0x00, 0x72, 0x29, 0x83, 0x00, 0x03, 0x39, 0x7f, 0x00, 0x71, 0x61, 0x9f, 0x00,
    0x76, 0x31, 0x9b, 0x00, 0x20, 0x1c, 0x7f, 0x00, 0x09, 0x20, 0x7f, 0x00, 0x40, 0x1c, 0x7f, 0x00,
    0x00, 0x40, 0x1f, 0x00, 0x69, 0x79, 0x84, 0x00, 0x00, 0x1e, 0x7f, 0x00, 0xfd, 0x78, 0x9f, 0x00,
    0x03, 0x39, 0x7f, 0x00, 0x85, 0x61, 0x9f, 0x00, 0xfb, 0x78, 0x9f, 0x00, 0x89, 0xd1, 0xdf, 0x00,
    0x8f, 0x11, 0x9b, 0x00, 0x01, 0x5c, 0x7f, 0x00, 0x01, 0x60, 0x7f, 0x00, 0x05, 0x39, 0x7f, 0x00,
    0x01, 0x44, 0x7f, 0x00, 0x95, 0x61, 0x9f, 0x00, 0x9b, 0x19, 0x9a, 0x00, 0x73, 0x79, 0x92, 0x00,
    0x00, 0x1e, 0x72, 0x00, 0x73, 0x79, 0x9f, 0x00, 0x5f, 0x6e, 0x61, 0x00, 0x6b, 0x0b, 0x7f, 0x00,
    0x40, 0x11, 0x7f, 0x00, 0x05, 0x5c, 0x7f, 0x00, 0x81, 0x38, 0x7f, 0x00, 0x83, 0x7a, 0xff, 0x00,
    0x53, 0x79, 0x7f, 0x00, 0x09, 0x78, 0x7f, 0x00, 0x50, 0x78, 0x7f, 0x00, 0xb7, 0x49, 0x86, 0x00,
    0x04, 0x3a, 0x7f, 0x00, 0xe6, 0x69, 0x9f, 0x00, 0x6f, 0x6e, 0x7f, 0x00, 0xbf, 0x49, 0x9f, 0x00,
    0x1f, 0x78, 0x7f, 0x00, 0xd9, 0x79, 0x9f, 0x00, 0x48, 0x78, 0x7f, 0x00, 0x0b, 0x78, 0x7f, 0x00,
    0xcd, 0x79, 0x9f, 0x00, 0xc5, 0x61, 0x9f, 0x00, 0x04, 0x3a, 0x64, 0x00, 0xc8, 0x29, 0x3f, 0x01,
    0xcb, 0x61, 0xcc, 0x00, 0x28, 0xb8, 0x69, 0x00, 0x00, 0xba, 0x69, 0x00, 0xd1, 0xe9, 0x89, 0x00,
    0xc4, 0xb1, 0x28, 0x01, 0xf1, 0x79, 0x9f, 0x00, 0xd6, 0x29, 0xc6, 0x00, 0x28, 0xb8, 0x69, 0x00,
    0x00, 0xb9, 0x69, 0x00, 0xdd, 0xe1, 0x29, 0x01, 0xd6, 0xb1, 0x88, 0x00, 0xf1, 0x79, 0x9f, 0x00,
    0xe2, 0x29, 0x9f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x28, 0xb8, 0x69, 0x00, 0x00, 0xba, 0x69, 0x00,
    0xeb, 0xe9, 0x89, 0x00, 0xe7, 0xa9, 0xc8, 0x00, 0xe2, 0x31, 0x9f, 0x00, 0x0b, 0x0b, 0x63, 0x00,
    0x01, 0x44, 0x7f, 0x00, 0x05, 0x39, 0x7f, 0x00, 0xf7, 0x61, 0x9f, 0x00, 0x0c, 0x2a, 0x83, 0x00,
    0x00, 0x00, 0x1f, 0x00, 0x00, 0x2a, 0x83, 0x00, 0x00, 0x1e, 0x7f, 0x00, 0x06, 0x2a, 0x83, 0x00,
    0x03, 0x39, 0x7f, 0x00, 0x05, 0x62, 0x9f, 0x00, 0x20, 0x1c, 0x7f, 0x00, 0x40, 0x1c, 0x7f, 0x00,
    0x00, 0x40, 0x1f, 0x00, 0xfd, 0x79, 0x84, 0x00, 0x23, 0x52, 0x9f, 0x00, 0x01, 0x44, 0x7f, 0x00,
    0x01, 0x60, 0x7f, 0x00, 0x05, 0x39, 0x7f, 0x00, 0x17, 0x62, 0x9f, 0x00, 0x08, 0x5c, 0x72, 0x00,
    0x1e, 0x1a, 0x9a, 0x00, 0x00, 0x1e, 0x7f, 0x00, 0x40, 0x1c, 0x7f, 0x00, 0x00, 0x40, 0x1f, 0x00,
    0x50, 0xf9, 0x7f, 0x00, 0x28, 0xd2, 0x9f, 0x00, 0x27, 0xd2, 0x89, 0x00, 0x11, 0x7a, 0x88, 0x00,
    0x00, 0x40, 0x0b, 0x00, 0x30, 0x02, 0x9f, 0x00, 0x01, 0x60, 0x7f, 0x00, 0x00, 0x40, 0x04, 0x00,
    0x38, 0x3a, 0x9a, 0x00, 0x01, 0x40, 0x6f, 0x00, 0x37, 0x7a, 0x8f, 0x00, 0x20, 0x38, 0x6f, 0x00,
    0x00, 0x00, 0x0f, 0x00, 0x3b, 0xd2, 0x8f, 0x00, 0x00, 0x20, 0x0f, 0x00, 0x00, 0x40, 0x1f, 0x00,
    0x43, 0x62, 0x9f, 0x00, 0x7f, 0x6d, 0x7f, 0x00, 0x6b, 0x09, 0x7f, 0x00, 0xc4, 0x78, 0x7f, 0x00,
    0x07, 0x78, 0x7f, 0x00, 0x04, 0x3a, 0x7f, 0x00, 0x56, 0x6a, 0x9f, 0x00, 0x01, 0x20, 0x7f, 0x00,
    0x57, 0x7a, 0x9f, 0x00, 0x61, 0x2a, 0xc5, 0x00, 0xd5, 0x00, 0x9f, 0x00, 0x56, 0x12, 0x9f, 0x00,
    0x04, 0x3a, 0x7f, 0x00, 0x5d, 0x6a, 0x9f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0xb9, 0x67, 0x00,
    0x82, 0x1c, 0x67, 0x00, 0x65, 0x62, 0x87, 0x00, 0x55, 0x52, 0x87, 0x00, 0x00, 0x00, 0x05, 0x00,
    0xd5, 0x00, 0x9f, 0x00, 0x00, 0x10, 0x1f, 0x00, 0x79, 0x4a, 0x9f, 0x00, 0x04, 0x3a, 0x7f, 0x00,
    0x73, 0x6a, 0x9f, 0x00, 0x00, 0x30, 0x05, 0x00, 0x6b, 0x7a, 0x85, 0x00, 0x6b, 0x0b, 0x7f, 0x00,
    0x20, 0x38, 0x7f, 0x00, 0xcc, 0x78, 0x7f, 0x00, 0x9b, 0x38, 0x9b, 0x00, 0x95, 0x7a, 0x9f, 0x00,
    0x88, 0x02, 0x9a, 0x00, 0x03, 0x39, 0x7f, 0x00, 0x87, 0x62, 0x9f, 0x00, 0x8c, 0x0a, 0x9a, 0x00,
    0x00, 0x1e, 0x7f, 0x00, 0x05, 0x39, 0x7f, 0x00, 0x93, 0x3a, 0x9b, 0x00, 0x94, 0x0a, 0x9b, 0x00,
    0x43, 0x2a, 0x9a, 0x00, 0x01, 0x2c, 0x7f, 0x00, 0x05, 0x2c, 0x7f, 0x00, 0x05, 0x2c, 0x7f, 0x00,
    0x00, 0x04, 0x7f, 0x00, 0x01, 0x40, 0x7f, 0x00, 0x9f, 0x62, 0x9f, 0x00, 0x01, 0x00, 0x1f, 0x00,
    0x6b, 0x0b, 0x61, 0x00, 0x40, 0x11, 0x7f, 0x00, 0x05, 0x5c, 0x7f, 0x00, 0x82, 0x38, 0x7f, 0x00,
    0x83, 0x7a, 0xff, 0x00, 0x01, 0x44, 0x7f, 0x00, 0x53, 0x79, 0x7f, 0x00, 0x5f, 0x6e, 0x7f, 0x00,
    0x09, 0x78, 0x7f, 0x00, 0x50, 0x78, 0x7f, 0x00, 0xbd, 0x4a, 0x86, 0x00, 0x04, 0x3a, 0x7f, 0x00,
    0x24, 0x6b, 0x9f, 0x00, 0x6f, 0x6e, 0x7f, 0x00, 0xc5, 0x4a, 0x9f, 0x00, 0x1f, 0x78, 0x7f, 0x00,
    0xfb, 0x7a, 0x9f, 0x00, 0x48, 0x78, 0x7f, 0x00, 0x0b, 0x78, 0x7f, 0x00, 0xd5, 0x7a, 0x9f, 0x00,
    0xcb, 0x62, 0x9f, 0x00, 0x04, 0x3a, 0x64, 0x00, 0x5d, 0x03, 0x9f, 0x00, 0xce, 0x2a, 0x3f, 0x01,
    0xd3, 0x62, 0xcc, 0x00, 0x20, 0x38, 0x69, 0x00, 0xd7, 0xd2, 0x89, 0x00, 0xea, 0x5a, 0x89, 0x00,
    0xca, 0x32, 0x28, 0x01, 0xef, 0x6e, 0x7f, 0x00, 0x08, 0x20, 0x7f, 0x00, 0x08, 0x78, 0x7f, 0x00,
    0x02, 0x1c, 0x7f, 0x00, 0x56, 0x79, 0x7f, 0x00, 0x01, 0x40, 0x7f, 0x00, 0x31, 0x79, 0x83, 0x00,
    0x6f, 0x6f, 0x68, 0x00, 0x0b, 0x78, 0x7f, 0x00, 0x04, 0x20, 0x7f, 0x00, 0x01, 0x40, 0x7f, 0x00,
    0x6e, 0x30, 0x3f, 0x01, 0x43, 0x7b, 0x9f, 0x00, 0x5d, 0x03, 0x9f, 0x00, 0xf6, 0x2a, 0xc6, 0x00,
    0x20, 0x38, 0x69, 0x00, 0xfd, 0xd2, 0x29, 0x01, 0x12, 0x5b, 0x89, 0x00, 0xf8, 0x32, 0x88, 0x00,
    0xef, 0x6e, 0x7f, 0x00, 0x08, 0x20, 0x7f, 0x00, 0x08, 0x78, 0x7f, 0x00, 0x02, 0x1c, 0x7f, 0x00,
    0x56, 0x79, 0x7f, 0x00, 0x01, 0x40, 0x7f, 0x00, 0x45, 0x29, 0x83, 0x00, 0x43, 0x79, 0x9f, 0x00,
    0x6f, 0x6f, 0x68, 0x00, 0x1f, 0x78, 0x7f, 0x00, 0x04, 0x20, 0x7f, 0x00, 0x01, 0x40, 0x7f, 0x00,
    0x80, 0x30, 0x9f, 0x00, 0x43, 0x7b, 0x9f, 0x00, 0x5d, 0x03, 0x9f, 0x00, 0x1e, 0x2b, 0x9f, 0x00,
    0x00, 0x00, 0x06, 0x00, 0x20, 0x38, 0x69, 0x00, 0x27, 0xd3, 0x89, 0x00, 0x36, 0x5b, 0x89, 0x00,
    0x25, 0x2b, 0xc8, 0x00, 0x1e, 0x33, 0x9f, 0x00, 0x52, 0x79, 0x7f, 0x00, 0xbf, 0x6e, 0x63, 0x00,
    0x57, 0x29, 0x9f, 0x00, 0x51, 0x79, 0x9f, 0x00, 0x5f, 0x6f, 0x68, 0x00, 0x07, 0x78, 0x7f, 0x00,
    0x04, 0x20, 0x7f, 0x00, 0x01, 0x40, 0x7f, 0x00, 0x8f, 0x28, 0x9f, 0x00, 0x8c, 0x30, 0x9f, 0x00,
    0x0b, 0x0b, 0x7f, 0x00, 0x05, 0x39, 0x7f, 0x00, 0x47, 0x63, 0x9f, 0x00, 0x4d, 0x13, 0x9a, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x50, 0x1b, 0x9a, 0x00, 0x00, 0x1e, 0x7f, 0x00, 0x56, 0x23, 0x9a, 0x00,
    0x03, 0x39, 0x7f, 0x00, 0x55, 0x63, 0x9f, 0x00, 0x20, 0x1c, 0x7f, 0x00, 0x40, 0x1c, 0x7f, 0x00,
    0x00, 0x40, 0x1f, 0x00, 0x62, 0x53, 0x9f, 0x00, 0x89, 0x91, 0x98, 0x00, 0xef, 0xb0, 0x98, 0x00,
    0x01, 0x44, 0x7f, 0x00, 0x01, 0x60, 0x7f, 0x00, 0x05, 0x39, 0x7f, 0x00, 0x69, 0x63, 0x9f, 0x00,
    0x08, 0x5c, 0x72, 0x00, 0x70, 0x1b, 0x9a, 0x00, 0x00, 0x1e, 0x7f, 0x00, 0x40, 0x1c, 0x7f, 0x00,
    0x00, 0x40, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x78, 0x0e, 0x24, 0x00, 0x88, 0x10, 0x01, 0x00, 0x07, 0x00, 0x24, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x15, 0x00,
    0xa0, 0x83, 0x14, 0x00, 0x28, 0x81, 0xaa, 0x00, 0x02, 0x10, 0x64, 0x00, 0x00, 0x40, 0x00, 0x00,
    0x01, 0x02, 0x00, 0x00, 0x05, 0xfe, 0x01, 0x00, 0xe7, 0xc2, 0x02, 0x00, 0x00, 0x5c, 0x43, 0x0a,
    0x1a, 0x01, 0x10, 0x00, 0x00, 0x00, 0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xac, 0x0c, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe4, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x07, 0xf8, 0x07, 0x00, 0xfe, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x07, 0x00, 0xfe, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3f, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x40, 0x41, 0x42, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54,
    0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71
};

#if defined(CFG_QUP0_SE0_SPI_EN) || \
    defined(CFG_QUP0_SE1_SPI_EN) || \
    defined(CFG_QUP0_SE2_SPI_EN) || \
    defined(CFG_QUP0_SE3_SPI_EN) || \
    defined(CFG_QUP0_SE4_SPI_EN) || \
    defined(CFG_QUP0_SE5_SPI_EN)
#define QUP0_COMMON_BASE		UL(0x009C0000)

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_COMMON_BASE,
			 QUP_SPI_COMMON_REG_SIZE);

static const char *common_clocks_qup0[] = {
	"gcc_qupv3_wrap0_core_2x_clk",
	"gcc_qupv3_wrap0_core_clk",
	"gcc_qupv3_wrap_0_s_ahb_clk",
	"gcc_qupv3_wrap_0_m_ahb_clk",
	NULL,
};
#endif

#if defined(CFG_QUP1_SE0_SPI_EN) || \
    defined(CFG_QUP1_SE1_SPI_EN) || \
    defined(CFG_QUP1_SE2_SPI_EN) || \
    defined(CFG_QUP1_SE3_SPI_EN) || \
    defined(CFG_QUP1_SE4_SPI_EN) || \
    defined(CFG_QUP1_SE5_SPI_EN)
#define QUP1_COMMON_BASE		UL(0x00AC0000)

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_COMMON_BASE,
			 QUP_SPI_COMMON_REG_SIZE);

static const char *common_clocks_qup1[] = {
	"gcc_qupv3_wrap1_core_2x_clk",
	"gcc_qupv3_wrap1_core_clk",
	"gcc_qupv3_wrap_1_s_ahb_clk",
	"gcc_qupv3_wrap_1_m_ahb_clk",
	NULL,
};
#endif

#if defined(CFG_QUP2_SE0_SPI_EN) || \
    defined(CFG_QUP2_SE1_SPI_EN) || \
    defined(CFG_QUP2_SE2_SPI_EN) || \
    defined(CFG_QUP2_SE3_SPI_EN) || \
    defined(CFG_QUP2_SE4_SPI_EN) || \
    defined(CFG_QUP2_SE5_SPI_EN) || \
    defined(CFG_QUP2_SE6_SPI_EN)
#define QUP2_COMMON_BASE		UL(0x008C0000)

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_COMMON_BASE,
			 QUP_SPI_COMMON_REG_SIZE);

static const char *common_clocks_qup2[] = {
	"gcc_qupv3_wrap2_core_2x_clk",
	"gcc_qupv3_wrap2_core_clk",
	"gcc_qupv3_wrap_2_s_ahb_clk",
	"gcc_qupv3_wrap_2_m_ahb_clk",
	NULL,
};
#endif

#ifdef CFG_QUP0_SE0_SPI_EN
#define QUP0_SE0_SPI_ID			1
#define QUP0_SE0_SPI_BASE		UL(0x00980000)
#define QUP0_SE0_SPI_IRQ		582

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE0_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP0_SE0_SPI_MISO_PIN		20
#define QUP0_SE0_SPI_MOSI_PIN		21
#define QUP0_SE0_SPI_CLK_PIN		22
#define QUP0_SE0_SPI_CS_PIN		23

static const unsigned int qup0_se0_spi_g0_pins[] = {
	QUP0_SE0_SPI_MISO_PIN,
	QUP0_SE0_SPI_MOSI_PIN,
	QUP0_SE0_SPI_CS_PIN,
};
static const unsigned int qup0_se0_spi_g1_pins[] = {
	QUP0_SE0_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup0_se0_spi_pin_groups[] = {
	{
		.pins        = qup0_se0_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se0_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup0_se0_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup0_se0_spi_g1_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE1_SPI_EN
#define QUP0_SE1_SPI_ID			2
#define QUP0_SE1_SPI_BASE		UL(0x00984000)
#define QUP0_SE1_SPI_IRQ		583

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE1_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP0_SE1_SPI_MISO_PIN		24
#define QUP0_SE1_SPI_MOSI_PIN		25
#define QUP0_SE1_SPI_CLK_PIN		26
#define QUP0_SE1_SPI_CS_PIN		27

static const unsigned int qup0_se1_spi_g0_pins[] = {
	QUP0_SE1_SPI_MISO_PIN,
	QUP0_SE1_SPI_MOSI_PIN,
	QUP0_SE1_SPI_CS_PIN,
};
static const unsigned int qup0_se1_spi_g1_pins[] = {
	QUP0_SE1_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup0_se1_spi_pin_groups[] = {
	{
		.pins        = qup0_se1_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se1_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup0_se1_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup0_se1_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE2_SPI_EN
#define QUP0_SE2_SPI_ID			3
#define QUP0_SE2_SPI_BASE		UL(0x00988000)
#define QUP0_SE2_SPI_IRQ		561

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE2_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP0_SE2_SPI_MISO_PIN		36
#define QUP0_SE2_SPI_MOSI_PIN		37
#define QUP0_SE2_SPI_CLK_PIN		38
#define QUP0_SE2_SPI_CS_PIN		39

static const unsigned int qup0_se2_spi_g0_pins[] = {
	QUP0_SE2_SPI_MISO_PIN,
	QUP0_SE2_SPI_MOSI_PIN,
};
static const unsigned int qup0_se2_spi_g1_pins[] = {
	QUP0_SE2_SPI_CS_PIN,
};
static const unsigned int qup0_se2_spi_g2_pins[] = {
	QUP0_SE2_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup0_se2_spi_pin_groups[] = {
	{
		.pins        = qup0_se2_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se2_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup0_se2_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup0_se2_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup0_se2_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup0_se2_spi_g2_pins),
		.func        = 2,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE3_SPI_EN
#define QUP0_SE3_SPI_ID			4
#define QUP0_SE3_SPI_BASE		UL(0x0098C000)
#define QUP0_SE3_SPI_IRQ		562

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE3_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP0_SE3_SPI_MISO_PIN		28
#define QUP0_SE3_SPI_MOSI_PIN		29
#define QUP0_SE3_SPI_CLK_PIN		30
#define QUP0_SE3_SPI_CS_PIN		31

static const unsigned int qup0_se3_spi_g0_pins[] = {
	QUP0_SE3_SPI_MISO_PIN,
	QUP0_SE3_SPI_MOSI_PIN,
	QUP0_SE3_SPI_CS_PIN,
};
static const unsigned int qup0_se3_spi_g1_pins[] = {
	QUP0_SE3_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup0_se3_spi_pin_groups[] = {
	{
		.pins        = qup0_se3_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se3_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup0_se3_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup0_se3_spi_g1_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE4_SPI_EN
#define QUP0_SE4_SPI_ID			5
#define QUP0_SE4_SPI_BASE		UL(0x00990000)
#define QUP0_SE4_SPI_IRQ		563

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE4_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP0_SE4_SPI_MISO_PIN		32
#define QUP0_SE4_SPI_MOSI_PIN		33
#define QUP0_SE4_SPI_CLK_PIN		34
#define QUP0_SE4_SPI_CS_PIN		35

static const unsigned int qup0_se4_spi_g0_pins[] = {
	QUP0_SE4_SPI_MISO_PIN,
	QUP0_SE4_SPI_MOSI_PIN,
	QUP0_SE4_SPI_CS_PIN,
};
static const unsigned int qup0_se4_spi_g1_pins[] = {
	QUP0_SE4_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup0_se4_spi_pin_groups[] = {
	{
		.pins        = qup0_se4_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se4_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup0_se4_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup0_se4_spi_g1_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP0_SE5_SPI_EN
#define QUP0_SE5_SPI_ID			6
#define QUP0_SE5_SPI_BASE		UL(0x00994000)
#define QUP0_SE5_SPI_IRQ		567

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP0_SE5_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP0_SE5_SPI_MISO_PIN		36
#define QUP0_SE5_SPI_MOSI_PIN		37
#define QUP0_SE5_SPI_CLK_PIN		38
#define QUP0_SE5_SPI_CS_PIN		39

static const unsigned int qup0_se5_spi_g0_pins[] = {
	QUP0_SE5_SPI_CS_PIN,
};
static const unsigned int qup0_se5_spi_g1_pins[] = {
	QUP0_SE5_SPI_MISO_PIN,
	QUP0_SE5_SPI_MOSI_PIN,
};
static const unsigned int qup0_se5_spi_g2_pins[] = {
	QUP0_SE5_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup0_se5_spi_pin_groups[] = {
	{
		.pins        = qup0_se5_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup0_se5_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup0_se5_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup0_se5_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup0_se5_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup0_se5_spi_g2_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE0_SPI_EN
#define QUP1_SE0_SPI_ID			7
#define QUP1_SE0_SPI_BASE		UL(0x00A80000)
#define QUP1_SE0_SPI_IRQ		385

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE0_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP1_SE0_SPI_MISO_PIN		40
#define QUP1_SE0_SPI_MOSI_PIN		41
#define QUP1_SE0_SPI_CLK_PIN		42
#define QUP1_SE0_SPI_CS_PIN		43

static const unsigned int qup1_se0_spi_g0_pins[] = {
	QUP1_SE0_SPI_MISO_PIN,
	QUP1_SE0_SPI_MOSI_PIN,
};
static const unsigned int qup1_se0_spi_g1_pins[] = {
	QUP1_SE0_SPI_CS_PIN,
};
static const unsigned int qup1_se0_spi_g2_pins[] = {
	QUP1_SE0_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup1_se0_spi_pin_groups[] = {
	{
		.pins        = qup1_se0_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se0_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se0_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup1_se0_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se0_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup1_se0_spi_g2_pins),
		.func        = 2,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE1_SPI_EN
#define QUP1_SE1_SPI_ID			8
#define QUP1_SE1_SPI_BASE		UL(0x00A84000)
#define QUP1_SE1_SPI_IRQ		386

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE1_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP1_SE1_SPI_MISO_PIN		42
#define QUP1_SE1_SPI_MOSI_PIN		43
#define QUP1_SE1_SPI_CLK_PIN		40
#define QUP1_SE1_SPI_CS_PIN		41

static const unsigned int qup1_se1_spi_g0_pins[] = {
	QUP1_SE1_SPI_MISO_PIN,
	QUP1_SE1_SPI_MOSI_PIN,
};
static const unsigned int qup1_se1_spi_g1_pins[] = {
	QUP1_SE1_SPI_CS_PIN,
};
static const unsigned int qup1_se1_spi_g2_pins[] = {
	QUP1_SE1_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup1_se1_spi_pin_groups[] = {
	{
		.pins        = qup1_se1_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se1_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se1_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup1_se1_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se1_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup1_se1_spi_g2_pins),
		.func        = 2,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE2_SPI_EN
#define QUP1_SE2_SPI_ID			9
#define QUP1_SE2_SPI_BASE		UL(0x00A88000)
#define QUP1_SE2_SPI_IRQ		387

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE2_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP1_SE2_SPI_MISO_PIN		46
#define QUP1_SE2_SPI_MOSI_PIN		47
#define QUP1_SE2_SPI_CLK_PIN		44
#define QUP1_SE2_SPI_CS_PIN		45

static const unsigned int qup1_se2_spi_g0_pins[] = {
	QUP1_SE2_SPI_CS_PIN,
};
static const unsigned int qup1_se2_spi_g1_pins[] = {
	QUP1_SE2_SPI_MISO_PIN,
	QUP1_SE2_SPI_MOSI_PIN,
};
static const unsigned int qup1_se2_spi_g2_pins[] = {
	QUP1_SE2_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup1_se2_spi_pin_groups[] = {
	{
		.pins        = qup1_se2_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se2_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se2_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup1_se2_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se2_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup1_se2_spi_g2_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE3_SPI_EN
#define QUP1_SE3_SPI_ID			10
#define QUP1_SE3_SPI_BASE		UL(0x00A8C000)
#define QUP1_SE3_SPI_IRQ		388

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE3_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP1_SE3_SPI_MISO_PIN		44
#define QUP1_SE3_SPI_MOSI_PIN		45
#define QUP1_SE3_SPI_CLK_PIN		46
#define QUP1_SE3_SPI_CS_PIN		47

static const unsigned int qup1_se3_spi_g0_pins[] = {
	QUP1_SE3_SPI_CS_PIN,
};
static const unsigned int qup1_se3_spi_g1_pins[] = {
	QUP1_SE3_SPI_MISO_PIN,
	QUP1_SE3_SPI_MOSI_PIN,
};
static const unsigned int qup1_se3_spi_g2_pins[] = {
	QUP1_SE3_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup1_se3_spi_pin_groups[] = {
	{
		.pins        = qup1_se3_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se3_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se3_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup1_se3_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se3_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup1_se3_spi_g2_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE4_SPI_EN
#define QUP1_SE4_SPI_ID			11
#define QUP1_SE4_SPI_BASE		UL(0x00A90000)
#define QUP1_SE4_SPI_IRQ		389

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE4_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP1_SE4_SPI_MISO_PIN		48
#define QUP1_SE4_SPI_MOSI_PIN		49
#define QUP1_SE4_SPI_CLK_PIN		50
#define QUP1_SE4_SPI_CS_PIN		51

static const unsigned int qup1_se4_spi_g0_pins[] = {
	QUP1_SE4_SPI_MISO_PIN,
	QUP1_SE4_SPI_MOSI_PIN,
	QUP1_SE4_SPI_CS_PIN,
};
static const unsigned int qup1_se4_spi_g1_pins[] = {
	QUP1_SE4_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup1_se4_spi_pin_groups[] = {
	{
		.pins        = qup1_se4_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se4_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se4_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup1_se4_spi_g1_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP1_SE5_SPI_EN
#define QUP1_SE5_SPI_ID			12
#define QUP1_SE5_SPI_BASE		UL(0x00A94000)
#define QUP1_SE5_SPI_IRQ		390

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP1_SE5_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP1_SE5_SPI_MISO_PIN		52
#define QUP1_SE5_SPI_MOSI_PIN		53
#define QUP1_SE5_SPI_CLK_PIN		54
#define QUP1_SE5_SPI_CS_PIN		55

static const unsigned int qup1_se5_spi_g0_pins[] = {
	QUP1_SE5_SPI_MISO_PIN,
	QUP1_SE5_SPI_MOSI_PIN,
	QUP1_SE5_SPI_CS_PIN,
};
static const unsigned int qup1_se5_spi_g1_pins[] = {
	QUP1_SE5_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup1_se5_spi_pin_groups[] = {
	{
		.pins        = qup1_se5_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup1_se5_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup1_se5_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup1_se5_spi_g1_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE0_SPI_EN
#define QUP2_SE0_SPI_ID			13
#define QUP2_SE0_SPI_BASE		UL(0x00880000)
#define QUP2_SE0_SPI_IRQ		405

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE0_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP2_SE0_SPI_MISO_PIN		80
#define QUP2_SE0_SPI_MOSI_PIN		81
#define QUP2_SE0_SPI_CLK_PIN		82
#define QUP2_SE0_SPI_CS_PIN		83

static const unsigned int qup2_se0_spi_g0_pins[] = {
	QUP2_SE0_SPI_MISO_PIN,
	QUP2_SE0_SPI_MOSI_PIN,
	QUP2_SE0_SPI_CS_PIN,
};
static const unsigned int qup2_se0_spi_g1_pins[] = {
	QUP2_SE0_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup2_se0_spi_pin_groups[] = {
	{
		.pins        = qup2_se0_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se0_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se0_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup2_se0_spi_g1_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE1_SPI_EN
#define QUP2_SE1_SPI_ID			14
#define QUP2_SE1_SPI_BASE		UL(0x00884000)
#define QUP2_SE1_SPI_IRQ		615

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE1_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP2_SE1_SPI_MISO_PIN		84
#define QUP2_SE1_SPI_MOSI_PIN		85
#define QUP2_SE1_SPI_CLK_PIN		99
#define QUP2_SE1_SPI_CS_PIN		100

static const unsigned int qup2_se1_spi_g0_pins[] = {
	QUP2_SE1_SPI_MISO_PIN,
	QUP2_SE1_SPI_MOSI_PIN,
};
static const unsigned int qup2_se1_spi_g1_pins[] = {
	QUP2_SE1_SPI_CS_PIN,
};
static const unsigned int qup2_se1_spi_g2_pins[] = {
	QUP2_SE1_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup2_se1_spi_pin_groups[] = {
	{
		.pins        = qup2_se1_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se1_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se1_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup2_se1_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se1_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup2_se1_spi_g2_pins),
		.func        = 2,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE2_SPI_EN
#define QUP2_SE2_SPI_ID			15
#define QUP2_SE2_SPI_BASE		UL(0x00888000)
#define QUP2_SE2_SPI_IRQ		616

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE2_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP2_SE2_SPI_MISO_PIN		86
#define QUP2_SE2_SPI_MOSI_PIN		87
#define QUP2_SE2_SPI_CLK_PIN		88
#define QUP2_SE2_SPI_CS_PIN		89

static const unsigned int qup2_se2_spi_g0_pins[] = {
	QUP2_SE2_SPI_MISO_PIN,
	QUP2_SE2_SPI_MOSI_PIN,
	QUP2_SE2_SPI_CS_PIN,
};
static const unsigned int qup2_se2_spi_g1_pins[] = {
	QUP2_SE2_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup2_se2_spi_pin_groups[] = {
	{
		.pins        = qup2_se2_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se2_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se2_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup2_se2_spi_g1_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE3_SPI_EN
#define QUP2_SE3_SPI_ID			16
#define QUP2_SE3_SPI_BASE		UL(0x0088C000)
#define QUP2_SE3_SPI_IRQ		617

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE3_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP2_SE3_SPI_MISO_PIN		91
#define QUP2_SE3_SPI_MOSI_PIN		92
#define QUP2_SE3_SPI_CLK_PIN		93
#define QUP2_SE3_SPI_CS_PIN		94

static const unsigned int qup2_se3_spi_g0_pins[] = {
	QUP2_SE3_SPI_MISO_PIN,
	QUP2_SE3_SPI_MOSI_PIN,
	QUP2_SE3_SPI_CS_PIN,
};
static const unsigned int qup2_se3_spi_g1_pins[] = {
	QUP2_SE3_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup2_se3_spi_pin_groups[] = {
	{
		.pins        = qup2_se3_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se3_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se3_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup2_se3_spi_g1_pins),
		.func        = 1,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE4_SPI_EN
#define QUP2_SE4_SPI_ID			17
#define QUP2_SE4_SPI_BASE		UL(0x00890000)
#define QUP2_SE4_SPI_IRQ		618

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE4_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP2_SE4_SPI_MISO_PIN		95
#define QUP2_SE4_SPI_MOSI_PIN		96
#define QUP2_SE4_SPI_CLK_PIN		97
#define QUP2_SE4_SPI_CS_PIN		98

static const unsigned int qup2_se4_spi_g0_pins[] = {
	QUP2_SE4_SPI_MISO_PIN,
	QUP2_SE4_SPI_MOSI_PIN,
};
static const unsigned int qup2_se4_spi_g1_pins[] = {
	QUP2_SE4_SPI_CS_PIN,
};
static const unsigned int qup2_se4_spi_g2_pins[] = {
	QUP2_SE4_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup2_se4_spi_pin_groups[] = {
	{
		.pins        = qup2_se4_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se4_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se4_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup2_se4_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se4_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup2_se4_spi_g2_pins),
		.func        = 2,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE5_SPI_EN
#define QUP2_SE5_SPI_ID			18
#define QUP2_SE5_SPI_BASE		UL(0x00894000)
#define QUP2_SE5_SPI_IRQ		619

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE5_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP2_SE5_SPI_MISO_PIN		99
#define QUP2_SE5_SPI_MOSI_PIN		100
#define QUP2_SE5_SPI_CLK_PIN		84
#define QUP2_SE5_SPI_CS_PIN		85

static const unsigned int qup2_se5_spi_g0_pins[] = {
	QUP2_SE5_SPI_MISO_PIN,
	QUP2_SE5_SPI_MOSI_PIN,
};
static const unsigned int qup2_se5_spi_g1_pins[] = {
	QUP2_SE5_SPI_CS_PIN,
};
static const unsigned int qup2_se5_spi_g2_pins[] = {
	QUP2_SE5_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup2_se5_spi_pin_groups[] = {
	{
		.pins        = qup2_se5_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se5_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se5_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup2_se5_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se5_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup2_se5_spi_g2_pins),
		.func        = 2,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

#ifdef CFG_QUP2_SE6_SPI_EN
#define QUP2_SE6_SPI_ID			19
#define QUP2_SE6_SPI_BASE		UL(0x00898000)
#define QUP2_SE6_SPI_IRQ		865

register_phys_mem_pgdir(MEM_AREA_IO_SEC, QUP2_SE6_SPI_BASE, QUP_SPI_REG_SIZE);

#define QUP2_SE6_SPI_MISO_PIN		97
#define QUP2_SE6_SPI_MOSI_PIN		98
#define QUP2_SE6_SPI_CLK_PIN		95
#define QUP2_SE6_SPI_CS_PIN		96

static const unsigned int qup2_se6_spi_g0_pins[] = {
	QUP2_SE6_SPI_MISO_PIN,
	QUP2_SE6_SPI_MOSI_PIN,
};
static const unsigned int qup2_se6_spi_g1_pins[] = {
	QUP2_SE6_SPI_CS_PIN,
};
static const unsigned int qup2_se6_spi_g2_pins[] = {
	QUP2_SE6_SPI_CLK_PIN,
};

static const struct tlmm_pin_group qup2_se6_spi_pin_groups[] = {
	{
		.pins        = qup2_se6_spi_g0_pins,
		.pin_count   = ARRAY_SIZE(qup2_se6_spi_g0_pins),
		.func        = 1,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se6_spi_g1_pins,
		.pin_count   = ARRAY_SIZE(qup2_se6_spi_g1_pins),
		.func        = 2,
		.pull        = TLMM_PULL_DOWN,
		.drive_ma    = 6,
		.strong_pull = false,
	},
	{
		.pins        = qup2_se6_spi_g2_pins,
		.pin_count   = ARRAY_SIZE(qup2_se6_spi_g2_pins),
		.func        = 2,
		.pull        = TLMM_PULL_NONE,
		.drive_ma    = 6,
		.strong_pull = false,
	},
};
#endif

const struct qup_spi_platform_cfg qup_spi_config[] = {
#ifdef CFG_QUP0_SE0_SPI_EN
	{
		.id                 = QUP0_SE0_SPI_ID,
		.base               = QUP0_SE0_SPI_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE0_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s0_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se0_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se0_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE1_SPI_EN
	{
		.id                 = QUP0_SE1_SPI_ID,
		.base               = QUP0_SE1_SPI_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE1_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s1_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se1_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se1_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE2_SPI_EN
	{
		.id                 = QUP0_SE2_SPI_ID,
		.base               = QUP0_SE2_SPI_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE2_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s2_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se2_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se2_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE3_SPI_EN
	{
		.id                 = QUP0_SE3_SPI_ID,
		.base               = QUP0_SE3_SPI_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE3_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s3_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se3_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se3_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE4_SPI_EN
	{
		.id                 = QUP0_SE4_SPI_ID,
		.base               = QUP0_SE4_SPI_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE4_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s4_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se4_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se4_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP0_SE5_SPI_EN
	{
		.id                 = QUP0_SE5_SPI_ID,
		.base               = QUP0_SE5_SPI_BASE,
		.common_base        = QUP0_COMMON_BASE,
		.itr_num            = QUP0_SE5_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap0_s5_clk",
		.common_clocks_name = common_clocks_qup0,
		.pin_groups         = qup0_se5_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup0_se5_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE0_SPI_EN
	{
		.id                 = QUP1_SE0_SPI_ID,
		.base               = QUP1_SE0_SPI_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE0_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s0_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se0_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se0_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE1_SPI_EN
	{
		.id                 = QUP1_SE1_SPI_ID,
		.base               = QUP1_SE1_SPI_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE1_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s1_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se1_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se1_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE2_SPI_EN
	{
		.id                 = QUP1_SE2_SPI_ID,
		.base               = QUP1_SE2_SPI_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE2_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s2_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se2_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se2_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE3_SPI_EN
	{
		.id                 = QUP1_SE3_SPI_ID,
		.base               = QUP1_SE3_SPI_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE3_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s3_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se3_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se3_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE4_SPI_EN
	{
		.id                 = QUP1_SE4_SPI_ID,
		.base               = QUP1_SE4_SPI_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE4_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s4_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se4_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se4_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP1_SE5_SPI_EN
	{
		.id                 = QUP1_SE5_SPI_ID,
		.base               = QUP1_SE5_SPI_BASE,
		.common_base        = QUP1_COMMON_BASE,
		.itr_num            = QUP1_SE5_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap1_s5_clk",
		.common_clocks_name = common_clocks_qup1,
		.pin_groups         = qup1_se5_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup1_se5_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE0_SPI_EN
	{
		.id                 = QUP2_SE0_SPI_ID,
		.base               = QUP2_SE0_SPI_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE0_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s0_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se0_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se0_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE1_SPI_EN
	{
		.id                 = QUP2_SE1_SPI_ID,
		.base               = QUP2_SE1_SPI_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE1_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s1_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se1_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se1_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE2_SPI_EN
	{
		.id                 = QUP2_SE2_SPI_ID,
		.base               = QUP2_SE2_SPI_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE2_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s2_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se2_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se2_spi_pin_groups),
		.fw_image           = spi_qup_fw, 
		.fw_image_size      = sizeof(spi_qup_fw),
	},
#endif
#ifdef CFG_QUP2_SE3_SPI_EN
	{
		.id                 = QUP2_SE3_SPI_ID,
		.base               = QUP2_SE3_SPI_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE3_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s3_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se3_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se3_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE4_SPI_EN
	{
		.id                 = QUP2_SE4_SPI_ID,
		.base               = QUP2_SE4_SPI_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE4_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s4_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se4_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se4_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE5_SPI_EN
	{
		.id                 = QUP2_SE5_SPI_ID,
		.base               = QUP2_SE5_SPI_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE5_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s5_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se5_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se5_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
#ifdef CFG_QUP2_SE6_SPI_EN
	{
		.id                 = QUP2_SE6_SPI_ID,
		.base               = QUP2_SE6_SPI_BASE,
		.common_base        = QUP2_COMMON_BASE,
		.itr_num            = QUP2_SE6_SPI_IRQ,
		.clk_hz             = 19200000,
		.se_clock_name      = "gcc_qupv3_wrap2_s6_clk",
		.common_clocks_name = common_clocks_qup2,
		.pin_groups         = qup2_se6_spi_pin_groups,
		.pin_group_count    = ARRAY_SIZE(qup2_se6_spi_pin_groups),
		.fw_image           = NULL,
		.fw_image_size      = 0,
	},
#endif
};

const size_t qup_spi_config_count = ARRAY_SIZE(qup_spi_config);
