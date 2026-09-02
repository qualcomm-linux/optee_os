// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <assert.h>
#include <drivers/clk.h>
#include <drivers/clk_qcom.h>
#include <drivers/qcom/tlmm/tlmm.h>
#include <drivers/qcom_geni_i2c.h>
#include <inttypes.h>
#include <io.h>
#include <kernel/delay.h>
#include <kernel/spinlock.h>
#include <keep.h>
#include <mm/core_memprot.h>
#include <string.h>
#include <trace.h>
#include <util.h>

/*
 * Qualcomm GENI I2C driver, structured exactly like qcom_geni_spi.c (same
 * SE, same generic GENI registers, same firmware-load sequence, same
 * polling-mode M_CMD completion): see that file's comments for the register
 * offsets shared between both protocols. This file only documents what is
 * I2C-specific.
 */

/* Generic GENI Serial Engine (SE) registers -- identical to qcom_geni_spi.c */
#define SE_GENI_STATUS			0x40
#define GENI_SER_M_CLK_CFG		0x48
#define GENI_FW_REVISION_RO		0x68
#define SE_GENI_CLK_SEL			0x7c
#define SE_GENI_DFS_IF_CFG		0x80
#define SE_GENI_DMA_MODE_EN		0x258
#define SE_GENI_M_CMD0			0x600
#define SE_GENI_M_CMD_CTRL_REG		0x604
#define SE_GENI_M_IRQ_STATUS		0x610
#define SE_GENI_M_IRQ_EN		0x614
#define SE_GENI_M_IRQ_CLEAR		0x618
#define SE_GENI_TX_FIFOn		0x700
#define SE_GENI_RX_FIFOn		0x780
#define SE_GENI_TX_FIFO_STATUS		0x800
#define SE_GENI_RX_FIFO_STATUS		0x804
#define SE_GENI_TX_WATERMARK_REG	0x80c
#define SE_GENI_RX_WATERMARK_REG	0x810
#define SE_GENI_RX_RFR_WATERMARK_REG	0x814
#define SE_GENI_TX_PACKING_CFG0		0x260
#define SE_GENI_TX_PACKING_CFG1		0x264
#define SE_GENI_RX_PACKING_CFG0		0x284
#define SE_GENI_RX_PACKING_CFG1		0x288
#define SE_GENI_BYTE_GRAN		0x254
#define SE_HW_PARAM_0			0xe24

#define GENI_DMA_MODE_EN		BIT32(0)
#define	SER_DFS_EN			BIT32(0)
#define M_OPCODE_SHFT			27
#define M_CMD_DONE_EN			BIT32(0)
#define M_CMD_OVERRUN_EN		BIT32(1)
#define M_ILLEGAL_CMD_EN		BIT32(2)
#define M_CMD_FAILURE_EN		BIT32(3)
#define M_CMD_CANCEL_EN			BIT32(4)
#define M_CMD_ABORT_EN			BIT32(5)
#define M_GENI_CMD_CANCEL		BIT32(0)
#define M_GENI_CMD_ABORT		BIT32(1)
#define M_GP_IRQ_0_EN			BIT32(9)
#define M_GP_IRQ_1_EN			BIT32(10)
#define M_GP_IRQ_2_EN			BIT32(11)
#define M_GP_IRQ_3_EN			BIT32(12)
#define M_GP_IRQ_4_EN			BIT32(13)
#define M_GP_IRQ_5_EN			BIT32(14)
#define M_RX_FIFO_WATERMARK_EN		BIT32(26)
#define M_RX_FIFO_LAST_EN		BIT32(27)
#define M_TX_FIFO_WATERMARK_EN		BIT32(30)

/*
 * I2C bus/command errors that abort an in-flight transfer. Deliberately
 * identical to mainline i2c-qcom-geni.c's SE_I2C_ERR: M_GP_IRQ_1_EN is
 * NACK (slave unresponsive/absent), M_GP_IRQ_3_EN is a bus protocol
 * error, M_GP_IRQ_4_EN is arbitration lost; the other three are GENI
 * command-engine errors. GP_IRQ_0/2/5 are deliberately NOT here -- they
 * are protocol-defined "general purpose" lines whose meaning for the I2C
 * firmware is not documented as fatal, and mainline does not treat them
 * as errors either. They are still decoded for logging by
 * qup_i2c_log_errors() when a real error is present.
 */
#define QUP_I2C_M_IRQ_ERR_MASK \
	(M_CMD_OVERRUN_EN | M_ILLEGAL_CMD_EN | M_CMD_FAILURE_EN | \
	 M_GP_IRQ_1_EN | M_GP_IRQ_3_EN | M_GP_IRQ_4_EN)

/*
 * Enabled in M_IRQ_EN so they show up in M_IRQ_STATUS. GP_IRQ_0/2/5 are
 * enabled purely so qup_i2c_log_errors() can report them if they ever
 * accompany a real error; they are not in QUP_I2C_M_IRQ_ERR_MASK, so
 * seeing one on its own never aborts a transfer.
 */
#define QUP_I2C_M_IRQ_EN_MASK \
	(M_CMD_DONE_EN | M_CMD_CANCEL_EN | M_CMD_ABORT_EN | \
	 M_RX_FIFO_WATERMARK_EN | M_RX_FIFO_LAST_EN | M_TX_FIFO_WATERMARK_EN | \
	 QUP_I2C_M_IRQ_ERR_MASK | \
	 M_GP_IRQ_0_EN | M_GP_IRQ_2_EN | M_GP_IRQ_5_EN)

#define TX_FIFO_WC_MSK			GENMASK_32(27, 0)
#define RX_FIFO_WC_MSK			GENMASK_32(24, 0)

/* GENI_SER_M_CLK_CFG fields (same as qcom_geni_spi.c). */
#define CLK_DIV_SHFT			4
#define CLK_DIV_MSK			GENMASK_32(15, 4)
#define SER_CLK_EN			BIT32(0)

#define TX_FIFO_WIDTH_MSK		GENMASK_32(29, 24)
#define TX_FIFO_WIDTH_SHFT		24
#define TX_FIFO_DEPTH_MSK		GENMASK_32(21, 16)
#define TX_FIFO_DEPTH_SHFT		16

#define GENI_STATUS_M_GENI_CMD_ACTIVE	BIT32(0)

/*
 * Byte-packing vector fields -- same generic SE hardware block as SPI's.
 * I2C is always byte-oriented (8-bit words), so this only ever runs with
 * bpw=8, packing 4 bytes per 32-bit FIFO word.
 */
#define NUM_PACKING_VECTORS		4
#define PACKING_START_SHIFT		5
#define PACKING_DIR_SHIFT		4
#define PACKING_LEN_SHIFT		1
#define PACKING_STOP_BIT		BIT32(0)
#define PACKING_VECTOR_SHIFT		10
#define QUP_I2C_BITS_PER_WORD		8
/*
 * Protocol words (bytes) packed into each 32-bit FIFO entry. Mainline
 * i2c-qcom-geni.c uses the same value (PACKING_BYTES_PW). This is used
 * both to build the packing vectors and to drive the FIFO fill/drain
 * loops -- the two MUST agree or the SE (un)packs a different number of
 * bytes per FIFO access than the driver moves.
 */
#define QUP_I2C_PACKING_BYTES_PW	4

/*
 * GENI SE firmware-load registers -- identical offsets to qcom_geni_spi.c
 * (verified there against the IP Catalog register map and against
 * u-boot's qcom_geni.c/qup-fw-load.h for this SE generation).
 */
