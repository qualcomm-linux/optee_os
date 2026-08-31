/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Tier 1 host unit-test mock of core/include/kernel/mutex.h.
 *
 * Locking is irrelevant for single-threaded host unit tests; struct mutex
 * only needs to exist as a storage member of driver_state/query_db, and
 * mutex_lock()/mutex_unlock() only need to be safe no-ops.
 */
#ifndef __KERNEL_MUTEX_H
#define __KERNEL_MUTEX_H

struct mutex {
	int dummy;
};

#define MUTEX_INITIALIZER { .dummy = 0 }

static inline void mutex_init(struct mutex *m __attribute__((unused))) {}
static inline void mutex_destroy(struct mutex *m __attribute__((unused))) {}
static inline void mutex_lock(struct mutex *m __attribute__((unused))) {}
static inline void mutex_unlock(struct mutex *m __attribute__((unused))) {}

#endif /* __KERNEL_MUTEX_H */
