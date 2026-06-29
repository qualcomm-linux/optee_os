/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * TLMM GPIO driver — internal register definitions and data structures.
 *
 * Not for inclusion outside the tlmm driver directory.
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __QCOM_TLMM_PRIVATE_H
#define __QCOM_TLMM_PRIVATE_H

#include <drivers/qcom/tlmm/tlmm.h>
#include <io.h>
#include <kernel/mutex.h>
#include <stdint.h>
#include <tee_api_types.h>
#include <trace.h>

/*
 * GPIO_CFG register — bit field layout.
 * Address: TLMM_BASE + tile_offset + pin * pin_reg_width + 0x00
 *
 * Talos  — bits [10:0]  (no eGPIO, no STRONG_PULL)
 * LeMans — bits [12:0]  (adds EGPIO_PRESENT[11], EGPIO_ENABLE[12])
 * Newer  — bits [13:0]  (adds STRONG_PULL[13])
 *
 * [1:0]  GPIO_PULL      — 0=no-pull, 1=pull-down, 2=keeper, 3=pull-up
 * [5:2]  FUNC_SEL       — function select (0 = GPIO mode)
 * [8:6]  DRV_STRENGTH   — 0=2mA ... 7=16mA in 2mA steps
 * [9]    GPIO_OE        — output enable (1=output, 0=input)
 * [10]   GPIO_HIHYS_EN  — high-hysteresis; HW-managed, never clear
 * [11]   EGPIO_PRESENT  — R-only; LPASS/SSC can own this GPIO
 * [12]   EGPIO_ENABLE   — RW; must be 1 when EGPIO_PRESENT=1
 * [13]   STRONG_PULL    — RW; strong pull-up (I2C); newer SoCs only
 */
#define TLMM_CFG_PULL_MASK		GENMASK_32(1, 0)
#define TLMM_CFG_FUNC_SHIFT		U(2)
#define TLMM_CFG_FUNC_MASK		GENMASK_32(5, 2)
#define TLMM_CFG_DRIVE_SHIFT		U(6)
#define TLMM_CFG_DRIVE_MASK		GENMASK_32(8, 6)
#define TLMM_CFG_OE			BIT(9)
#define TLMM_CFG_HIHYS			BIT(10)
#define TLMM_CFG_EGPIO_PRESENT		BIT(11)
#define TLMM_CFG_EGPIO_ENABLE		BIT(12)
#define TLMM_CFG_STRONG_PULL		BIT(13)

/* Bits preserved unconditionally on every GPIO_CFG RMW */
#define TLMM_CFG_PRESERVE_MASK		TLMM_CFG_HIHYS

/*
 * GPIO_IN_OUT register — bit field layout.
 * Address: TLMM_BASE + tile_offset + pin * pin_reg_width + 0x04
 *
 * [0]  GPIO_IN   R   — current pad level
 * [1]  GPIO_OUT  RW  — output latch (direct write, no RMW needed)
 */
#define TLMM_IN_OUT_IN_BIT		BIT(0)
#define TLMM_IN_OUT_OUT_BIT		BIT(1)

/*
 * GPIO_ID_STATUS register.
 * [0]  R — 1 if this pin exists in this tile. Probed on-demand for
 *          multi-tile SoCs. Never bulk-probed (HLOS pins are XPU-protected).
 */
#define TLMM_ID_STATUS_PRESENT		BIT(0)

/* Compile-time maximum GPIOs supported — sized for the ownership bitmap */
#define TLMM_MAX_GPIOS			U(256)

/* Register offsets within each pin's register set */
#define TLMM_REG_CFG			U(0x00)
#define TLMM_REG_IN_OUT			U(0x04)
#define TLMM_REG_ID_STATUS		U(0x10)
#define TLMM_REG_LP_CFG			U(0x14)

/*
 * LP_CFG register — XBL programs this for every pin before handing off
 * to OP-TEE.  Bits [12:0] share the same layout as GPIO_CFG.  Bit 13
 * meaning differs per SoC (GPIO_VALID on Lemans, I2C_PULL on others) —
 * tlmm_desc.lp_cfg_mask controls which bits are copied to GPIO_CFG on
 * pin release.
 */

/*
 * struct tlmm_chip - Runtime state for one TLMM controller instance.
 *
 * @gpio_chip:   Embedded gpio_chip — MUST be first member so container_of
 *               in chip_to_tlmm() resolves to a zero offset.
 * @base:        Virtual base address after MMU mapping.
 * @desc:        SoC-specific hardware descriptor.
 * @lock:        Serialises access to pin_owners.
 * @pin_owners:  Bitmap tracking which pins are currently claimed.  Each bit
 *               is set by tlmm_request_pin() and cleared by
 *               tlmm_release_pin().  conf_apply() sets bits atomically for
 *               the whole pin group; conf_free() clears them.
 */
struct tlmm_chip {
	struct gpio_chip        gpio_chip;
	vaddr_t                 base;
	const struct tlmm_desc *desc;
	struct mutex            lock; /* serialises pin_owners access */
	uint32_t                pin_owners[TLMM_MAX_GPIOS / 32];
};

/*
 * Function declarations — defined in tlmm.c, shared with tlmm_pinctrl.c.
 * Headers declare, .c files define — no function bodies in headers.
 */

/* Return the tile byte-offset that owns @pin */
uint32_t tlmm_tile_offset(const struct tlmm_chip *chip, unsigned int pin);

/* Compute virtual address: base + tile_offset + stride*pin + reg_off */
vaddr_t tlmm_pin_reg(const struct tlmm_chip *chip, unsigned int pin,
		     uint32_t reg_off);

/* RMW GPIO_CFG — preserves HIHYS, auto-sets EGPIO_ENABLE when needed */
void tlmm_write_cfg(struct tlmm_chip *chip, unsigned int pin,
		    uint32_t clear_mask, uint32_t set_mask);

/* Restore GPIO_CFG from LP_CFG on pin release */
void tlmm_restore_lp(struct tlmm_chip *chip, unsigned int pin);

/* Convenience address macros */
#define TLMM_GPIO_CFG(c, p)		tlmm_pin_reg((c), (p), TLMM_REG_CFG)
#define TLMM_GPIO_IN_OUT(c, p)		tlmm_pin_reg((c), (p), TLMM_REG_IN_OUT)
#define TLMM_GPIO_ID_STATUS(c, p)	\
	tlmm_pin_reg((c), (p), TLMM_REG_ID_STATUS)
#define TLMM_GPIO_LP_CFG(c, p)		tlmm_pin_reg((c), (p), TLMM_REG_LP_CFG)

/* The single TLMM controller instance — defined in tlmm.c */
extern struct tlmm_chip tlmm;

#endif /* __QCOM_TLMM_PRIVATE_H */