#define GENI_INIT_CFG_REVISION		0x0
#define GENI_S_INIT_CFG_REVISION	0x4
#define GENI_FORCE_DEFAULT_REG		0x20
#define GENI_OUTPUT_CTRL		0x24
#define GENI_CGC_CTRL			0x28
#define GENI_CGC_CTRL_PROG_RAM_MSK	(BIT32(9) | BIT32(8))
#define GENI_CGC_CTRL_DEFAULT_EN_MSK	GENMASK_32(6, 0)
#define GENI_CFG_REG0			0x100
#define SE_HW_PARAM_1			0xe28
#define RX_FIFO_WIDTH_SHFT		24
#define RX_FIFO_WIDTH_MSK		GENMASK_32(29, 24)
#define SE_GSI_EVENT_EN			0xe18
#define SE_IRQ_EN			0xe1c
#define SE_IRQ_EN_MSK			GENMASK_32(3, 0)
#define SE_DMA_GENERAL_CFG		0xe30
#define SE_DMA_GENERAL_CFG_CGC_ON_MSK	GENMASK_32(3, 0)
#define SE_GENI_S_IRQ_EN		0x644
#define S_CMD_OVERRUN_EN		BIT32(1)
#define S_ILLEGAL_CMD_EN		BIT32(2)
#define S_CMD_CANCEL_EN			BIT32(4)
#define S_CMD_ABORT_EN			BIT32(5)
#define S_GP_IRQ_0_EN			BIT32(9)
#define S_GP_IRQ_1_EN			BIT32(10)
#define S_GP_IRQ_2_EN			BIT32(11)
#define S_GP_IRQ_3_EN			BIT32(12)
#define S_RX_FIFO_WR_ERR_EN		BIT32(25)
#define S_RX_FIFO_RD_ERR_EN		BIT32(24)
#define SE_DMA_TX_IRQ_EN_SET		0xc4c
#define DMA_TX_RESET_DONE_EN_SET	BIT32(3)
#define DMA_TX_SBE_EN_SET		BIT32(2)
#define DMA_TX_DMA_DONE_EN_SET		BIT32(0)
#define SE_DMA_RX_IRQ_EN_SET		0xd4c
#define DMA_RX_FLUSH_DONE_EN_SET	BIT32(4)
#define DMA_RX_RESET_DONE_EN_SET	BIT32(3)
#define DMA_RX_SBE_EN_SET		BIT32(2)
#define DMA_RX_DMA_DONE_EN_SET		BIT32(0)
#define SE_GENI_FW_REVISION		0x1000
#define SE_S_FW_REVISION		0x1004
#define FW_REV_PROTOCOL_SHFT		8
#define SE_GENI_CFG_RAMN		0x1010
#define SE_GENI_CLK_CTRL		0x2000
#define GENI_CLK_CTRL_SER_CLK_SEL	BIT32(0)
#define SE_DMA_IF_EN			0x2004
#define DMA_IF_EN_DMA_IF_EN		BIT32(0)
#define SE_FIFO_IF_DISABLE		0x2008
#define FIFO_IF_DISABLE			BIT32(0)

/* QUPv3 wrapper-common registers, offset from qi->common_base */
#define QUPV3_SE_AHB_M_CFG		0x118
#define AHB_M_CLK_CGC_ON		BIT32(0)
#define QUPV3_COMMON_CFG		0x120
#define FAST_SWITCH_TO_HIGH_DISABLE	BIT32(0)
#define QUPV3_COMMON_CGC_CTRL		0x21c
#define COMMON_CSR_SLV_CLK_CGC_ON	BIT32(0)

#define QUP_FW_MAGIC			0x57464553
#define QUP_FW_HDR_VERSION		1
#define QUP_FW_SERIAL_PROTOCOL_I2C	3

/* Same header format as qcom_geni_spi.c's struct elf_se_hdr. */
struct elf_se_hdr {
	uint32_t magic;
	uint32_t version;
	uint32_t core_version;
	uint16_t serial_protocol;
	uint16_t fw_version;
	uint16_t cfg_version;
	uint16_t fw_size_in_items;
	uint16_t fw_offset;
	uint16_t cfg_size_in_items;
	uint16_t cfg_idx_offset;
	uint16_t cfg_val_offset;
};

/* I2C-specific SE registers */
#define SE_I2C_TX_TRANS_LEN		0x26c
#define SE_I2C_RX_TRANS_LEN		0x270
#define SE_I2C_SCL_COUNTERS		0x278
#define TRANS_LEN_MSK			GENMASK_32(23, 0)

#define HIGH_COUNTER_SHFT		20
#define HIGH_COUNTER_MSK		GENMASK_32(29, 20)
#define LOW_COUNTER_SHFT		10
#define LOW_COUNTER_MSK			GENMASK_32(19, 10)
#define CYCLE_COUNTER_MSK		GENMASK_32(9, 0)

/* M_CMD opcodes for the I2C protocol */
#define I2C_WRITE			1
#define I2C_READ			2

/* M_CMD params for the I2C protocol */
#define SLV_ADDR_SHFT			9
#define SLV_ADDR_MSK			GENMASK_32(15, 9)

#define QUP_I2C_M_CMD_TIMEOUT_US	1000000
#define QUP_I2C_CANCEL_TIMEOUT_US	1000000
#define QUP_I2C_ABORT_TIMEOUT_US	1000000

/*
 * SCL timing per target bus speed, straight from mainline's
 * geni_i2c_clk_map_19p2mhz[]/geni_i2c_clk_map_32mhz[] tables
 * (drivers/i2c/busses/i2c-qcom-geni.c): the SE runs its M clock off a
 * fixed, never-rate-switched source (19.2 MHz XO or a 32 MHz source),
 * and every I2C bus speed is reached purely via GENI_SER_M_CLK_CFG's
 * clk_div and SE_I2C_SCL_COUNTERS -- no DFS/set_rate involved.
 *
 *   t_high = (t_high_cnt * clk_div) / source_clock
 *   t_low  = (t_low_cnt  * clk_div) / source_clock
 *   t_cycle = (t_cycle_cnt * clk_div) / source_clock
 */
struct qup_i2c_clk_fld {
	unsigned int speed_hz;
	uint8_t clk_div;
	uint8_t t_high_cnt;
	uint8_t t_low_cnt;
	uint8_t t_cycle_cnt;
};

static const struct qup_i2c_clk_fld qup_i2c_clk_map_19p2mhz[] = {
	{ 100000, 7, 10, 12, 26 },
	{ 400000, 2,  5, 11, 22 },
	{ 1000000, 1, 2,  8, 18 },
};

static const struct qup_i2c_clk_fld qup_i2c_clk_map_32mhz[] = {
	{ 100000, 8, 14, 18, 38 },
	{ 400000, 4,  3,  9, 19 },
	{ 1000000, 2,  3,  5, 15 },
};

/*
 * Picks the SCL-timing row for qi->speed_hz.
 *
 * The counter values are only valid for the SE clock's actual rate, so
 * that rate selects the table -- exactly as mainline does
 * (geni_i2c_clk_map_idx() keys off clk_get_rate(gi2c->se.clk), not off a
 * config value). Trusting the platform-cfg number instead would silently
 * mis-time SCL by whatever ratio the real rate differs by, so read it
 * back from the clock and only fall back to the cfg value if the clock
 * framework cannot report one.
 */
