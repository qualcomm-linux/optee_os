/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc.
 *
 * Overview:
 *
 *   The Hardware Key Manager (HWKM) is a dedicated security block that
 *   manages cryptographic keys entirely within hardware-protected storage.
 *   Keys are identified by slot indices and never leave the hardware in
 *   plaintext unless the policy explicitly permits SW_KEY read-back.
 *   All key operations (generation, derivation, wrapping, clearing) are
 *   submitted as packed command words through a write-only FIFO, and
 *   results are read back from a corresponding response FIFO.
 *
 * Driver design:
 *
 *   A single struct hwkm_drv_ctx holds all driver state, including the
 *   pre-mapped virtual addresses for each MMIO region, the mutex, cached
 *   key material, and hardware information. It is populated once by
 *   hwkm_hal_init() during driver initialization and is subsequently
 *   retrieved through hwkm_get_context().
 *
 *   Per-operation transaction handles (hwkm_handle_t) carry only a pointer
 *   to the driver context, the destination selector, and the list of
 *   transactions (hwkm_transaction_t). Handles and their associated
 *   hwkm_transaction_t nodes represent a sequence of commands to be issued.
 */

#ifndef __HWKM_HAL_H__
#define __HWKM_HAL_H__

#include <kernel/mutex.h>
#include <kernel/tee_common_otp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <types_ext.h>

/* HAL error codes. */

#define HWKM_SUCCESS                    0
#define HWKM_ERR_GENERIC                1
#define HWKM_ERR_INVALID_ARG            2
#define HWKM_ERR_NO_MEMORY              3
#define HWKM_ERR_NOT_SUPPORTED          4
#define HWKM_ERR_INVALID_DEST           5
#define HWKM_ERR_CMD_FIFO_NOT_EMPTY     6
#define HWKM_ERR_CMD_FIFO_TIMEOUT       7
#define HWKM_ERR_RSP_FIFO_TIMEOUT       8
#define HWKM_ERR_RSP_OVERFLOW           9
#define HWKM_ERR_CLK                    10
#define HWKM_ERR_BIST                   11

#define HWKM_MAX_KEY_SIZE               32   /* bytes */
#define HWKM_MAX_CTX_SIZE               64   /* bytes */
#define HWKM_MAX_BLOB_SIZE              68   /* bytes */

typedef enum {
	KM_MASTER = 0,
	GPCE_SLAVE = 1,
	ICE_SLAVE = 3,
} hwkm_destination_t;

/**
 * enum hwkm_security_level_t - Visibility and ownership level for a key slot.
 *
 * The security level is encoded in the key policy word and enforced by
 * hardware for every command that references the slot. It is selected at
 * key-creation time (NIST_KEYGEN or SYSTEM_KDF) and cannot be changed
 * without clearing and re-deriving the slot. The levels form a strict
 * hierarchy: SW_KEY is the least restrictive, and HW_KEY is the most
 * restrictive.
 *
 * @SW_KEY: Software-readable key. The raw key material may be read back by
 *   software using a KEY_SLOT_RDWR command with is_write = false. Use this
 *   level only when key material must be passed to a non-HWKM engine (for
 *   example, a software crypto fallback). All other operations, including
 *   derivation, wrapping, and encryption, remain permitted.
 *
 * @MANAGED_KEY: Software-managed, hardware-opaque key. The key material is
 *   never returned to software in plaintext; KEY_SLOT_RDWR read-back is
 *   rejected by hardware. Software retains full control over the slot
 *   lifecycle (derive, wrap-export, swap, clear) but cannot observe the raw
 *   key bytes. This is the appropriate level for runtime application keys
 *   derived by OP-TEE into general-purpose slots.
 *
 * @HW_KEY: Hardware-owned key. The slot is inaccessible to all software,
 *   including OP-TEE. Neither read-back nor explicit clear is permitted;
 *   the hardware manages the slot contents autonomously.
 */
typedef enum {
	SW_KEY = 0,
	MANAGED_KEY = 1,
	HW_KEY = 2,
} hwkm_security_level_t;

