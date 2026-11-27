/* Copyright (c) bavci1988 2026-11-23. */

#pragma once

// XR806 power management for applications: standby with a timer and a GPIO wake source,
// PM wakelocks, WLAN power save parameters and UART state for an idle check.

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Wake events (bits of xr806_standby_result_t.event)
#define XR806_WAKE_EVENT_TIMER	  (1u << 16)
#define XR806_WAKE_EVENT_WLAN	  (1u << 17)
#define XR806_WAKE_EVENT_SOFTWARE (1u << 27)
#define XR806_WAKE_EVENT_INTERNAL 0xF0000000u

// Longest wake timer: the 31-bit wake-up counter runs at about 32 kHz
#define XR806_WAKE_TIMER_MAX_MS (18u * 3600u * 1000u)

// GPIO PA10..PA23 are the wake inputs WKIO0..WKIO13
#define XR806_WAKE_PIN_FIRST 10
#define XR806_WAKE_PIN_LAST	 23

typedef enum {
	XR806_STANDBY_ENTERED = 0, //!< the chip was in standby (or PM returned an error), see pm_return
	XR806_STANDBY_NO_TIMER,	   //!< the wake timer could not be armed
	XR806_STANDBY_PIN_INVALID, //!< the wake pin is not a wake input
	XR806_STANDBY_PIN_OWNED,   //!< the wake pin is an output or another interrupt uses it
	XR806_STANDBY_PIN_ACTIVE,  //!< the wake pin is already at its wake level
	XR806_STANDBY_PIN_MODE,	   //!< the wake pin did not switch to its interrupt function
	XR806_STANDBY_REFUSED,	   //!< the commit callback refused the standby
} xr806_standby_status_t;

typedef struct {
	uint32_t timer_ms;		   //!< wake timer, at least 10 ms; always armed
	int8_t wake_pin;		   //!< PA10..PA23 (GPIO number 10..23), or -1
	bool wake_falling;		   //!< wake on a falling edge (with pull-up), else rising (pull-down)
	bool (*commit)(void *arg); //!< called with the wake sources armed, right before PM entry; NULL to skip
	void *arg;				   //!< argument of commit
} xr806_standby_config_t;

typedef struct {
	xr806_standby_status_t status;
	int pm_return;	//!< return value of pm_enter_mode_timeout(), if entered
	uint32_t event; //!< wake events, if entered
} xr806_standby_result_t;

/**
 * @brief Enter standby until the timer, the wake pin or (with power save) the WLAN wakes the chip.
 *
 * The wake pin keeps its pin function outside of this call. All RAM is retained,
 * and the call returns after the wake-up.
 */
xr806_standby_result_t xr806_standby_enter(const xr806_standby_config_t *config);

/**
 * @brief Whether a wake input is at its wake level now, read with the pull resistor of the wake edge.
 * Returns false for an output or a pin used as interrupt.
 */
bool xr806_wake_pin_active(uint8_t pin, bool falling);

/**
 * @brief Wake event bit of a wake pin, or 0 if the pin is not a wake input.
 */
static inline uint32_t xr806_wake_pin_event(int pin) {
	return pin >= XR806_WAKE_PIN_FIRST && pin <= XR806_WAKE_PIN_LAST ? 1u << (pin - XR806_WAKE_PIN_FIRST) : 0;
}

/**
 * @brief Microseconds of the RTC counter, which keeps counting in standby.
 */
uint64_t xr806_rtc_time_us(void);

/**
 * @brief Microseconds since this boot, from the RTC counter (also counts in standby).
 */
uint64_t xr806_uptime_us(void);

/**
 * @brief A PM wakelock: while it is held, PM does not suspend the chip.
 * The name must stay valid. Returns NULL without memory.
 */
void *xr806_wakelock_create(const char *name);
bool xr806_wakelock_acquire(void *wakelock);
bool xr806_wakelock_release(void *wakelock);

/**
 * @brief DTIM period and listen interval used while WiFi sleep (power save) is on;
 * applied by WiFi.setSleep(true) and on every connection. Defaults: 1 and 0 (the AP's value).
 */
void xr806_wlan_set_power_save(uint8_t dtim, uint8_t listen_interval);

/**
 * @brief Count of received bytes that were lost (buffer full) or damaged (line error) on the UART.
 */
uint32_t xr806_uart_rx_errors(uint8_t port);

/**
 * @brief Count that changes with every byte sent or received and every line error on the UART.
 */
uint32_t xr806_uart_activity(uint8_t port);

/**
 * @brief The UART has nothing to send (including the shift register), nothing received in the FIFO, and is not busy.
 */
bool xr806_uart_hw_idle(uint8_t port);

/**
 * @brief Wait until the UART has sent everything, including the shift register. False on a timeout.
 */
bool xr806_uart_wait_tx_done(uint8_t port, uint32_t timeout_ms);

// Called by Serial for the bytes it sends and receives
void xr806_uart_note_activity(uint8_t port, uint32_t bytes, uint32_t errors);

#ifdef __cplusplus
} // extern "C"
#endif
