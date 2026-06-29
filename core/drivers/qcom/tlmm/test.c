// SPDX-License-Identifier: BSD-2-Clause
/*
 * Consumer usage examples and tests for the Qcom TLMM GPIO and pinctrl drivers.
 *
 * Compiled only when CFG_QCOM_TLMM_TEST=y.
 *
 * GPIO consumer use cases covered:
 *   1. Output pin  — request, drive high/low, release
 *   2. Input pin   — request, read level, release
 *   3. Active-low  — logical polarity inversion via dt_flags
 *
 * Pinctrl consumer use cases covered:
 *   4. Single pin               — one pin, one group, one config
 *   5. Single group, same config — multiple pins share one func/pull/drive
 *   6. Multiple groups, different config — each group has its own settings
 *
 * Ownership and protection use cases covered:
 *   7. GPIO pin ownership     — exclusive claim, double-request returns BUSY
 *   8. Pinctrl pin ownership  — second apply on owned pins returns BUSY
 *   9. Overlap detection      — pin in two groups: second group rejected
 *  10. Re-apply after free    — ownership released, pin can be reclaimed
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/gpio.h>
#include <drivers/qcom/tlmm/tlmm.h>
#include <initcall.h>
#include <trace.h>
#include <util.h>

#define TEST_GPIO_OUTPUT	U(100)
#define TEST_GPIO_INPUT		U(101)
#define TEST_PINCTRL_PINS	{ U(100), U(101) }

/* =========================================================================
 * Use cases 1-3: GPIO consumers
 *
 * Pattern:
 *   tlmm_request_pin(pin)        -- claim exclusive ownership
 *   gpio_set_direction / value   -- drive or read the pin
 *   tlmm_release_pin(pin)        -- release ownership when done
 * =======================================================================
 */

/*
 * Use case 1: output pin — drive a pad high or low.
 *
 * Request ownership first; gpio_set_direction/value reject operations on
 * unowned pins to prevent silent conflicts between callers.
 */
static void test_gpio_output(struct gpio_chip *chip)
{
	struct gpio g = {
		.chip     = chip,
		.pin      = TEST_GPIO_OUTPUT,
		.dt_flags = 0,
	};
	enum gpio_dir dir = GPIO_DIR_IN;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: output GPIO %u", g.pin);

	res = tlmm_request_pin(TEST_GPIO_OUTPUT);
	if (res) {
		EMSG("TLMM test FAIL: request pin %u returned %#x",
		     TEST_GPIO_OUTPUT, res);
		return;
	}

	gpio_set_direction(&g, GPIO_DIR_OUT);

	dir = gpio_get_direction(&g);
	if (dir != GPIO_DIR_OUT)
		EMSG("TLMM test FAIL: pin %u expected OUT got %d",
		     g.pin, (int)dir);

	gpio_set_value(&g, GPIO_LEVEL_HIGH);
	DMSG("TLMM test: pin %u driven HIGH", g.pin);

	gpio_set_value(&g, GPIO_LEVEL_LOW);
	DMSG("TLMM test: pin %u driven LOW", g.pin);

	tlmm_release_pin(TEST_GPIO_OUTPUT);
}

/*
 * Use case 2: input pin — read the current pad level.
 */
static void test_gpio_input(struct gpio_chip *chip)
{
	struct gpio g = {
		.chip     = chip,
		.pin      = TEST_GPIO_INPUT,
		.dt_flags = 0,
	};
	enum gpio_level level = GPIO_LEVEL_LOW;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: input GPIO %u", g.pin);

	res = tlmm_request_pin(TEST_GPIO_INPUT);
	if (res) {
		EMSG("TLMM test FAIL: request pin %u returned %#x",
		     TEST_GPIO_INPUT, res);
		return;
	}

	gpio_set_direction(&g, GPIO_DIR_IN);
	level = gpio_get_value(&g);

	DMSG("TLMM test: pin %u level = %s",
	     g.pin, level == GPIO_LEVEL_HIGH ? "HIGH" : "LOW");

	tlmm_release_pin(TEST_GPIO_INPUT);
}

/*
 * Use case 3: active-low pin — logical value is inverted by the driver.
 *
 * Setting GPIO_ACTIVE_LOW in dt_flags causes gpio_set_value to invert the
 * logical level before writing the hardware register.  The caller always
 * works in logical (not hardware) polarity.
 */