/**
 * enum hwkm_type_t - Functional role of a key stored in the HWKM key table.
 *
 * The hardware enforces that a key is used only for operations that match
 * its declared type. The type is encoded in the key policy word and is
 * checked for every command that references the slot.
 *
 * @KEY_DERIVATION_KEY: Key Derivation Key (KDK). May be used only as the
 *   root input to a SYSTEM_KDF command. It cannot be used for encryption,
 *   decryption, wrapping, or swapping. The L1 and L2 KDKs in the master
 *   key table all carry this type.
 *
 * @KEY_WRAPPING_KEY: Key Wrapping Key (KWK). Used exclusively as the
 *   wrapping key in KEY_WRAP_EXPORT and KEY_UNWRAP_IMPORT commands. A KWK
 *   may not be used for bulk-data encryption or as a KDF root.
 *
 * @KEY_SWAPPING_KEY: Key Swapping Key (KSK). Permits the key slot to
 *   participate in atomic key-swap operations between the key manager and
 *   a slave engine.
 *
 * @TRANSPORT_KEY: Transport Protection Key (TPK or TPKEY). A session-unique
 *   key pair (even/odd slots), derived at boot by hwkm_hal_init(). It is
 *   used internally by the hardware to protect keys in transit to slave
 *   engines. Software must not use TPKEY slots for any other purpose.
 *
 * @GENERIC_KEY: General-purpose data key. May be used for any algorithm
 *   permitted by the accompanying hwkm_alg_t policy field, including
 *   encryption, decryption. This is the type assigned to application-level
 *   keys derived into general-purpose slots.
 */
typedef enum {
	KEY_DERIVATION_KEY = 0,
	KEY_WRAPPING_KEY = 1,
	KEY_SWAPPING_KEY = 2,
	TRANSPORT_KEY = 3,
	GENERIC_KEY = 4,
} hwkm_type_t;

typedef enum {
	AES128_ECB = 0,
	AES256_ECB = 1,
	AES128_CBC = 4,
	AES256_CBC = 5,
	AES256_SIV = 12,
	AES128_CTR = 13,
	AES256_CTR = 14,
	AES128_XTS = 15,
	AES256_XTS = 16,
	AES128_CMAC = 19,
	AES256_CMAC = 20,
} hwkm_alg_t;

/* Master key slot assignments. */
typedef enum {
	/* L1 KDKs - HW-only: */
	NKDK_L1 = 0,
	PKDK_L1 = 1,
	SKDK_L1 = 2,
	UKDK_L1 = 3,
	/* L2 KDKs: */
	TZ_NKDK_L2 = 4,
	TZ_PKDK_L2 = 5,
	TZ_SKDK_L2 = 6,
	MODEM_PKDK_L2 = 7,
	MODEM_SKDK_L2 = 8,
	TZ_UKDK_L2 = 9,
	/* TPKEY pair: */
	TPKEY_SLOT = 10,
	TPKEY_ODD_SLOT = 11,
	/* Swap key pair: */
	TZ_SWAP_KEY_SLOT = 12,
	TZ_SWAP_KEY_ODD_SLOT = 13,
	/* Wrap key pair: */
	TZ_WRAP_KEY_SLOT = 14,
	TZ_WRAP_KEY_ODD_SLOT = 15,
	/* General purpose scratch: */
	TZ_GENERAL_PURPOSE_SLOT1 = 16,
	TZ_GENERAL_PURPOSE_SLOT2 = 17,
	/* Persistent shared pairs: */
	PERSISTENT_SHARED_SLOT_PAIR1 = 18,
	PERSISTENT_SHARED_SLOT_PAIR1_ODD = 19,
	PERSISTENT_SHARED_SLOT_PAIR2 = 20,
	PERSISTENT_SHARED_SLOT_PAIR2_ODD = 21,
	/* Mixing key: */
	TZ_MIXING_KEY_SLOT = 22,
} hwkm_master_key_slots_t;

/* Driver-level key policy and BSVE structs. */

/* Key policy word packed into every command that creates or references a slot. */
typedef struct {
	bool km_by_tz_allowed;       /* TZ may issue commands against this slot. */
	bool km_by_nsec_allowed;     /* Non-secure world may use this slot. */
	bool km_by_modem_allowed;    /* Modem subsystem may use this slot. */
	bool km_by_spu_allowed;      /* SPU may use this slot. */
	hwkm_alg_t alg_allowed;      /* Only algorithm permitted for this key. */
	bool enc_allowed;            /* Key may be used for encryption. */
	bool dec_allowed;            /* Key may be used for decryption. */
	hwkm_type_t key_type;        /* Functional role. */
	uint8_t kdf_depth;           /* Max remaining KDF derivation hops.  */
	bool wrap_export_allowed;    /* Key may be exported via KEY_WRAP_EXPORT. */
	bool swap_export_allowed;    /* Key may be delivered via key-swap. */
	hwkm_security_level_t security_lvl;  /* SW_KEY / MANAGED_KEY / HW_KEY. */
	hwkm_destination_t hw_destination;   /* Slave engine this key is bound to. */
	bool wrap_with_tpk_allowed;  /* Wrapping under the TPKEY is permitted. */
} hwkm_key_policy_t;

