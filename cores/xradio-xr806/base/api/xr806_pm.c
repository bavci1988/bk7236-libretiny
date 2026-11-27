/* Copyright (c) bavci1988 2026-11-23. */

#include <libretiny.h>
#include <sdk_private.h>

#include <xr806_pm.h>

#include "driver/chip/hal_prcm.h"
#include "driver/chip/hal_util.h"
#include "driver/chip/hal_wakeup.h"
#include "net/wlan/wlan.h"
#include "net/wlan/wlan_ext_req.h"
#include "pm/pm.h"

#include <FreeRTOS.h>
#include <task.h>

// A wake line that nothing drives needs time after its pull resistor is switched on.
#define WAKE_PIN_SETTLE_US	100
// pm_enter_mode_timeout(): how long PM waits for the wakelocks to be released
#define PM_WAKELOCK_WAIT_MS 500

// the FIFO error bit stays set while any bad byte is in the FIFO, so it is not counted
#define UART_LINE_ERROR_BITS (UART_FRAME_ERROR_BIT | UART_PARITY_ERROR_BIT | UART_OVERRUN_ERROR_BIT)

static void wake_pin_input(uint8_t pin, const GPIO_InitParam *previous, GPIO_WorkMode mode, bool falling) {
	GPIO_InitParam param = *previous;
	param.mode			 = mode;
	param.pull			 = falling ? GPIO_PULL_UP : GPIO_PULL_DOWN;
	HAL_GPIO_Init(GPIO_PORT_A, (GPIO_Pin)pin, &param);
	HAL_UDelay(WAKE_PIN_SETTLE_US);
}

static bool wake_pin_at_level(uint8_t pin, bool falling) {
	GPIO_PinState level = HAL_GPIO_ReadPin(GPIO_PORT_A, (GPIO_Pin)pin);
	return falling ? level == GPIO_PIN_LOW : level == GPIO_PIN_HIGH;
}

// tasks.c (ROM): the tick count of the next task wake-up
extern volatile TickType_t xNextTaskUnblockTime;

// The FreeRTOS tick stops in standby and PM does not add the time spent there, so lwIP, WLAN and
// other OS timers would fall behind (e.g. a DHCP lease renewed only after it expired).
static void step_tick_over_standby(uint64_t start_us, TickType_t start_ticks) {
	uint64_t elapsed_ms = (HAL_RTC_GetFreeRunTime() - start_us) / 1000;
	vTaskSuspendAll();
	TickType_t now		= xTaskGetTickCount();
	uint64_t counted_ms = (uint64_t)(TickType_t)(now - start_ticks) * portTICK_PERIOD_MS;
	if (elapsed_ms > counted_ms) {
		uint64_t missing = (elapsed_ms - counted_ms) / portTICK_PERIOD_MS;
		// A regular tick must reach portMAX_DELAY: it wakes the tasks due before the tick count
		// wraps, and only the next one switches the delayed task lists.
		TickType_t room = now < portMAX_DELAY - 1 ? portMAX_DELAY - 1 - now : 0;
		vTaskStepTick((TickType_t)(missing < room ? missing : room));
		// the next tick looks for tasks that became due while asleep (also before an idle sleep)
		xNextTaskUnblockTime = xTaskGetTickCount();
	}
	xTaskResumeAll();
}

static bool wake_pin_owned(const GPIO_InitParam *param) {
	// the interrupt handler of another user cannot be saved and restored
	return param->mode == GPIOx_Pn_F1_OUTPUT || param->mode == GPIOx_Pn_F6_EINT;
}

bool xr806_wake_pin_active(uint8_t pin, bool falling) {
	if (xr806_wake_pin_event(pin) == 0)
		return false;
	GPIO_InitParam previous;
	HAL_GPIO_GetConfig(GPIO_PORT_A, (GPIO_Pin)pin, &previous);
	if (wake_pin_owned(&previous))
		return false;
	wake_pin_input(pin, &previous, GPIOx_Pn_F0_INPUT, falling);
	bool active = wake_pin_at_level(pin, falling);
	HAL_GPIO_Init(GPIO_PORT_A, (GPIO_Pin)pin, &previous);
	return active;
}

