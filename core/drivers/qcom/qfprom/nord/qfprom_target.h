/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __QFPROM_TARGET_H__
#define __QFPROM_TARGET_H__

#include <clock_group_qcom.h>
#include <platform_config.h>
#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>
#include <util.h>

#define QFPROM_RAW_BASE                          0x360c0000
#define QFPROM_CORR_BASE                         0x360c8000
#define QFPROM_SIZE                              0x8000

/* Unverified Nord programming values. */
#define QFPROM_BLOW_TIMER_OFFSET                 0x2030
#define QFPROM_ACCEL_OFFSET                      0x2038
#define QFPROM_BLOW_STATUS_OFFSET                0x203c
#define QFPROM_BLOW_STATUS_RMSK                  0x3
#define QFPROM_BLOW_STATUS_BUSY_VAL              0x1
#define QFPROM_BLOW_STATUS_ERROR_VAL             0x2
#define QFPROM_BLOW_STATUS_READY_VAL             0x0
#define QFPROM_FEC_ESR_OFFSET                    0x2058
#define QFPROM_BLOW_TIMER_CLK_FREQ_MHZ_X10       48
#define QFPROM_FUSE_BLOW_TIME_IN_US              5
#define QFPROM_GATELAST_VAL                      0x1
#define QFPROM_TRIPPT_SEL_VAL                    0x4
#define QFPROM_ACCEL_VAL                         0x10
#define QFPROM_GATELAST_SHFT                     11
#define QFPROM_TRIPPT_SEL_SHFT                   8
#define QFPROM_ACCEL_SHFT                        0
#define QFPROM_ACCEL_VALUE \
	((QFPROM_GATELAST_VAL << QFPROM_GATELAST_SHFT) | \
	 (QFPROM_TRIPPT_SEL_VAL << QFPROM_TRIPPT_SEL_SHFT) | \
	 (QFPROM_ACCEL_VAL << QFPROM_ACCEL_SHFT))
#define QFPROM_ACCEL_RESET_VALUE \
	(0x1 << QFPROM_GATELAST_SHFT)

#define QFPROM_RAW_TO_CORR(raw_addr) \
	((raw_addr) + (QFPROM_CORR_BASE - QFPROM_RAW_BASE))

#define QFPROM_FEC_EAR_OFFSET                    0x24
#define QFPROM_FEC_ESR_ERR_SEEN_BMSK             BIT(0)
#define QFPROM_FEC_EAR_ERR_ADDR_BMSK             0xFFFF

#define QFPROM_HW_MUTEX_ID                       8
#define QFPROM_HW_MUTEX_PID                      1
#define QFPROM_HW_MUTEX_TIMEOUT_US               10000
#define QFPROM_MUTEX_REG_ADDR \
	(TCSR_MUTEX_BASE + (0x1000 * QFPROM_HW_MUTEX_ID))

enum qfprom_region_name {
	QFPROM_LAST_REGION_DUMMY,
};

/* Unverified Nord fuse-row addresses. */
#define FEC_ENABLES_ADDR                        (QFPROM_RAW_BASE + 0x1a0)
#define PK_HASH_0_ADDR                           (QFPROM_RAW_BASE + 0x2a0)

#define SECURE_BOOT_APPS_ADDR			(SECURITY_CONTROL_BASE + 0x0704)
#define SECURE_BOOT_AUTH_EN_BMSK		0x20
#define SECURE_BOOT_USE_SERIAL_NUM_BMSK		0x40
#define QFPROM_ROOT_OF_TRUST_SENSE_ADDR         (SECURITY_CONTROL_BASE + 0x608)
#define QFPROM_ROOT_OF_TRUST_SENSE_REG_COUNT    14

#define QFPROM_ROOT_OF_TRUST_BYTE_SIZE		48

/* PIL anti-rollback layout is unresolved; authentication remains disabled. */

#define OEM_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0800)
#define OEM_ID_BMSK				0x0000ffff
#define OEM_ID_SHFT				0
#define MODEL_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0804)
#define MODEL_ID_BMSK				0x0000ffff
#define MODEL_ID_SHFT				0
#define JTAG_ID_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0844)
#define JTAG_ID_AUTH_BMSK			0x0fffffff
#define JTAG_ID_AUTH_SHFT			0
#define SERIAL_NUM_SENSE_ADDR			(SECURITY_CONTROL_BASE + 0x0710)

/* Unverified Nord address; do not enable SOC_HW_VERSION authentication. */
#define TCSR_SOC_HW_VERSION_ADDR		0x01FC8000
#define SOC_HW_VERSION_FAM_DEV_BMSK		0xffff0000
#define SOC_HW_VERSION_FAM_DEV_SHFT		16

#define OEM_CONFIG2_ADDR			(SECURITY_CONTROL_BASE + 0x0308)
#define EKU_ENFORCEMENT_EN_SHFT			30

#define SEGMENT_HASH_SELECT_SUPPORTED		0


#endif /* __QFPROM_TARGET_H__ */