/* Boot-stage validation enable word controls which fuse/state checks are
 * enforced by hardware during SYSTEM_KDF. */
typedef struct {
	bool enabled;                /* Master enable for all BSVE checks. */
	bool km_key_policy_ver_en;   /* Enforce key policy version field. */
	bool km_apps_secure_en;      /* Require apps processor to be in secure state. */
	bool km_msa_secure_en;       /* Require modem to be in secure state. */
	bool km_lcm_fuse_en;         /* Bind derivation to life-cycle fuse state. */
	bool km_boot_stage_otp_en;   /* Bind derivation to boot-stage OTP value. */
	bool km_swc_en;              /* Enforce software component version check. */
	bool km_child_key_policy_en; /* Child key policy must be subset of parent. */
	bool km_mks_en;              /* Mixing key slot value is included in KDF. */
	uint64_t km_fuse_region_sha_digest_en; /* Bitmask of fuse regions hashed into KDF. */
} hwkm_bsve_t;

/* Command and response structs. */

typedef enum {
	NIST_KEYGEN = 0,   /* Generate a fresh key into a slot via hardware PRNG. */
	SYSTEM_KDF,        /* Derive a child key from a KDK slot via hardware KDF. */
	QFPROM_KEY_RDWR,   /* Read or write a key from fuse storage - hardware only. */
	KEY_WRAP_EXPORT,   /* Encrypt a slot under a KWK/KSK and return the blob. */
	KEY_UNWRAP_IMPORT, /* Decrypt a wrapped blob and write the key into a slot. */
	KEY_SLOT_CLEAR,    /* Zeroize a slot and invalidate its policy word. */
	KEY_SLOT_RDWR,     /* Read or write raw key material for a SW_KEY slot. */
	SET_TPKEY,         /* Install a slot as the active transport protection key. */
} hwkm_op_t;

typedef struct {
	uint8_t dks;
	hwkm_key_policy_t policy;
} hwkm_keygen_cmd_t;

typedef struct {
	uint8_t slot;
	bool is_write;
	hwkm_key_policy_t policy;
	uint8_t key[HWKM_MAX_KEY_SIZE];
	size_t sz;
} hwkm_rdwr_cmd_t;

typedef struct {
	uint8_t dks;
	uint8_t kdk;
	uint8_t mks;
	hwkm_key_policy_t policy;
	hwkm_bsve_t bsve;
	uint8_t ctx[HWKM_MAX_CTX_SIZE];
	size_t sz;
} hwkm_kdf_cmd_t;

typedef struct {
	uint8_t sks;
} hwkm_set_tpkey_cmd_t;

typedef struct {
	uint8_t dks;
	uint8_t kwk;
	uint8_t wkb[HWKM_MAX_BLOB_SIZE];
	uint8_t sz;
} hwkm_unwrap_cmd_t;

typedef struct {
	uint8_t sks;
	uint8_t kwk;
	hwkm_bsve_t bsve;
} hwkm_wrap_cmd_t;

typedef struct {
	uint8_t dks;
	bool is_double_key;
} hwkm_clear_cmd_t;

typedef struct {
	hwkm_op_t op;
	union {
		hwkm_keygen_cmd_t keygen;
		hwkm_rdwr_cmd_t rdwr;
		hwkm_kdf_cmd_t kdf;
		hwkm_set_tpkey_cmd_t set_tpkey;
		hwkm_unwrap_cmd_t unwrap;
		hwkm_wrap_cmd_t wrap;
		hwkm_clear_cmd_t clear;
	};
} hwkm_cmd_t;

typedef struct {
	hwkm_key_policy_t policy;
	uint8_t key[HWKM_MAX_KEY_SIZE];
	size_t sz;
} hwkm_rdwr_rsp_t;