static void test_gpio_active_low(struct gpio_chip *chip)
{
	struct gpio g = {
		.chip     = chip,
		.pin      = TEST_GPIO_OUTPUT,
		.dt_flags = GPIO_ACTIVE_LOW,
	};
	TEE_Result res = TEE_SUCCESS;

	res = tlmm_request_pin(TEST_GPIO_OUTPUT);
	if (res) {
		EMSG("TLMM test FAIL: request pin %u returned %#x",
		     TEST_GPIO_OUTPUT, res);
		return;
	}

	gpio_set_direction(&g, GPIO_DIR_OUT);

	gpio_set_value(&g, GPIO_LEVEL_HIGH);
	DMSG("TLMM test: ACTIVE_LOW pin %u: logical HIGH → hw LOW", g.pin);

	gpio_set_value(&g, GPIO_LEVEL_LOW);
	DMSG("TLMM test: ACTIVE_LOW pin %u: logical LOW → hw HIGH", g.pin);

	tlmm_release_pin(TEST_GPIO_OUTPUT);
}

/* =========================================================================
 * Use cases 4-6: pinctrl consumers
 *
 * All pinctrl scenarios use the same three-step pattern regardless of how
 * many pins or groups are involved:
 *
 *   tlmm_make_pin_state(groups, N, &state)  -- build config descriptor
 *   tlmm_apply_pin_state(state)             -- claim pins, write GPIO_CFG
 *   tlmm_free_pin_state(state)              -- release ownership, free memory
 *
 * Ownership is handled automatically by apply/free — no explicit
 * tlmm_request_pin() call is needed for pinctrl consumers.
 *
 * Equivalent Qcom internal HAL: HAL_gpio_ConfigPin(pin, func, dir, pull, drive)
 * Our pinctrl writes FUNC_SEL, GPIO_PULL, DRV_STRENGTH to GPIO_CFG.
 * Direction (GPIO_OE) is set separately via gpio_ops when needed.
 * =======================================================================
 */

/*
 * Use case 4: single pin — one pin, one group, one configuration.
 *
 * The simplest pinctrl usage: configure a single pad's mux, pull and drive.
 * The groups array has one element with pin_count=1.
 */
static void test_pinctrl_single_pin(void)
{
	const unsigned int pin = TEST_GPIO_OUTPUT;
	const struct tlmm_pin_group groups[] = {
		{ &pin, 1, 0, TLMM_PULL_NONE, 2, false },
	};
	struct pinctrl_state *state = NULL;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: pinctrl — single pin %u, GPIO mode, no pull, 2mA",
	     pin);

	res = tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
	if (!res)
		res = tlmm_apply_pin_state(state);
	if (res)
		EMSG("TLMM test FAIL: tlmm_apply_pin_state returned %#x", res);
	else
		DMSG("TLMM test: single pin %u config applied", pin);

	if (state)
		tlmm_free_pin_state(state);
}

/*
 * Use case 5: single group — multiple pins sharing the same configuration.
 *
 * Both pins get func=1 (UART), no pull, 6 mA drive.  One group array element
 * covers all pins that need identical settings.
 *
 * Equivalent internal HAL:
 *   HAL_gpio_ConfigPin(100, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_6MA);
 *   HAL_gpio_ConfigPin(101, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_6MA);
 */
static void test_pinctrl_uart(void)
{
	unsigned int pins[] = TEST_PINCTRL_PINS;
	const struct tlmm_pin_group groups[] = {
		{ pins, 2, 1, TLMM_PULL_NONE, 6, false },
	};
	struct pinctrl_state *state = NULL;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: pinctrl — UART function on pins %u, %u",
	     pins[0], pins[1]);

	res = tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
	if (!res)
		res = tlmm_apply_pin_state(state);
	if (res)
		EMSG("TLMM test FAIL: tlmm_apply_pin_state returned %#x", res);
	else
		DMSG("TLMM test: pinctrl UART config applied");

	if (state)
		tlmm_free_pin_state(state);
}

/*
 * Use case 5 (variant): single group, multiple pins, different pull/drive
 * from use case 5.
 *
 * Same single-group pattern, but with pull-up and 2 mA instead of no-pull
 * and 6 mA.  Both pins still share one config — the group has pin_count=2.
 * Demonstrates that pull and drive are independently configurable per group.
 *
 * Equivalent internal HAL:
 *   HAL_gpio_ConfigPin(100, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA);
 *   HAL_gpio_ConfigPin(101, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA);
 */