xr806_standby_result_t xr806_standby_enter(const xr806_standby_config_t *config) {
	xr806_standby_result_t result = {
		.status	   = XR806_STANDBY_ENTERED,
		.pm_return = -1,
		.event	   = 0,
	};
	int pin			 = config->wake_pin;
	uint32_t wake_io = pin - XR806_WAKE_PIN_FIRST;
	bool pin_armed	 = false;
	GPIO_InitParam previous;

	if (pin >= 0 && xr806_wake_pin_event(pin) == 0) {
		result.status = XR806_STANDBY_PIN_INVALID;
		return result;
	}
	// the timer always runs, also as a fallback for the wake pin
	uint32_t timer_ms = MIN(MAX(config->timer_ms, WAKEUP_TIMER_MIN_TIME), XR806_WAKE_TIMER_MAX_MS);
	if (HAL_Wakeup_SetTimer_mS(timer_ms) != 0) {
		result.status = XR806_STANDBY_NO_TIMER;
		return result;
	}

	if (pin >= 0) {
		HAL_GPIO_GetConfig(GPIO_PORT_A, (GPIO_Pin)pin, &previous);
		if (wake_pin_owned(&previous)) {
			result.status = XR806_STANDBY_PIN_OWNED;
			goto cleanup;
		}
		wake_pin_input(pin, &previous, GPIOx_Pn_F6_EINT, config->wake_falling);
		pin_armed = true;
		// a wake level that is already there would end the standby at once
		if (wake_pin_at_level(pin, config->wake_falling)) {
			result.status = XR806_STANDBY_PIN_ACTIVE;
			goto cleanup;
		}
		HAL_Wakeup_SetIO(
			wake_io,
			config->wake_falling ? WKUPIO_WK_MODE_FALLING_EDGE : WKUPIO_WK_MODE_RISING_EDGE,
			config->wake_falling ? GPIO_PULL_UP : GPIO_PULL_DOWN
		);
		HAL_PRCM_SetWakeupDebClk0(0);
		HAL_PRCM_SetWakeupIOxDebSrc(wake_io, 0);
		HAL_PRCM_SetWakeupIOxDebounce(wake_io, 1);
	}

	if (!HAL_Wakeup_CheckIOMode()) {
		result.status = XR806_STANDBY_PIN_MODE;
		goto cleanup;
	}
	if (config->commit != NULL && !config->commit(config->arg)) {
		result.status = XR806_STANDBY_REFUSED;
		goto cleanup;
	}

	HAL_Wakeup_SetEvent(0);
	// waits for the wakelocks for at most PM_WAKELOCK_WAIT_MS, then returns after the wake-up
	uint64_t start_us	   = HAL_RTC_GetFreeRunTime();
	TickType_t start_ticks = xTaskGetTickCount();
	result.pm_return	   = pm_enter_mode_timeout(PM_MODE_STANDBY, PM_WAKELOCK_WAIT_MS);
	result.event		   = HAL_Wakeup_GetEvent();
	step_tick_over_standby(start_us, start_ticks);

cleanup:
	HAL_Wakeup_ClrTimer();
	if (pin_armed) {
		HAL_Wakeup_ClrIO(wake_io);
		HAL_PRCM_SetWakeupIOxDebounce(wake_io, 0);
		HAL_GPIO_Init(GPIO_PORT_A, (GPIO_Pin)pin, &previous);
	}
	return result;
}

uint64_t xr806_rtc_time_us(void) {
	return HAL_RTC_GetFreeRunTime();
}

void *xr806_wakelock_create(const char *name) {
	struct wakelock *wakelock = calloc(1, sizeof(struct wakelock));
	if (wakelock != NULL)
		wakelock->name = (char *)name;
	return wakelock;
}

bool xr806_wakelock_acquire(void *wakelock) {
	return wakelock != NULL && pm_wake_lock(wakelock) == 0;
}

bool xr806_wakelock_release(void *wakelock) {
	return wakelock != NULL && pm_wake_unlock(wakelock) == 0;
}

static uint8_t wlan_dtim = 1;
static uint8_t wlan_listen_interval;

void xr806_wlan_set_power_save(uint8_t dtim, uint8_t listen_interval) {
	wlan_dtim			 = dtim;
	wlan_listen_interval = listen_interval;
}

// The SDK switches power save on with every connection (net_ctrl.c), so WiFi applies it again then.
bool xr806_wlan_apply_power_save(struct netif *nif, bool enable) {
	if (nif == NULL)
		return false;
	if (enable && (wlan_ext_request(nif, WLAN_EXT_CMD_SET_PM_DTIM, wlan_dtim) != 0 ||
				   wlan_ext_request(nif, WLAN_EXT_CMD_SET_LISTEN_INTERVAL, wlan_listen_interval) != 0))
		return false;
	return wlan_set_ps_mode(nif, enable) == 0;
}

static uint32_t uart_rx_errors[UART_NUM];
static uint32_t uart_activity[UART_NUM];

void xr806_uart_note_activity(uint8_t port, uint32_t bytes, uint32_t errors) {
	if (port >= UART_NUM)
		return;
	__atomic_fetch_add(&uart_rx_errors[port], errors, __ATOMIC_RELAXED);
	__atomic_fetch_add(&uart_activity[port], bytes + errors, __ATOMIC_RELAXED);
}

uint32_t xr806_uart_rx_errors(uint8_t port) {
	return port < UART_NUM ? __atomic_load_n(&uart_rx_errors[port], __ATOMIC_RELAXED) : 0;
}

uint32_t xr806_uart_activity(uint8_t port) {
	return port < UART_NUM ? __atomic_load_n(&uart_activity[port], __ATOMIC_RELAXED) : 0;
}

// Reading the line status clears its error bits, so they are counted here.
static uint32_t uart_line_status(uint8_t port, UART_T *uart) {
	uint32_t status = uart->LINE_STATUS;
	if (status & UART_LINE_ERROR_BITS)
		xr806_uart_note_activity(port, 0, 1);
	return status;
}

bool xr806_uart_hw_idle(uint8_t port) {
	UART_T *uart = port < UART_NUM ? HAL_UART_GetInstance((UART_ID)port) : NULL;
	if (uart == NULL)
		return true;
	uint32_t status = uart_line_status(port, uart);
	return (status & UART_TX_EMPTY_BIT) && !(status & UART_RX_READY_BIT) && !(uart->STATUS & UART_BUSY_BIT);
}

bool xr806_uart_wait_tx_done(uint8_t port, uint32_t timeout_ms) {
	UART_T *uart = port < UART_NUM ? HAL_UART_GetInstance((UART_ID)port) : NULL;
	if (uart == NULL)
		return true;
	uint32_t start = OS_GetTicks();
	while (!(uart_line_status(port, uart) & UART_TX_EMPTY_BIT)) {
		if (OS_TicksToMSecs(OS_GetTicks() - start) > timeout_ms)
			return false;
	}
	return true;
}