typedef struct {
	uint8_t wkb[HWKM_MAX_BLOB_SIZE];
	size_t sz;
} hwkm_wrap_rsp_t;

typedef struct {
	uint32_t status;
	union {
		hwkm_rdwr_rsp_t rdwr;
		hwkm_wrap_rsp_t wrap;
	};
} hwkm_rsp_t;

/* END. Command and response structs. */

/* Hardware info. */
typedef struct {
	uint32_t ipcat_version;
	uint32_t key_policy_version;
	uint32_t key_table_size;
	bool tpkey_ready;
	uint8_t tpkey_slot;
	bool bist_failed;
} hwkm_info_t;

/* Driver context is singleton, populated by hwkm_hal_init(). */
typedef struct hwkm_drv_ctx {
	/* Pre-mapped virtual bases - set once, never change: */
	vaddr_t  master_base;  /* KM_MASTER register window.  */
	vaddr_t  gpce0_base;   /* GPCE slave register window. */

	/* Serialises all hardware accesses from OP-TEE. */
	struct mutex lock;

	/* Master info read at init. */
	hwkm_info_t info;

	/* Cached one-shot key material. */
	uint8_t  huk[HW_UNIQUE_KEY_LENGTH];
	bool huk_ready;

	uint8_t die_id[32];   /* 32 bytes = one AES-256 key width. */
	bool die_id_ready;

	/* Set to true after hwkm_hal_init() completes successfully. */
	bool initialised;
} hwkm_drv_ctx_t;

/*
 * hwkm_get_context - return a pointer to the driver singleton.
 *
 * Returns NULL if hwkm_hal_init() has not been called yet.
 * All public API functions call this first and return an error if NULL.
 */
hwkm_drv_ctx_t *hwkm_get_context(void);

/*
 * Transaction handle and node.
 *
 * hwkm_handle_t is the per-call context that groups one or more commands
 * destined for the same hardware engine. hwkm_transaction_t is a singly-
 * linked list node that pairs one hwkm_cmd_t with its hwkm_rsp_t.
 *
 * Typical usage - derive a key then immediately clear the source slot:
 *
 *   hwkm_cmd_t kdf_cmd = { .op = SYSTEM_KDF, ... };
 *   hwkm_rsp_t kdf_rsp = { };
 *   hwkm_transaction_t kdf_tran = { };
 *
 *   hwkm_cmd_t clr_cmd = { .op = KEY_SLOT_CLEAR, ... };
 *   hwkm_rsp_t clr_rsp = { };
 *   hwkm_transaction_t clr_tran = { };
 *
 *   hwkm_handle_t h = { };
 *   hwkm_handle_init(&h, hwkm_get_context(), KM_MASTER);
 *   hwkm_add_req(&h, &kdf_tran, &kdf_cmd, &kdf_rsp);
 *   hwkm_add_req(&h, &clr_tran, &clr_cmd, &clr_rsp);
 *   hwkm_run_cmds(&h);
 *
 * hwkm_run_cmds() holds the driver mutex for the entire batch, so commands
 * within one handle are issued atomically with respect to other callers.
 * The queue is cleared on return regardless of whether an error occurred,
 * so the handle must not be reused after hwkm_run_cmds() returns.
 */
typedef struct hwkm_transaction {
	hwkm_cmd_t *cmd;  /* command to issue                  */
	hwkm_rsp_t *rsp;  /* buffer for the hardware response  */
	struct hwkm_transaction *next;
} hwkm_transaction_t;

/* Per-call handle; initialise with hwkm_handle_init() before use. */
typedef struct {
	hwkm_drv_ctx_t *drv;       /* driver singleton from hwkm_get_context() */
	hwkm_destination_t dest;   /* target engine: KM_MASTER or GPCE_SLAVE   */
	hwkm_transaction_t *front;
	hwkm_transaction_t *end;
} hwkm_handle_t;

/* PUBLIC API. */

int hwkm_handle_init(hwkm_handle_t *h, hwkm_drv_ctx_t *drv,
		     hwkm_destination_t dest);
int hwkm_add_req(hwkm_handle_t *h, hwkm_transaction_t *t,
		 hwkm_cmd_t *cmd, hwkm_rsp_t *rsp);
int hwkm_run_cmds(hwkm_handle_t *h);

/* END. PUBLIC API. */

TEE_Result hwkm_hal_init(void);

#endif /* __HWKM_HAL_H__ */