static void test_pinctrl_gpio_pull(void)
{
	unsigned int pins[] = TEST_PINCTRL_PINS;
	const struct tlmm_pin_group groups[] = {
		{ pins, 2, 0, TLMM_PULL_UP, 2, false },
	};
	struct pinctrl_state *state = NULL;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: pinctrl — GPIO mode, pull-up, 2mA on pins %u, %u",
	     pins[0], pins[1]);

	res = tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
	if (!res)
		res = tlmm_apply_pin_state(state);
	if (res)
		EMSG("TLMM test FAIL: tlmm_apply_pin_state returned %#x", res);
	else
		DMSG("TLMM test: pinctrl GPIO+pull-up config applied");

	if (state)
		tlmm_free_pin_state(state);
}

/* =========================================================================
 * Use cases 7-8: ownership and protection
 * =======================================================================
 */

/*
 * Use case 7: GPIO pin ownership — exclusive access enforcement.
 *
 * A pin can only be owned by one caller at a time.  A second
 * tlmm_request_pin() on an already-owned pin returns TEE_ERROR_BUSY.
 * Once released, the pin can be claimed again.
 */
static void test_pin_ownership(void)
{
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: pin ownership — start");

	/* First request: succeeds */
	res = tlmm_request_pin(TEST_GPIO_OUTPUT);
	if (res)
		EMSG("TLMM test FAIL: first request returned %#x", res);
	else
		DMSG("TLMM test: request pin %u OK", TEST_GPIO_OUTPUT);

	/* Second request on same pin: must return BUSY */
	res = tlmm_request_pin(TEST_GPIO_OUTPUT);
	if (res == TEE_ERROR_BUSY)
		DMSG("TLMM test: double-request correctly rejected (BUSY)");
	else
		EMSG("TLMM test FAIL: double-req returned %#x (expected BUSY)",
		     res);

	tlmm_release_pin(TEST_GPIO_OUTPUT);

	/* After release: can be claimed again */
	res = tlmm_request_pin(TEST_GPIO_OUTPUT);
	if (res)
		EMSG("TLMM test FAIL: post-release request returned %#x", res);
	else
		DMSG("TLMM test: post-release request OK");

	tlmm_release_pin(TEST_GPIO_OUTPUT);

	DMSG("TLMM test: pin ownership — done");
}

/*
 * Use case 8: pinctrl ownership — applying a different config without
 * releasing the first is rejected.
 *
 * tlmm_apply_pin_state() claims pins atomically.  A second apply on the same
 * pins before free returns TEE_ERROR_BUSY — preventing a caller from silently
 * overwriting another consumer's pin configuration.  tlmm_free_pin_state()
 * releases ownership; after that, a new apply succeeds.
 */
static void test_pinctrl_ownership(void)
{
	unsigned int pins[] = TEST_PINCTRL_PINS;
	const struct tlmm_pin_group groups1[] = {
		{ pins, 2, 0, TLMM_PULL_NONE, 2, false },
	};
	const struct tlmm_pin_group groups2[] = {
		{ pins, 2, 1, TLMM_PULL_UP, 4, false },
	};
	struct pinctrl_state *state1 = NULL;
	struct pinctrl_state *state2 = NULL;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: pinctrl ownership — start");

	/* First apply: pins 100,101 claimed */
	res = tlmm_make_pin_state(groups1, 1, &state1);
	if (!res)
		res = tlmm_apply_pin_state(state1);
	if (res) {
		EMSG("TLMM test FAIL: first apply returned %#x", res);
		if (state1)
			tlmm_free_pin_state(state1);
		return;
	}
	DMSG("TLMM test: first apply on pins %u,%u OK", pins[0], pins[1]);

	/* Second apply on same pins without free: must return BUSY */
	res = tlmm_make_pin_state(groups2, 1, &state2);
	if (!res)
		res = tlmm_apply_pin_state(state2);
	if (res == TEE_ERROR_BUSY)
		DMSG("TLMM test: second apply without free correctly rejected");
	else
		EMSG("TLMM test FAIL: second apply returned %#x (exp BUSY)",
		     res);

	/* state2 not applied — free releases no ownership, only memory */
	if (state2)
		tlmm_free_pin_state(state2);
	/* state1 was applied — free releases pins 100,101 */
	tlmm_free_pin_state(state1);

	/* After free: same pins can be configured again */
	state1 = NULL;
	res = tlmm_make_pin_state(groups1, 1, &state1);
	if (!res)
		res = tlmm_apply_pin_state(state1);
	if (res)
		EMSG("TLMM test FAIL: post-free apply returned %#x", res);
	else
		DMSG("TLMM test: post-free apply OK");
	if (state1)
		tlmm_free_pin_state(state1);

	DMSG("TLMM test: pinctrl ownership — done");
}

