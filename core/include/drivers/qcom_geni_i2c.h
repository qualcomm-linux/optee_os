/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef DRIVERS_QCOM_GENI_I2C_H
#define DRIVERS_QCOM_GENI_I2C_H

#include <drivers/i2c.h>
#include <mm/core_memprot.h>
#include <types_ext.h>

struct clk;
struct tlmm_pin_group;
struct pinctrl_state;

/* Wrapper/common clocks per SE: core_2x, core, s_ahb, m_ahb */
#define QUP_I2C_MAX_COMMON_CLKS		4

/* Register window size mapped for each GENI I2C SE. */
#define QUP_I2C_REG_SIZE		0x4000

/* Register window size mapped for the QUPv3 wrapper-common region. */
#define QUP_I2C_COMMON_REG_SIZE	0x1000

struct qup_i2c_data {
	/*
	 * Embeds the generic i2c_ctrl_ops table (see drivers/i2c.h); bind a
	 * caller's struct i2c_dev to it via qup_i2c_dev_init() -- no
	 * devicetree involved, same static-table style as qup_spi_data.
	 */
	struct i2c_ctrl ctrl;
	vaddr_t base;
	/* QUPv3 wrapper-common register region for this SE's wrapper */
	vaddr_t common_base;
	/* Stable identifier for the SE this instance is bound to */
	unsigned int id;
	/* SE source ("se") clock rate in Hz, fixed by the platform */
	unsigned int clk_hz;
	/* SE source ("se") clock name, used to acquire se_clk */
	const char *se_clock_name;
	/* SE source clock handle, acquired by qup_i2c_init() */
	struct clk *se_clk;
	/* NULL-terminated array of wrapper/common clock names */
	const char **common_clocks_name;
	/* Resolved wrapper/common clock handles, acquired by qup_i2c_init() */
	struct clk *common_clks[QUP_I2C_MAX_COMMON_CLKS];
	/* Number of entries populated in common_clks[] */
	unsigned int num_common_clks;
	/*
	 * TLMM pin groups for this SE's pads, from the platform cfg. The
	 * resolved pinctrl state is built and applied once by qup_i2c_init().
	 */
	const struct tlmm_pin_group *pin_groups;
	unsigned int pin_group_count;
	struct pinctrl_state *pin_state;
	/*
	 * GENI SE firmware image for this SE, from the platform cfg. NULL if
	 * this SE does not need firmware loading (e.g. already loaded by an
	 * earlier boot stage on every boot path).
	 */
	const uint8_t *fw_image;
	size_t fw_image_size;
	/* Set once qup_i2c_init() has loaded (or found already-loaded) the
	 * GENI SE firmware; gates re-running the load on repeat init calls.
	 */
	bool fw_loaded;
	/* Requested I2C bus clock in Hz; set before the first read()/write().
	 * Only 100000 (Standard), 400000 (Fast) and 1000000 (Fast Plus) are
	 * supported, matching mainline's geni_i2c_clk_map tables.
	 */
	unsigned int speed_hz;
	/* Cached from SE_HW_PARAM_0 by qup_i2c_configure_bus() */
	unsigned int tx_fifo_depth;
	unsigned int fifo_width_bits;

	/* SE interrupt number, filled in by qup_i2c_get_platform_data() */
	size_t itr_num;

	/*
	 * Internal driver state. Not to be touched by callers.
	 */
	unsigned int lock;
	/* Set once the bus (clock/SCL counters) has been configured for
	 * qs->speed_hz; re-checked/rebuilt on every read()/write() in case
	 * speed_hz changed since.
	 */
	bool bus_configured;
	unsigned int configured_speed_hz;
	const uint8_t *tx_buf;
	uint8_t *rx_buf;
	size_t tx_rem_bytes;
	size_t rx_rem_bytes;
};

/*
 * Implemented by platform code: fills in base, itr_num, clk_hz (and
 * any other platform-fixed field) in *qi for the given SE id.
 * Returns false if qup_i2c_id was not enabled in this build.
 */
bool qup_i2c_get_platform_data(unsigned int qup_i2c_id,
				struct qup_i2c_data *qi);

/*
 * Initializes *qi for the SE identified by qup_i2c_id, loading its GENI
 * firmware if needed and muxing its pads. qi->speed_hz should be set
 * either before or after this call, but before the first
 * qup_i2c_dev_init()-bound read()/write().
 */
TEE_Result qup_i2c_init(struct qup_i2c_data *qi, unsigned int qup_i2c_id);

/*
 * Binds *dev to *qi at the given 7-bit slave address, so dev can be passed
 * to i2c_read()/i2c_write() (drivers/i2c.h). Direct equivalent of
 * i2c_create_dev(), without the devicetree lookup. Returns
 * TEE_ERROR_BAD_PARAMETERS if addr is not a 7-bit address.
 *
 * Note: this controller implements only .read and .write. SMBus-style
 * combined transfers (write-then-read with a repeated START) are not
 * supported, so .smbus is left NULL and i2c_smbus_raw() and its helpers
 * return TEE_ERROR_NOT_SUPPORTED for this bus.
 */
TEE_Result qup_i2c_dev_init(struct qup_i2c_data *qi, struct i2c_dev *dev,
			     uint16_t addr);

/*
 * Per-SE hardware descriptor. Defined per platform in
 * qcom_geni_i2c_config.c; qup_i2c_config[] below is the resulting
 * table qup_i2c_get_platform_data() looks up by id.
 */
struct qup_i2c_platform_cfg {
	unsigned int id;
	paddr_t base;
	/* QUPv3 wrapper-common region physical base for this SE's wrapper */
	paddr_t common_base;
	size_t itr_num;
	unsigned int clk_hz;
	const char *se_clock_name;
	const char **common_clocks_name;
	/*
	 * TLMM pin groups muxing this SE's SDA/SCL pads to the I2C
	 * function. NULL/0 if the platform sets up pinmux elsewhere.
	 */
	const struct tlmm_pin_group *pin_groups;
	unsigned int pin_group_count;
	/*
	 * GENI SE firmware image to load into this SE, and its size in
	 * bytes. NULL/0 if this SE does not need firmware loading.
	 */
	const uint8_t *fw_image;
	size_t fw_image_size;
};

/* SoC-specific hardware descriptor, defined per platform in qcom_geni_i2c_config.c */
extern const struct qup_i2c_platform_cfg qup_i2c_config[];

extern const size_t qup_i2c_config_count;
#endif /* DRIVERS_QCOM_GENI_I2C_H */
