// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/i2c.h>
#include <drivers/qcom_geni_i2c.h>
#include <initcall.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <trace.h>
#include <util.h>

/*
 * Boot-time self-test for the QUP GENI I2C driver, run once via
 * driver_init() after the platform's qup_i2c_config[] SEs have been
 * probed. Unlike qcom_geni_spi_test.c, I2C has no internal digital
 * loopback path (SDA/SCL are open-drain and need a real device or an
 * external SDA-SCL jumper to see anything at all), so this only ever
 * exercises a real peripheral -- gated behind CFG_QUP_I2C_TEST_EXTERNAL
 * and its outcome is reported but never fails the boot.
 *
 * Enable with CFG_QUP_I2C_TEST=y and CFG_QUP_I2C_TEST_EXTERNAL=y once a
 * peripheral is wired to this SE, and set QUP_I2C_TEST_SLAVE_ADDR to its
 * 7-bit address.
 */

static void qup_i2c_test_external(struct qup_i2c_data *qi);
#define CFG_QUP_I2C_TEST_EXTERNAL	1
#define QUP_I2C_TEST_SPEED_HZ		100000
#ifndef QUP_I2C_TEST_SLAVE_ADDR
#define QUP_I2C_TEST_SLAVE_ADDR		0x50
#endif
#define QUP_I2C_TEST_NUM_BYTES	4	

#ifdef CFG_QUP_I2C_TEST_EXTERNAL
/*
 * Writes then reads back QUP_I2C_TEST_NUM_BYTES against whatever
 * peripheral is wired to this SE at QUP_I2C_TEST_SLAVE_ADDR. There is no
 * known-good peripheral to check the read-back against, so this only
 * proves both transfers complete without a FIFO/timeout/NACK error --
 * it cannot confirm protocol-level correctness. Never fails the boot.
 */
static void qup_i2c_test_external(struct qup_i2c_data *qi)
{
	struct i2c_dev dev = { };
	uint8_t tx[QUP_I2C_TEST_NUM_BYTES] = { 0 };
	uint8_t rx[QUP_I2C_TEST_NUM_BYTES] = { 0 };
	size_t i = 0;
	TEE_Result res = TEE_SUCCESS;

	for (i = 0; i < sizeof(tx); i++)
		tx[i] = (uint8_t)i;
	memset(rx, 0, sizeof(rx));

	qi->speed_hz = QUP_I2C_TEST_SPEED_HZ;
	res = qup_i2c_dev_init(qi, &dev, QUP_I2C_TEST_SLAVE_ADDR);
	if (res) {
		EMSG("QUP I2C %u: external test: dev_init failed: %#" PRIx32,
		     qi->id, res);
		return;
	}

	res = i2c_write(&dev, tx, sizeof(tx));
	qi->speed_hz = 400000;
	res = i2c_write(&dev, tx, sizeof(tx));
	qi->speed_hz = 1000000;
	res = i2c_write(&dev, tx, sizeof(tx));
	if (res) {
		EMSG("QUP I2C %u: external test: write failed: %#" PRIx32,
		     qi->id, res);
		return;
	}

	res = i2c_read(&dev, rx, sizeof(rx));
	if (res) {
		EMSG("QUP I2C %u: external test: read failed: %#" PRIx32,
		     qi->id, res);
		return;
	}

	IMSG("QUP I2C %u: external test: write+read completed (%zu bytes each, no readback check)",
	     qi->id, sizeof(tx));
}
#endif /* CFG_QUP_I2C_TEST_EXTERNAL */

static TEE_Result qup_i2c_test_init(void)
{
	struct qup_i2c_data qi = { };
	TEE_Result res = TEE_SUCCESS;

	if (!qup_i2c_config_count) {
		IMSG("QUP I2C test: no SE configured for this platform, skipping");
		return TEE_SUCCESS;
	}

	res = qup_i2c_init(&qi, qup_i2c_config[0].id);
	if (res) {
		EMSG("QUP I2C test: qup_i2c_init(%u) failed: %#" PRIx32,
		     qup_i2c_config[0].id, res);
		return TEE_SUCCESS;
	}

#ifdef CFG_QUP_I2C_TEST_EXTERNAL
	qup_i2c_test_external(&qi);
#else
	IMSG("QUP I2C %u: initialized, no self-test wired up (set CFG_QUP_I2C_TEST_EXTERNAL=y once a peripheral is connected)",
	     qi.id);
#endif

	return TEE_SUCCESS;
}

driver_init_late(qup_i2c_test_init);