/* =========================================================================
 * Use cases 6, 9, 10: multi-group pin state
 * =======================================================================
 */

/*
 * Use case 6: multiple groups, multiple pins, DIFFERENT configs per group.
 *
 * When a peripheral's pins need different electrical settings, each
 * requirement becomes one group.  Here two UART TX pins share one config
 * (no pull, 6 mA) and two UART RX pins share another (pull-up, 4 mA).
 * A single tlmm_make_pin_state() + tlmm_apply_pin_state() configures all
 * four pins atomically in one call.
 *
 * Equivalent internal HAL:
 *   HAL_gpio_ConfigPin(100, 1, GPIO_INPUT, GPIO_NO_PULL,  GPIO_6MA); TX0
 *   HAL_gpio_ConfigPin(102, 1, GPIO_INPUT, GPIO_NO_PULL,  GPIO_6MA); TX1
 *   HAL_gpio_ConfigPin(101, 1, GPIO_INPUT, GPIO_PULL_UP,  GPIO_4MA); RX0
 *   HAL_gpio_ConfigPin(103, 1, GPIO_INPUT, GPIO_PULL_UP,  GPIO_4MA); RX1
 */
static void test_pin_state_basic(void)
{
	const unsigned int tx_pins[] = { TEST_GPIO_OUTPUT, U(102) };
	const unsigned int rx_pins[] = { TEST_GPIO_INPUT,  U(103) };
	const struct tlmm_pin_group groups[] = {
		{ tx_pins, 2, 1, TLMM_PULL_NONE, 6, false }, /* TX: no pull */
		{ rx_pins, 2, 1, TLMM_PULL_UP,   4, false }, /* RX: pull-up */
	};
	struct pinctrl_state *state = NULL;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: pin state basic — start");

	res = tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
	if (res) {
		EMSG("TLMM test FAIL: make_pin_state returned %#x", res);
		return;
	}

	res = tlmm_apply_pin_state(state);
	if (res)
		EMSG("TLMM test FAIL: apply_state returned %#x", res);
	else
		DMSG("TLMM test: TX{%u,%u} no-pull/6mA RX{%u,%u} pull-up/4mA",
		     tx_pins[0], tx_pins[1], rx_pins[0], rx_pins[1]);

	tlmm_free_pin_state(state);

	DMSG("TLMM test: pin state basic — done");
}

/*
 * Use case 9: overlap detection — a pin appearing in two groups of the same
 * state is rejected.
 *
 * tlmm_apply_pin_state() claims groups in order.  If group 1 contains a pin
 * already claimed by group 0, it returns TEE_ERROR_BUSY without touching
 * group 1's hardware.  tlmm_free_pin_state() then releases group 0's pins
 * correctly (its applied flag is set) while simply freeing group 1's memory
 * (its applied flag is clear).
 */
