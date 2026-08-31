/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/arm.h.
 *
 * rpmh_client.c includes <arm.h> but calls none of its functions/macros
 * directly (confirmed by inspection); this stub exists only to satisfy the
 * #include on hosts where the real arch header isn't available.
 */
#ifndef ARM_H
#define ARM_H
#endif /* ARM_H */
