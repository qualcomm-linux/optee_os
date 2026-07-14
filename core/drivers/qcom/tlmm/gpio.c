// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <assert.h>
#include <drivers/gpio.h>
#include <drivers/qcom/tlmm/tlmm.h>
#include <initcall.h>
#include <io.h>
#include <keep.h>
#include <kernel/spinlock.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <trace.h>
#include <util.h>

#include "tlmm_private.h"

register_phys_mem_pgdir(MEM_AREA_IO_SEC, TLMM_BASE, TLMM_BASE_SIZE);

struct tlmm_chip tlmm = { };

static const struct gpio_ops tlmm_gpio_ops;

uint32_t tlmm_tile_offset(const struct tlmm_chip *chip, unsigned int pin)
{
	const struct tlmm_desc *d = chip->desc;
	unsigned int t = 0;
	vaddr_t id_reg = 0;

	if (d->num_tiles <= 1)
		return d->tile_offsets[0];

	for (t = 0; t < d->num_tiles; t++) {
		id_reg = chip->base
			 + d->tile_offsets[t]
			 + (vaddr_t)d->pin_reg_width * pin
			 + TLMM_REG_ID_STATUS;
		if (io_read32(id_reg) & TLMM_ID_STATUS_PRESENT)
			return d->tile_offsets[t];
	}

	EMSG("TLMM: pin %u not found in any tile, defaulting to tile 0", pin);
	return d->tile_offsets[0];
}

vaddr_t tlmm_pin_reg(const struct tlmm_chip *chip, unsigned int pin,
		     uint32_t reg_off)
{
	return chip->base
	       + tlmm_tile_offset(chip, pin)
	       + (vaddr_t)chip->desc->pin_reg_width * pin
	       + reg_off;
}

void tlmm_write_cfg(struct tlmm_chip *chip, unsigned int pin,
		    uint32_t clear_mask, uint32_t set_mask)
{
	vaddr_t reg = TLMM_GPIO_CFG(chip, pin);
	uint32_t val = io_read32(reg);

	clear_mask &= ~TLMM_CFG_PRESERVE_MASK;

	if (chip->desc->has_egpio && (val & TLMM_CFG_EGPIO_PRESENT))
		set_mask |= TLMM_CFG_EGPIO_ENABLE;

	val = (val & ~clear_mask) | set_mask;
	io_write32(reg, val);
}

static bool is_tlmm_chip(struct gpio_chip *chip)
{
	return chip && chip->ops == &tlmm_gpio_ops;
}

static struct tlmm_chip *chip_to_tlmm(struct gpio_chip *chip)
{
	assert(is_tlmm_chip(chip));
	return container_of(chip, struct tlmm_chip, gpio_chip);
}

static bool tlmm_pin_is_owned(unsigned int pin)
{
	if (pin >= TLMM_MAX_GPIOS || pin >= tlmm.desc->num_gpios)
		return false;

	return tlmm.pin_owners[pin / 32] & BIT(pin % 32);
}

void tlmm_restore_lp(struct tlmm_chip *chip, unsigned int pin)
{
	uint32_t lp_val = io_read32(TLMM_GPIO_LP_CFG(chip, pin));

	if (!(lp_val & TLMM_LP_CFG_APPLIED))
		return;

	io_write32(TLMM_GPIO_IN_OUT(chip, pin),
		   (lp_val & TLMM_LP_CFG_OUT_VAL) ? TLMM_IN_OUT_OUT_BIT : U(0));

	io_write32(TLMM_GPIO_CFG(chip, pin), lp_val & TLMM_LP_CFG_CONFIG_MASK);
}

TEE_Result tlmm_request_pin(unsigned int pin)
{
	uint32_t exceptions = 0;

	if (pin >= TLMM_MAX_GPIOS || pin >= tlmm.desc->num_gpios)
		return TEE_ERROR_BAD_PARAMETERS;

	exceptions = cpu_spin_lock_xsave(&tlmm.lock);
	if (tlmm.pin_owners[pin / 32] & BIT(pin % 32)) {
		cpu_spin_unlock_xrestore(&tlmm.lock, exceptions);
		EMSG("TLMM: pin %u already owned", pin);
		return TEE_ERROR_BUSY;
	}
	tlmm.pin_owners[pin / 32] |= BIT(pin % 32);
	cpu_spin_unlock_xrestore(&tlmm.lock, exceptions);

	return TEE_SUCCESS;
}