static void test_pin_state_overlap(void)
{
	const unsigned int g0_pins[] = { TEST_GPIO_OUTPUT, TEST_GPIO_INPUT };
	/* pin 101 overlaps with g0_pins */
	const unsigned int g1_pins[] = { TEST_GPIO_INPUT, U(102) };
	const struct tlmm_pin_group groups[] = {
		{ g0_pins, 2, 0, TLMM_PULL_NONE, 2, false },
		{ g1_pins, 2, 0, TLMM_PULL_UP,   2, false },
	};
	struct pinctrl_state *state = NULL;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: pin state overlap — start");

	res = tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
	if (res) {
		EMSG("TLMM test FAIL: make_pin_state returned %#x", res);
		return;
	}

	res = tlmm_apply_pin_state(state);
	if (res == TEE_ERROR_BUSY)
		DMSG("TLMM test: overlapping pin correctly rejected (BUSY)");
	else
		EMSG("TLMM test FAIL: overlap apply returned %#x (exp BUSY)",
		     res);

	/* free_state: group 0 releases {100,101}; group 1 frees memory only */
	tlmm_free_pin_state(state);

	/* Confirm pins 100 and 101 are no longer owned */
	res = tlmm_request_pin(TEST_GPIO_OUTPUT);
	if (res)
		EMSG("TLMM test FAIL: pin %u still owned after free (%#x)",
		     TEST_GPIO_OUTPUT, res);
	else
		DMSG("TLMM test: pin %u correctly released", TEST_GPIO_OUTPUT);
	tlmm_release_pin(TEST_GPIO_OUTPUT);

	res = tlmm_request_pin(TEST_GPIO_INPUT);
	if (res)
		EMSG("TLMM test FAIL: pin %u still owned after free (%#x)",
		     TEST_GPIO_INPUT, res);
	else
		DMSG("TLMM test: pin %u correctly released", TEST_GPIO_INPUT);
	tlmm_release_pin(TEST_GPIO_INPUT);

	DMSG("TLMM test: pin state overlap — done");
}

/*
 * Use case 10: re-apply after free — ownership is fully released by
 * tlmm_free_pin_state(), allowing the same pins to be configured again.
 *
 * Demonstrates the complete apply → use → free → re-apply lifecycle that
 * a real driver would follow across PM state transitions (active/sleep).
 */
static void test_pin_state_post_free(void)
{
	const unsigned int pins[] = { TEST_GPIO_OUTPUT, TEST_GPIO_INPUT };
	const struct tlmm_pin_group groups[] = {
		{ pins, 2, 0, TLMM_PULL_NONE, 2, false },
	};
	struct pinctrl_state *state = NULL;
	TEE_Result res = TEE_SUCCESS;

	DMSG("TLMM test: pin state post-free — start");

	res = tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
	if (!res)
		res = tlmm_apply_pin_state(state);
	if (res) {
		EMSG("TLMM test FAIL: first apply returned %#x", res);
		if (state)
			tlmm_free_pin_state(state);
		return;
	}
	DMSG("TLMM test: first state applied OK");
	tlmm_free_pin_state(state);
	state = NULL;

	/* Re-apply: must succeed because free released ownership */
	res = tlmm_make_pin_state(groups, ARRAY_SIZE(groups), &state);
	if (!res)
		res = tlmm_apply_pin_state(state);
	if (res)
		EMSG("TLMM test FAIL: post-free re-apply returned %#x", res);
	else
		DMSG("TLMM test: post-free re-apply OK");
	if (state)
		tlmm_free_pin_state(state);

	DMSG("TLMM test: pin state post-free — done");
}

/* =========================================================================
 * Entry points
 * =======================================================================
 */

static void tlmm_gpio_test(void)
{
	struct gpio_chip *chip = tlmm_get_chip();

	IMSG("TLMM GPIO test: start");
	test_pin_ownership();       /* use case 7 */
	test_gpio_output(chip);     /* use case 1 */
	test_gpio_input(chip);      /* use case 2 */
	test_gpio_active_low(chip); /* use case 3 */
	IMSG("TLMM GPIO test: done");
}

static void tlmm_pinctrl_test(void)
{
	IMSG("TLMM pinctrl test: start");
	test_pinctrl_ownership();   /* use case 8 */
	test_pinctrl_single_pin();  /* use case 4 */
	test_pinctrl_uart();        /* use case 5 */
	test_pinctrl_gpio_pull();   /* use case 5 (variant) */
	test_pin_state_basic();     /* use case 6 */
	test_pin_state_overlap();   /* use case 9 */
	test_pin_state_post_free(); /* use case 10 */
	IMSG("TLMM pinctrl test: done");
}

/*
 * Auto-invoked at driver_init_late — after tlmm_init() has run at
 * driver_init phase, ensuring the hardware mapping and gpio_ops are ready.
 */
static TEE_Result tlmm_run_tests(void)
{
	tlmm_gpio_test();
	tlmm_pinctrl_test();
	return TEE_SUCCESS;
}
driver_init_late(tlmm_run_tests);