static const struct qup_i2c_clk_fld *
qup_i2c_find_clk_fld(struct qup_i2c_data *qi)
{
	const struct qup_i2c_clk_fld *tbl = NULL;
	unsigned long src_hz = 0;
	size_t n = 0;
	size_t i = 0;

	if (qi->se_clk)
		src_hz = clk_get_rate(qi->se_clk);
	if (!src_hz)
		src_hz = qi->clk_hz;

	if (src_hz != qi->clk_hz)
		DMSG("QUP I2C %u: SE clock is %lu Hz, platform cfg says %u Hz -- using %lu Hz for SCL timing",
		     qi->id, src_hz, qi->clk_hz, src_hz);

	if (src_hz == 32000000) {
		tbl = qup_i2c_clk_map_32mhz;
		n = ARRAY_SIZE(qup_i2c_clk_map_32mhz);
	} else if (src_hz == 19200000) {
		tbl = qup_i2c_clk_map_19p2mhz;
		n = ARRAY_SIZE(qup_i2c_clk_map_19p2mhz);
	} else {
		/*
		 * No counter table for this source rate: programming another
		 * table's values would put SCL out of I2C spec, so refuse.
		 */
		EMSG("QUP I2C %u: no SCL timing table for a %lu Hz SE clock (need 19.2 or 32 MHz)",
		     qi->id, src_hz);
		return NULL;
	}

	for (i = 0; i < n; i++)
		if (tbl[i].speed_hz == qi->speed_hz)
			return &tbl[i];

	return NULL;
}

static struct qup_i2c_data *to_qup_i2c(struct i2c_ctrl *ctrl)
{
	return container_of(ctrl, struct qup_i2c_data, ctrl);
}

/* Clock gating helpers, defined below but used by read()/write(). */
static TEE_Result qup_i2c_clk_enable(struct qup_i2c_data *qi);
static void qup_i2c_clk_disable(struct qup_i2c_data *qi);

/*
 * Outcome of qup_i2c_poll_m_cmd(): DONE is success; ERROR means the SE
 * itself reported a bus/command error (NACK, arbitration lost, bus
 * protocol error, ...) -- the SE has already cleanly terminated the
 * command sequence, so no cancel/abort is needed, unlike TIMEOUT.
 */
enum qup_i2c_poll_result {
	QUP_I2C_POLL_DONE,
	QUP_I2C_POLL_ERROR,
	QUP_I2C_POLL_TIMEOUT,
};

/*
 * Polling-mode M_CMD completion wait, defined below (it services TX/RX
 * FIFO via qup_i2c_fifo_fill_tx(), defined further down) but used by
 * geni_i2c_cancel_and_abort_m_cmd() above that.
 */
static enum qup_i2c_poll_result
qup_i2c_poll_m_cmd(struct qup_i2c_data *qi, uint32_t done_bit,
		    bool check_rem_bytes, unsigned int timeout_us);

static void geni_i2c_setup_m_cmd(struct qup_i2c_data *qi, uint32_t cmd,
				  uint32_t params)
{
	uint32_t m_cmd = (cmd << M_OPCODE_SHFT) | params;

	io_write32(qi->base + SE_GENI_M_CMD0, m_cmd);
}

/*
 * The active M_CMD did not complete in time: request a cancel, and
 * escalate to an abort if the cancel itself does not complete. This
 * leaves the SE in a clean state before the caller reports failure.
 */
static void geni_i2c_cancel_and_abort_m_cmd(struct qup_i2c_data *qi)
{
	io_write32(qi->base + SE_GENI_M_CMD_CTRL_REG, M_GENI_CMD_CANCEL);
	if (qup_i2c_poll_m_cmd(qi, M_CMD_CANCEL_EN, false,
			       QUP_I2C_CANCEL_TIMEOUT_US) == QUP_I2C_POLL_DONE)
		return;

	EMSG("QUP I2C: cancel timed out, trying abort");

	io_write32(qi->base + SE_GENI_M_CMD_CTRL_REG, M_GENI_CMD_ABORT);
	if (qup_i2c_poll_m_cmd(qi, M_CMD_ABORT_EN, false,
			       QUP_I2C_ABORT_TIMEOUT_US) != QUP_I2C_POLL_DONE)
		EMSG("QUP I2C: abort timed out, SE may require re-init");
}

static void geni_get_fifo_depth_width(struct qup_i2c_data *qi)
{
	uint32_t val = io_read32(qi->base + SE_HW_PARAM_0);

	qi->tx_fifo_depth = (val & TX_FIFO_DEPTH_MSK) >> TX_FIFO_DEPTH_SHFT;
	qi->fifo_width_bits = (val & TX_FIFO_WIDTH_MSK) >> TX_FIFO_WIDTH_SHFT;

	DMSG("QUP I2C %u: HW_PARAM_0=%#" PRIx32 " tx_fifo_depth=%u fifo_width=%u",
	     qi->id, val, qi->tx_fifo_depth, qi->fifo_width_bits);
}

/*
 * Builds the byte-packing vectors that tell the SE how to pack 8-bit I2C
 * words into 32-bit FIFO words (and unpack them back out on receive) --
 * same generic SE hardware block as qcom_geni_spi.c's
 * geni_config_packing(), and the same vector construction as mainline's
 * geni_se_config_packing() (drivers/soc/qcom/qcom-geni-se.c).
 *
 * @pack_words is how many protocol words share one 32-bit FIFO entry, and
 * MUST match the number of bytes qup_i2c_fifo_fill_tx()/
 * qup_i2c_fifo_drain_rx() move per FIFO access: the vector count
 * ((ceil_bpw * pack_words) / 8) is exactly what the SE uses to decide how
 * many bytes to (un)pack per entry. Mainline I2C passes 4
 * (PACKING_BYTES_PW) for bpw=8, i.e. a full 32-bit FIFO word per access.
 */
static void geni_config_packing(struct qup_i2c_data *qi, unsigned int bpw,
				 unsigned int pack_words)
{
	uint32_t cfg[NUM_PACKING_VECTORS] = { 0 };
	int idx_start = bpw - 1;
	int idx = idx_start;
	int idx_delta = -8;
	int temp_bpw = bpw;
	int ceil_bpw = ROUNDUP(bpw, 8);
	int iter = (ceil_bpw * pack_words) / 8;
	int i = 0;
	int len = 0;

	if (iter <= 0 || iter > NUM_PACKING_VECTORS) {
		EMSG("QUP I2C: invalid bits_per_word %u / pack_words %u", bpw,
		     pack_words);
		return;
	}

	for (i = 0; i < iter; i++) {
		len = MIN(temp_bpw, 8) - 1;
		cfg[i] = idx << PACKING_START_SHIFT;
		cfg[i] |= 1U << PACKING_DIR_SHIFT;
		cfg[i] |= len << PACKING_LEN_SHIFT;

		if (temp_bpw <= 8) {
			idx = ((i + 1) * 8) + idx_start;
			temp_bpw = bpw;
		} else {
			idx = idx + idx_delta;
			temp_bpw = temp_bpw - 8;
		}
	}
	cfg[iter - 1] |= PACKING_STOP_BIT;

	io_write32(qi->base + SE_GENI_TX_PACKING_CFG0,
		   cfg[0] | (cfg[1] << PACKING_VECTOR_SHIFT));
	io_write32(qi->base + SE_GENI_TX_PACKING_CFG1,
		   cfg[2] | (cfg[3] << PACKING_VECTOR_SHIFT));
	io_write32(qi->base + SE_GENI_RX_PACKING_CFG0,
		   cfg[0] | (cfg[1] << PACKING_VECTOR_SHIFT));
	io_write32(qi->base + SE_GENI_RX_PACKING_CFG1,
		   cfg[2] | (cfg[3] << PACKING_VECTOR_SHIFT));

	io_write32(qi->base + SE_GENI_BYTE_GRAN, bpw / 16);
}

