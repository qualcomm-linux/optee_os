/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CVP_H_
#define _CVP_H_

#include "pas_subsys.h"

/* Offsets from CVP_BASE */
#define CVP_WRAPPER_TSW_REG_BASE	0x000c0000
#define CVP_TOP_TSW0_REG_BASE		0x000c2000

extern const struct qcom_pas_ops cvp_ops;

#endif /* _CVP_H_ */