void tlmm_release_pin(unsigned int pin)
{
	uint32_t exceptions = 0;

	if (pin >= TLMM_MAX_GPIOS || pin >= tlmm.desc->num_gpios)
		return;

	tlmm_restore_lp(&tlmm, pin);

	exceptions = cpu_spin_lock_xsave(&tlmm.lock);
	tlmm.pin_owners[pin / 32] &= ~BIT(pin % 32);
	cpu_spin_unlock_xrestore(&tlmm.lock, exceptions);
}

static enum gpio_dir tlmm_get_direction(struct gpio_chip *chip,
					unsigned int pin)
{
	struct tlmm_chip *tc = chip_to_tlmm(chip);

	assert(pin < tc->desc->num_gpios);
	return (io_read32(TLMM_GPIO_CFG(tc, pin)) & TLMM_CFG_OE) ?
		GPIO_DIR_OUT : GPIO_DIR_IN;
}

static void tlmm_set_direction(struct gpio_chip *chip, unsigned int pin,
			       enum gpio_dir dir)
{
	struct tlmm_chip *tc = chip_to_tlmm(chip);

	assert(pin < tc->desc->num_gpios);
	if (!tlmm_pin_is_owned(pin)) {
		EMSG("TLMM: pin %u not owned, set_direction rejected", pin);
		return;
	}
	if (dir == GPIO_DIR_OUT)
		tlmm_write_cfg(tc, pin, 0, TLMM_CFG_OE);
	else
		tlmm_write_cfg(tc, pin, TLMM_CFG_OE, 0);
}

static enum gpio_level tlmm_get_value(struct gpio_chip *chip, unsigned int pin)
{
	struct tlmm_chip *tc = chip_to_tlmm(chip);

	assert(pin < tc->desc->num_gpios);
	return (io_read32(TLMM_GPIO_IN_OUT(tc, pin)) & TLMM_IN_OUT_IN_BIT) ?
		GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
}

static void tlmm_set_value(struct gpio_chip *chip, unsigned int pin,
			   enum gpio_level level)
{
	struct tlmm_chip *tc = chip_to_tlmm(chip);

	assert(pin < tc->desc->num_gpios);
	if (!tlmm_pin_is_owned(pin)) {
		EMSG("TLMM: pin %u not owned, set_value rejected", pin);
		return;
	}
	io_write32(TLMM_GPIO_IN_OUT(tc, pin),
		   (level == GPIO_LEVEL_HIGH) ? TLMM_IN_OUT_OUT_BIT : U(0));
}

static const struct gpio_ops tlmm_gpio_ops = {
	.get_direction  = tlmm_get_direction,
	.set_direction  = tlmm_set_direction,
	.get_value      = tlmm_get_value,
	.set_value      = tlmm_set_value,
};
DECLARE_KEEP_PAGER(tlmm_gpio_ops);

struct gpio_chip *tlmm_get_chip(void)
{
	assert(tlmm.gpio_chip.ops);
	return &tlmm.gpio_chip;
}

static TEE_Result tlmm_init(void)
{
	const struct tlmm_desc *desc = &tlmm_soc_desc;

	DMSG("TLMM: init start, mapping base 0x%"PRIxPA" size 0x%zx",
	     desc->base, desc->size);

	tlmm.base = (vaddr_t)phys_to_virt(desc->base, MEM_AREA_IO_SEC,
					  desc->size);
	if (!tlmm.base) {
		EMSG("TLMM: failed to map base 0x%"PRIxPA, desc->base);
		return TEE_ERROR_GENERIC;
	}

	tlmm.desc = desc;
	tlmm.gpio_chip.ops = &tlmm_gpio_ops;

	DMSG("TLMM: mapped base 0x%"PRIxPA" -> va 0x%"PRIxVA
	     ", stride 0x%x, tile0 0x%x, strong_pull=%s",
	     desc->base, tlmm.base, desc->pin_reg_width,
	     desc->tile_offsets[0], desc->has_strong_pull ? "yes" : "no");

	IMSG("TLMM: %u GPIOs, %u tile(s), egpio=%s, base=0x%"PRIxPA,
	     desc->num_gpios, desc->num_tiles,
	     desc->has_egpio ? "yes" : "no",
	     desc->base);

	return TEE_SUCCESS;
}

driver_init(tlmm_init);