/*
 * Configures the SE for qi->speed_hz: byte packing, FIFO watermarks, the
 * M clock divider, and the I2C SCL high/low/cycle counters. Idempotent
 * per speed_hz -- skips reprogramming if already configured for the
 * current qi->speed_hz.
 */
static TEE_Result qup_i2c_configure_bus(struct qup_i2c_data *qi)
{
	const struct qup_i2c_clk_fld *fld = NULL;
	uint32_t val = 0;
	TEE_Result res = TEE_SUCCESS;

	if (qi->bus_configured && qi->configured_speed_hz == qi->speed_hz)
		return TEE_SUCCESS;

	fld = qup_i2c_find_clk_fld(qi);
	if (!fld) {
		/*
		 * Either speed_hz is not one of the supported bus speeds or
		 * the SE clock runs at a rate with no counter table;
		 * qup_i2c_find_clk_fld() has already said which.
		 */
		EMSG("QUP I2C %u: no SCL timing for speed_hz=%u (supported: 100k/400k/1M)",
		     qi->id, qi->speed_hz);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	geni_get_fifo_depth_width(qi);

	/*
	 * QUP_I2C_PACKING_BYTES_PW assumes a 32-bit FIFO entry (as mainline's
	 * PACKING_BYTES_PW does). Flag a mismatch rather than silently
	 * (un)packing the wrong number of bytes per entry -- that is exactly
	 * the class of bug the packing/FIFO-loop pairing has to avoid.
	 */
	if (qi->fifo_width_bits !=
	    QUP_I2C_PACKING_BYTES_PW * QUP_I2C_BITS_PER_WORD)
		EMSG("QUP I2C %u: FIFO is %u bits wide, but packing assumes %u -- check QUP_I2C_PACKING_BYTES_PW",
		     qi->id, qi->fifo_width_bits,
		     QUP_I2C_PACKING_BYTES_PW * QUP_I2C_BITS_PER_WORD);

	/*
	 * Fire the RX watermark 3 words before the FIFO is full, and the
	 * ready-for-receive threshold 2 words before full, leaving headroom
	 * to drain the FIFO without overrun.
	 *
	 * tx_fifo_depth comes straight from SE_HW_PARAM_0, so treat an
	 * implausible value as a hardware/clock problem and fail cleanly
	 * rather than underflowing the watermark registers. (An assert()
	 * would panic the whole TEE in a debug build and, worse, vanish
	 * under NDEBUG leaving only the underflow.)
	 */
	if (qi->tx_fifo_depth < 4) {
		EMSG("QUP I2C %u: implausible tx_fifo_depth=%u from SE_HW_PARAM_0 (clock gated?)",
		     qi->id, qi->tx_fifo_depth);
		return TEE_ERROR_BAD_STATE;
	}
	io_write32(qi->base + SE_GENI_RX_WATERMARK_REG, qi->tx_fifo_depth - 3);
	io_write32(qi->base + SE_GENI_RX_RFR_WATERMARK_REG,
		   qi->tx_fifo_depth - 2);

	val = io_read32(qi->base + SE_GENI_DMA_MODE_EN);
	val &= ~GENI_DMA_MODE_EN;
	io_write32(qi->base + SE_GENI_DMA_MODE_EN, val);

	geni_config_packing(qi, QUP_I2C_BITS_PER_WORD,
			     QUP_I2C_PACKING_BYTES_PW);

	/* No DFS for I2C: the SE clock is a fixed, never-rate-switched
	 * source (qi->clk_hz), so SE_GENI_CLK_SEL always selects bank 0.
	 */
	//io_write32(qi->base + SE_GENI_CLK_SEL, 1);
	res = clk_set_rate(qi->se_clk, qi->clk_hz);
	if (res)
		return res;

	io_write32(qi->base + GENI_SER_M_CLK_CFG,
		   (((uint32_t)fld->clk_div << CLK_DIV_SHFT) & CLK_DIV_MSK) |
		   SER_CLK_EN);

	val = ((uint32_t)fld->t_high_cnt << HIGH_COUNTER_SHFT) &
	      HIGH_COUNTER_MSK;
	val |= ((uint32_t)fld->t_low_cnt << LOW_COUNTER_SHFT) &
	       LOW_COUNTER_MSK;
	val |= (uint32_t)fld->t_cycle_cnt & CYCLE_COUNTER_MSK;
	io_write32(qi->base + SE_I2C_SCL_COUNTERS, val);

	io_write32(qi->base + SE_GENI_M_IRQ_EN, QUP_I2C_M_IRQ_EN_MASK);

	DMSG("QUP I2C %u: configured speed=%u Hz clk_div=%u high=%u low=%u cycle=%u",
	     qi->id, qi->speed_hz, fld->clk_div, fld->t_high_cnt,
	     fld->t_low_cnt, fld->t_cycle_cnt);

	qi->configured_speed_hz = qi->speed_hz;
	qi->bus_configured = true;

	return TEE_SUCCESS;
}

/*
 * Fills the TX FIFO with whatever currently fits. Used both for the
 * initial fill (before the first watermark can possibly latch) and from
 * qup_i2c_poll_m_cmd()'s polling loop. I2C packs up to 4 bytes (a full
 * word) per FIFO entry, always LSB-first.
 */
static void qup_i2c_fifo_fill_tx(struct qup_i2c_data *qi)
{
	uint32_t tx_wc = io_read32(qi->base + SE_GENI_TX_FIFO_STATUS) &
			  TX_FIFO_WC_MSK;
	uint32_t tx_free_words = 0;

	if (qi->tx_fifo_depth > tx_wc)
		tx_free_words = qi->tx_fifo_depth - tx_wc;

	while (tx_free_words && qi->tx_rem_bytes) {
		uint32_t word = 0;
		unsigned int j = 0;

		for (j = 0; j < QUP_I2C_PACKING_BYTES_PW && qi->tx_rem_bytes;
		     j++, qi->tx_rem_bytes--) {
			uint8_t b = qi->tx_buf ? *qi->tx_buf++ : 0;

			word |= (uint32_t)b << (8 * j);
		}
		io_write32(qi->base + SE_GENI_TX_FIFOn, word);
		tx_free_words--;
	}

	if (!qi->tx_rem_bytes)
		io_write32(qi->base + SE_GENI_TX_WATERMARK_REG, 0);
}

/*
 * Drains whatever is currently in the RX FIFO into qi->rx_buf. Used by
 * qup_i2c_poll_m_cmd()'s polling loop.
 */
static void qup_i2c_fifo_drain_rx(struct qup_i2c_data *qi)
{
	uint32_t rx_wc = io_read32(qi->base + SE_GENI_RX_FIFO_STATUS) &
			  RX_FIFO_WC_MSK;

	while (rx_wc && qi->rx_rem_bytes) {
		uint32_t word = io_read32(qi->base + SE_GENI_RX_FIFOn);
		unsigned int j = 0;

		for (j = 0; j < QUP_I2C_PACKING_BYTES_PW && qi->rx_rem_bytes;
		     j++, qi->rx_rem_bytes--) {
			if (qi->rx_buf)
				*qi->rx_buf++ = (word >> (8 * j)) & 0xff;
		}
		rx_wc--;
	}
}

/*
 * Logs which specific I2C bus/command error(s) are set in m_irq, matching
 * mainline i2c-qcom-geni.c's geni_i2c_err() decode table.
 */
static void qup_i2c_log_errors(struct qup_i2c_data *qi, uint32_t m_irq)
{
	if (m_irq & M_GP_IRQ_1_EN)
		EMSG("QUP I2C %u: NACK -- slave unresponsive, check its power/address/pull-ups",
		     qi->id);
	if (m_irq & M_GP_IRQ_3_EN)
		EMSG("QUP I2C %u: bus protocol error (noisy/unexpected start or stop)",
		     qi->id);
	if (m_irq & M_GP_IRQ_4_EN)
		EMSG("QUP I2C %u: arbitration lost, clock line undriveable",
		     qi->id);
	if (m_irq & M_CMD_OVERRUN_EN)
		EMSG("QUP I2C %u: command overrun", qi->id);
	if (m_irq & M_ILLEGAL_CMD_EN)
		EMSG("QUP I2C %u: illegal command", qi->id);
	if (m_irq & M_CMD_FAILURE_EN)
		EMSG("QUP I2C %u: command failure", qi->id);
	if (m_irq & M_GP_IRQ_0_EN)
		EMSG("QUP I2C %u: unknown bus error (GP_IRQ0)", qi->id);
	if (m_irq & M_GP_IRQ_2_EN)
		EMSG("QUP I2C %u: unknown bus error (GP_IRQ2)", qi->id);
	if (m_irq & M_GP_IRQ_5_EN)
		EMSG("QUP I2C %u: unknown bus error (GP_IRQ5)", qi->id);
}

/*
 * Polls M_IRQ_STATUS directly from the calling thread (no interrupt
 * handler -- same polling-only design as qcom_geni_spi.c's
 * qup_spi_poll_m_cmd()) until done_bit is observed (with the transfer
 * fully drained/filled too, when check_rem_bytes is set), an I2C bus/
 * command error is seen (QUP_I2C_M_IRQ_ERR_MASK), or timeout_us elapses.
 * Services any RX/TX FIFO watermark and clears every bit it observes.
 */
static enum qup_i2c_poll_result
qup_i2c_poll_m_cmd(struct qup_i2c_data *qi, uint32_t done_bit,
		    bool check_rem_bytes, unsigned int timeout_us)
{
	uint64_t timeout_ref = timeout_init_us(timeout_us);

	while (!timeout_elapsed(timeout_ref)) {
		uint32_t m_irq = io_read32(qi->base + SE_GENI_M_IRQ_STATUS);

		if (!m_irq)
			continue;

		if ((m_irq & M_RX_FIFO_WATERMARK_EN) ||
		    (m_irq & M_RX_FIFO_LAST_EN))
			qup_i2c_fifo_drain_rx(qi);
		if (m_irq & M_TX_FIFO_WATERMARK_EN)
			qup_i2c_fifo_fill_tx(qi);

		if (m_irq & QUP_I2C_M_IRQ_ERR_MASK) {
			qup_i2c_log_errors(qi, m_irq);
			/* The SE has already terminated the command
			 * sequence; just stop feeding the TX FIFO.
			 */
			io_write32(qi->base + SE_GENI_TX_WATERMARK_REG, 0);
			io_write32(qi->base + SE_GENI_M_IRQ_CLEAR, m_irq);
			return QUP_I2C_POLL_ERROR;
		}

		io_write32(qi->base + SE_GENI_M_IRQ_CLEAR, m_irq);

		if (!(m_irq & done_bit))
			continue;
		if (check_rem_bytes && (qi->tx_rem_bytes || qi->rx_rem_bytes))
			continue;

		return QUP_I2C_POLL_DONE;
	}

	return QUP_I2C_POLL_TIMEOUT;
}

/* True if this SE's GENI firmware is already loaded (e.g. by an earlier
 * boot stage), identified by GENI_FW_REVISION_RO already reporting the
 * I2C protocol.
 */
static bool qup_i2c_fw_already_loaded(struct qup_i2c_data *qi)
{
	uint32_t protocol = io_read32(qi->base + GENI_FW_REVISION_RO) >>
			     FW_REV_PROTOCOL_SHFT;

	return protocol == QUP_FW_SERIAL_PROTOCOL_I2C;
}

/*
 * Validates that hdr's magic/version/protocol match this image, and that
 * its three sub-array offsets and item counts all stay within
 * qi->fw_image_size, mirroring u-boot qcom_geni.c's valid_seg_size()
 * (see qcom_geni_spi.c's qup_spi_fw_hdr_valid(), same logic here).
 */
static bool qup_i2c_fw_hdr_valid(struct qup_i2c_data *qi,
				  const struct elf_se_hdr *hdr)
{
	if (hdr->magic != QUP_FW_MAGIC || hdr->version != QUP_FW_HDR_VERSION ||
	    hdr->serial_protocol != QUP_FW_SERIAL_PROTOCOL_I2C) {
		EMSG("QUP I2C %u: bad fw header: magic=%#" PRIx32 " version=%" PRIu32 " protocol=%u",
		     qi->id, hdr->magic, hdr->version,
		     (unsigned int)hdr->serial_protocol);
		return false;
	}

	if (qi->fw_image_size < hdr->fw_offset +
	    hdr->fw_size_in_items * sizeof(uint32_t) ||
	    qi->fw_image_size < hdr->cfg_val_offset +
	    hdr->cfg_size_in_items * sizeof(uint32_t) ||
	    qi->fw_image_size < hdr->cfg_idx_offset +
	    hdr->cfg_size_in_items * sizeof(uint8_t)) {
		EMSG("QUP I2C %u: fw image too small for header's offsets/sizes",
		     qi->id);
		return false;
	}

	return true;
}

/*
 * Loads qi->fw_image into the SE's GENI RAM and programs its config
 * register table, following u-boot qcom_geni.c's load_se_firmware() --
 * identical sequence to qcom_geni_spi.c's qup_spi_load_fw(), just with
 * the I2C protocol value. Runs once per qi: gated by qi->fw_loaded, and
 * a no-op if an earlier boot stage already loaded the firmware or if
 * this SE has no fw_image configured.
 */
static TEE_Result qup_i2c_load_fw(struct qup_i2c_data *qi)
{
	struct elf_se_hdr hdr = { };
	const uint8_t *fw_val_base = NULL;
	const uint8_t *cfg_val_base = NULL;
	const uint8_t *cfg_idx_arr = NULL;
	uint32_t reg_value = 0;
	uint32_t rx_fifo_width = 0;
	unsigned int i = 0;
	TEE_Result res = TEE_SUCCESS;

	if (qi->fw_loaded)
		return TEE_SUCCESS;

	if (!qi->fw_image) {
		DMSG("QUP I2C %u: no fw_image configured, skipping fw load",
		     qi->id);
		qi->fw_loaded = true;
		return TEE_SUCCESS;
	}

	/*
	 * GENI_FW_REVISION_RO and every other register touched below is on
	 * the wrapper AHB clock domain, so it must be ungated before the
	 * "already loaded" check can be trusted, not just before the load
	 * itself.
	 */
	res = qup_i2c_clk_enable(qi);
	if (res)
		return res;

	if (qup_i2c_fw_already_loaded(qi)) {
		DMSG("QUP I2C %u: fw already loaded", qi->id);
		qi->fw_loaded = true;
		qup_i2c_clk_disable(qi);
		return TEE_SUCCESS;
	}

	/*
	 * fw_image is a plain byte array with no alignment guarantee, and
	 * this target enforces strict alignment (-mstrict-align): copy the
	 * header out by value instead of casting/dereferencing it in place.
	 */
	if (qi->fw_image_size < sizeof(hdr)) {
		EMSG("QUP I2C %u: fw image smaller than its header", qi->id);
		qup_i2c_clk_disable(qi);
		return TEE_ERROR_BAD_PARAMETERS;
	}
	memcpy(&hdr, qi->fw_image, sizeof(hdr));

	if (!qup_i2c_fw_hdr_valid(qi, &hdr)) {
		qup_i2c_clk_disable(qi);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	fw_val_base = qi->fw_image + hdr.fw_offset;
	cfg_idx_arr = qi->fw_image + hdr.cfg_idx_offset;
	cfg_val_base = qi->fw_image + hdr.cfg_val_offset;

	/*
	 * Disable high-priority interrupts until current low-priority ones
	 * are handled, and hand CGC control of the wrapper to hardware.
	 */
	io_setbits32(qi->common_base + QUPV3_COMMON_CFG,
		     FAST_SWITCH_TO_HIGH_DISABLE);
	io_setbits32(qi->common_base + QUPV3_SE_AHB_M_CFG, AHB_M_CLK_CGC_ON);
	io_setbits32(qi->common_base + QUPV3_COMMON_CGC_CTRL,
		     COMMON_CSR_SLV_CLK_CGC_ON);

	/* Allow the SE to drive its output pads according to hardware value. */
	io_write32(qi->base + GENI_OUTPUT_CTRL, 0x0);

	/* Ungate SCLK/HCLK to program the GENI RAM. */
	io_setbits32(qi->base + GENI_CGC_CTRL, GENI_CGC_CTRL_PROG_RAM_MSK);
	io_write32(qi->base + SE_GENI_CLK_CTRL, 0x0);
	io_clrbits32(qi->base + GENI_CGC_CTRL, GENI_CGC_CTRL_PROG_RAM_MSK);

	/* Enable clocks for the DMA CSR, TX and RX. */
	io_setbits32(qi->base + SE_DMA_GENERAL_CFG,
		     SE_DMA_GENERAL_CFG_CGC_ON_MSK);

	/*
	 * Let hardware control CGC by default: without this the command
	 * FSM's own clocks (SCLK/TX/RX/AHB "always on" bits) stay gated, so
	 * M_CMD writes (and even cancel/abort) never get acknowledged.
	 */
	io_write32(qi->base + GENI_CGC_CTRL, GENI_CGC_CTRL_DEFAULT_EN_MSK);

	/* Version of the configuration-register part of the firmware. */
	io_write32(qi->base + GENI_INIT_CFG_REVISION, hdr.cfg_version);
	io_write32(qi->base + GENI_S_INIT_CFG_REVISION, hdr.cfg_version);

	/* Configure the GENI primitive table. */
	for (i = 0; i < hdr.cfg_size_in_items; i++) {
		uint32_t cfg_val = 0;

		memcpy(&cfg_val, cfg_val_base + i * sizeof(cfg_val),
		       sizeof(cfg_val));
		io_write32(qi->base + GENI_CFG_REG0 +
			   cfg_idx_arr[i] * sizeof(uint32_t), cfg_val);
	}

	/* RX_RFR_WATERMARK: assert 2 words before the RX FIFO is full. */
	rx_fifo_width = (io_read32(qi->base + SE_HW_PARAM_1) &
			  RX_FIFO_WIDTH_MSK) >> RX_FIFO_WIDTH_SHFT;
	if (rx_fifo_width >= 2)
		io_write32(qi->base + SE_GENI_RX_RFR_WATERMARK_REG,
			   rx_fifo_width - 2);
	else
		EMSG("QUP I2C %u: implausible rx_fifo_width=%" PRIu32 ", leaving RFR watermark at default",
		     qi->id, rx_fifo_width);

	/* Let hardware drive the output pads again. */
	io_setbits32(qi->base + GENI_OUTPUT_CTRL, 0x7f);

	/* FIFO mode: no DMA, no GSI events. */
	io_clrbits32(qi->base + SE_GENI_DMA_MODE_EN, GENI_DMA_MODE_EN);
	io_write32(qi->base + SE_IRQ_EN, SE_IRQ_EN_MSK);
	io_write32(qi->base + SE_GSI_EVENT_EN, 0x0);

	/*
	 * Bring-up default IRQ-status enables. M_IRQ_EN is reprogrammed by
	 * qup_i2c_configure_bus(); the S_/DMA ones are left as set here
	 * (this driver is master-side FIFO mode only). Nothing is delivered
	 * to the GIC -- these only make the bits observable in the *_STATUS
	 * registers that qup_i2c_poll_m_cmd() polls.
	 */
	io_write32(qi->base + SE_GENI_M_IRQ_EN, QUP_I2C_M_IRQ_EN_MASK);
	reg_value = S_CMD_OVERRUN_EN | S_ILLEGAL_CMD_EN | S_CMD_CANCEL_EN |
		    S_CMD_ABORT_EN | S_GP_IRQ_0_EN | S_GP_IRQ_1_EN |
		    S_GP_IRQ_2_EN | S_GP_IRQ_3_EN | S_RX_FIFO_WR_ERR_EN |
		    S_RX_FIFO_RD_ERR_EN;
	io_write32(qi->base + SE_GENI_S_IRQ_EN, reg_value);
	io_write32(qi->base + SE_DMA_TX_IRQ_EN_SET,
		   DMA_TX_RESET_DONE_EN_SET | DMA_TX_SBE_EN_SET |
		   DMA_TX_DMA_DONE_EN_SET);
	io_write32(qi->base + SE_DMA_RX_IRQ_EN_SET,
		   DMA_RX_FLUSH_DONE_EN_SET | DMA_RX_RESET_DONE_EN_SET |
		   DMA_RX_SBE_EN_SET | DMA_RX_DMA_DONE_EN_SET);

	/* Program the firmware revision/protocol. */
	reg_value = (hdr.serial_protocol << FW_REV_PROTOCOL_SHFT) |
		    (hdr.fw_version & 0xff);
	io_write32(qi->base + SE_GENI_FW_REVISION, reg_value);
	io_write32(qi->base + SE_S_FW_REVISION, reg_value);

	/* Program the GENI RAM firmware words. */
	for (i = 0; i < hdr.fw_size_in_items; i++) {
		uint32_t fw_val = 0;

		memcpy(&fw_val, fw_val_base + i * sizeof(fw_val),
		       sizeof(fw_val));
		io_write32(qi->base + SE_GENI_CFG_RAMN + i * sizeof(uint32_t),
			   fw_val);
	}

	/* Enable dfs index */
	io_write32(qi->base + SE_GENI_DFS_IF_CFG, SER_DFS_EN);

	/* Put default values on the GENI output pads. */
	io_write32(qi->base + GENI_FORCE_DEFAULT_REG, 0x1);

	/* Toggle SCLK/HCLK once more to latch the RAM contents. */
	io_setbits32(qi->base + GENI_CGC_CTRL, GENI_CGC_CTRL_PROG_RAM_MSK);
	io_setbits32(qi->base + SE_GENI_CLK_CTRL, GENI_CLK_CTRL_SER_CLK_SEL);
	io_clrbits32(qi->base + GENI_CGC_CTRL, GENI_CGC_CTRL_PROG_RAM_MSK);

	/* Enable the SE's DMA interface and its FIFO interface (FIFO mode). */
	io_setbits32(qi->base + SE_DMA_IF_EN, DMA_IF_EN_DMA_IF_EN);
	io_clrbits32(qi->base + SE_FIFO_IF_DISABLE, FIFO_IF_DISABLE);

	qup_i2c_clk_disable(qi);

	qi->fw_loaded = true;
	IMSG("QUP I2C %u: fw loaded (protocol=%u fw_version=%#x cfg_version=%#x)",
	     qi->id, (unsigned int)hdr.serial_protocol,
	     (unsigned int)hdr.fw_version, (unsigned int)hdr.cfg_version);

	return TEE_SUCCESS;
}

/*
 * Runs one I2C_WRITE or I2C_READ M_CMD against i2c_dev->addr. Common
 * body for qup_i2c_read()/qup_i2c_write(): sets up TRANS_LEN, issues the
 * M_CMD with the slave address in m_param, primes the TX FIFO for a
 * write, and polls to completion.
 */
static TEE_Result qup_i2c_xfer(struct qup_i2c_data *qi, uint16_t addr,
				bool is_read, const uint8_t *wdat,
				uint8_t *rdat, size_t len)
{
	uint32_t m_param = ((uint32_t)addr << SLV_ADDR_SHFT) & SLV_ADDR_MSK;
	enum qup_i2c_poll_result poll_res = QUP_I2C_POLL_TIMEOUT;
	uint32_t exceptions = 0;
	TEE_Result res = TEE_SUCCESS;

	if (!len || len > TRANS_LEN_MSK)
		return TEE_ERROR_BAD_PARAMETERS;

	/*
	 * A NULL buffer with a non-zero length is a caller bug: the FIFO
	 * helpers tolerate it (they discard RX / shift out zeros) but that
	 * would silently clock a bogus transaction onto a shared bus.
	 */
	if (is_read ? !rdat : !wdat)
		return TEE_ERROR_BAD_PARAMETERS;

	res = qup_i2c_clk_enable(qi);
	if (res)
		return res;

	res = qup_i2c_configure_bus(qi);
	if (res) {
		qup_i2c_clk_disable(qi);
		return res;
	}

	io_write32(qi->base + (is_read ? SE_I2C_RX_TRANS_LEN :
					  SE_I2C_TX_TRANS_LEN),
		   len & TRANS_LEN_MSK);

	exceptions = cpu_spin_lock_xsave(&qi->lock);
	qi->tx_buf = is_read ? NULL : wdat;
	qi->rx_buf = is_read ? rdat : NULL;
	qi->tx_rem_bytes = is_read ? 0 : len;
	qi->rx_rem_bytes = is_read ? len : 0;

	geni_i2c_setup_m_cmd(qi, is_read ? I2C_READ : I2C_WRITE, m_param);
	if (!is_read) {
		/*
		 * Arm the watermark and do the initial fill directly:
		 * M_IRQ_STATUS has not latched yet, so the polling loop
		 * would see nothing to do on its first iteration otherwise.
		 */
		io_write32(qi->base + SE_GENI_TX_WATERMARK_REG, 1);
		qup_i2c_fifo_fill_tx(qi);
	}
	cpu_spin_unlock_xrestore(&qi->lock, exceptions);

	poll_res = qup_i2c_poll_m_cmd(qi, M_CMD_DONE_EN, true,
				       QUP_I2C_M_CMD_TIMEOUT_US);

	if (poll_res != QUP_I2C_POLL_DONE) {
		if (poll_res == QUP_I2C_POLL_TIMEOUT)
			EMSG("QUP I2C %u: %s timed out (addr=%#x len=%zu)",
			     qi->id, is_read ? "read" : "write", addr, len);
		else
			EMSG("QUP I2C %u: %s failed (addr=%#x len=%zu)",
			     qi->id, is_read ? "read" : "write", addr, len);

		/*
		 * A timeout means the command never finished, so it must be
		 * cancelled/aborted. An error may or may not have terminated
		 * it (a NACK, for instance, does not always set M_CMD_DONE),
		 * so ask the hardware instead of guessing: leaving a command
		 * active here would corrupt the next transfer.
		 */
		if (poll_res == QUP_I2C_POLL_TIMEOUT ||
		    (io_read32(qi->base + SE_GENI_STATUS) &
		     GENI_STATUS_M_GENI_CMD_ACTIVE))
			geni_i2c_cancel_and_abort_m_cmd(qi);

		qup_i2c_clk_disable(qi);
		return TEE_ERROR_COMMUNICATION;
	}

	qup_i2c_clk_disable(qi);

	if (qi->tx_rem_bytes || qi->rx_rem_bytes) {
		EMSG("QUP I2C %u: short transfer, tx_rem=%zu rx_rem=%zu",
		     qi->id, qi->tx_rem_bytes, qi->rx_rem_bytes);
		return TEE_ERROR_COMMUNICATION;
	}

	return TEE_SUCCESS;
}

static TEE_Result qup_i2c_read(struct i2c_dev *i2c_dev, uint8_t *buf,
				size_t len)
{
	struct qup_i2c_data *qi = to_qup_i2c(i2c_dev->ctrl);

	return qup_i2c_xfer(qi, i2c_dev->addr, true, NULL, buf, len);
}

static TEE_Result qup_i2c_write(struct i2c_dev *i2c_dev, const uint8_t *buf,
				 size_t len)
{
	struct qup_i2c_data *qi = to_qup_i2c(i2c_dev->ctrl);

	return qup_i2c_xfer(qi, i2c_dev->addr, false, buf, NULL, len);
}

static const struct i2c_ctrl_ops qup_i2c_ops = {
	.read = qup_i2c_read,
	.write = qup_i2c_write,
};
DECLARE_KEEP_PAGER(qup_i2c_ops);

/*
 * Acquire the SE source clock plus the wrapper/common clocks by name.
 * Unlike qcom_geni_spi.c, no DFS/rate control is set up here: I2C runs
 * its SE clock at a fixed, never-rate-switched source rate (qi->clk_hz,
 * matching mainline's model where only clk_get_rate() is ever called on
 * this clock -- see qup_i2c_find_clk_fld()), so it is simply enabled and
 * disabled like the common clocks. All clocks are left gated here;
 * qup_i2c_clk_enable() ungates them once, before each read()/write().
 */
static TEE_Result qup_i2c_clk_setup(struct qup_i2c_data *qi)
{
	TEE_Result res = TEE_SUCCESS;
	unsigned int i = 0;

	if (!qi->se_clock_name) {
		EMSG("QUP I2C: no se clock name for SE %u", qi->id);
		return TEE_ERROR_BAD_STATE;
	}

	res = qcom_clk_get_by_name(qi->se_clock_name, &qi->se_clk);
	if (res) {
		EMSG("QUP I2C: cannot get clock %s: %#" PRIx32,
		     qi->se_clock_name, res);
		return res;
	}

	qi->num_common_clks = 0;
	if (!qi->common_clocks_name)
		return TEE_SUCCESS;

	for (i = 0; qi->common_clocks_name[i]; i++) {
		if (i >= QUP_I2C_MAX_COMMON_CLKS) {
			EMSG("QUP I2C: too many common clocks (max %d)",
			     QUP_I2C_MAX_COMMON_CLKS);
			return TEE_ERROR_BAD_STATE;
		}

		res = qcom_clk_get_by_name(qi->common_clocks_name[i],
					   &qi->common_clks[i]);
		if (res) {
			EMSG("QUP I2C: cannot get clock %s: %#" PRIx32,
			     qi->common_clocks_name[i], res);
			return res;
		}
		qi->num_common_clks++;
	}

	return TEE_SUCCESS;
}

/*
 * Ungate the wrapper/common clocks first, then the SE source clock,
 * ahead of a transfer. Unwinds anything already enabled on failure so
 * the enable/disable refcounts stay balanced.
 */
static TEE_Result qup_i2c_clk_enable(struct qup_i2c_data *qi)
{
	TEE_Result res = TEE_SUCCESS;
	unsigned int i = 0;

	for (i = 0; i < qi->num_common_clks; i++) {
		res = clk_enable(qi->common_clks[i]);
		if (res) {
			EMSG("QUP I2C: clk_enable(%s) failed: %#" PRIx32,
			     qi->common_clocks_name[i], res);
			goto err;
		}
	}

	if (!qi->se_clk) {
		res = TEE_ERROR_BAD_STATE;
		goto err;
	}

	res = clk_enable(qi->se_clk);
	if (res) {
		EMSG("QUP I2C: clk_enable(%s) failed: %#" PRIx32,
		     qi->se_clock_name, res);
		goto err;
	}

	return TEE_SUCCESS;

err:
	while (i-- > 0)
		clk_disable(qi->common_clks[i]);

	return res;
}

/* Gate the SE source clock, then the wrapper/common clocks, once idle. */
static void qup_i2c_clk_disable(struct qup_i2c_data *qi)
{
	unsigned int i = qi->num_common_clks;

	if (qi->se_clk)
		clk_disable(qi->se_clk);

	while (i-- > 0)
		clk_disable(qi->common_clks[i]);
}

/*
 * Mux this SE's pads to the I2C function once, at init: build the TLMM
 * pinctrl state and apply it. apply() claims pin ownership, so it runs a
 * single time here and the pads stay owned/configured for the driver's
 * lifetime. A platform that leaves pin_groups NULL is assumed to set up
 * pinmux elsewhere. Idempotent: gated by qi->pin_state, so a repeat
 * qup_i2c_init() call is a no-op here (re-applying would be rejected by
 * TLMM anyway, since ownership was never released).
 */
static TEE_Result qup_i2c_pinctrl_setup(struct qup_i2c_data *qi)
{
	TEE_Result res = TEE_SUCCESS;

	if (qi->pin_state)
		return TEE_SUCCESS;

	if (!qi->pin_groups || !qi->pin_group_count) {
		DMSG("QUP I2C %u: no pin_groups, skipping pinmux", qi->id);
		return TEE_SUCCESS;
	}

	res = tlmm_make_pin_state(qi->pin_groups, qi->pin_group_count,
				  &qi->pin_state);
	if (res) {
		EMSG("QUP I2C: tlmm_make_pin_state failed: %#" PRIx32, res);
		return res;
	}

	res = tlmm_apply_pin_state(qi->pin_state);
	if (res) {
		EMSG("QUP I2C: tlmm_apply_pin_state failed: %#" PRIx32, res);
		tlmm_free_pin_state(qi->pin_state);
		qi->pin_state = NULL;
		return res;
	}

	IMSG("QUP I2C %u: applied %u pin group(s)", qi->id,
	     qi->pin_group_count);

	return TEE_SUCCESS;
}

bool qup_i2c_get_platform_data(unsigned int qup_i2c_id,
				struct qup_i2c_data *qi)
{
	size_t i = 0;

	for (i = 0; i < qup_i2c_config_count; i++) {
		if (qup_i2c_config[i].id != qup_i2c_id)
			continue;

		qi->base = (vaddr_t)phys_to_virt(qup_i2c_config[i].base,
						  MEM_AREA_IO_SEC,
						  QUP_I2C_REG_SIZE);
		if (!qi->base) {
			EMSG("QUP I2C: failed to map SE %u regs at %#" PRIxPA,
			     qup_i2c_id, qup_i2c_config[i].base);
			return false;
		}

		qi->common_base = (vaddr_t)phys_to_virt(qup_i2c_config[i].common_base,
							 MEM_AREA_IO_SEC,
							 QUP_I2C_COMMON_REG_SIZE);
		if (!qi->common_base) {
			EMSG("QUP I2C: failed to map SE %u wrapper-common regs at %#" PRIxPA,
			     qup_i2c_id, qup_i2c_config[i].common_base);
			return false;
		}

		qi->itr_num = qup_i2c_config[i].itr_num;
		qi->clk_hz = qup_i2c_config[i].clk_hz;
		qi->se_clock_name = qup_i2c_config[i].se_clock_name;
		qi->common_clocks_name = qup_i2c_config[i].common_clocks_name;
		qi->pin_groups = qup_i2c_config[i].pin_groups;
		qi->pin_group_count = qup_i2c_config[i].pin_group_count;
		qi->fw_image = qup_i2c_config[i].fw_image;
		qi->fw_image_size = qup_i2c_config[i].fw_image_size;
		DMSG("QUP I2C %u: base=%#" PRIxVA " irq=%zu se_clk=%s",
		     qup_i2c_id, qi->base, qi->itr_num, qi->se_clock_name);
		return true;
	}

	EMSG("QUP I2C: no platform data for SE id %u", qup_i2c_id);
	return false;
}

TEE_Result qup_i2c_init(struct qup_i2c_data *qi, unsigned int qup_i2c_id)
{
	TEE_Result res = TEE_SUCCESS;

	assert(qi);

	if (!qup_i2c_get_platform_data(qup_i2c_id, qi))
		return TEE_ERROR_ITEM_NOT_FOUND;

	qi->id = qup_i2c_id;
	qi->ctrl.ops = &qup_i2c_ops;
	qi->lock = 0;

	res = qup_i2c_clk_setup(qi);
	if (res)
		return res;

	/*
	 * Load the SE's GENI firmware if it is not already loaded (e.g. by
	 * an earlier boot stage). Idempotent: gated by qi->fw_loaded, so a
	 * repeat qup_i2c_init() call is a no-op here.
	 */
	res = qup_i2c_load_fw(qi);
	if (res)
		return res;

	/* Mux the SE's pads to the I2C function once for the driver's life. */
	res = qup_i2c_pinctrl_setup(qi);
	if (res)
		return res;

	/*
	 * This driver uses polling mode: read()/write() poll M_IRQ_STATUS
	 * from the calling thread (see qup_i2c_poll_m_cmd()) instead of
	 * registering an interrupt handler, so qi->itr_num is not
	 * registered with the GIC here.
	 */
	IMSG("QUP I2C %u: initialized (irq %zu)", qi->id, qi->itr_num);

	return TEE_SUCCESS;
}

TEE_Result qup_i2c_dev_init(struct qup_i2c_data *qi, struct i2c_dev *dev,
			     uint16_t addr)
{
	assert(qi && dev);

	/*
	 * The SE takes the slave address in M_CMD's SLV_ADDR field, which is
	 * 7 bits wide -- there is no 10-bit addressing support here. Reject
	 * anything wider instead of silently masking it, which would target
	 * a different device on a shared bus.
	 */
	if (addr > (SLV_ADDR_MSK >> SLV_ADDR_SHFT)) {
		EMSG("QUP I2C %u: slave address %#x is not a 7-bit address",
		     qi->id, addr);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	dev->ctrl = &qi->ctrl;
	dev->addr = addr;

	return TEE_SUCCESS;
}
