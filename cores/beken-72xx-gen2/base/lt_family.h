/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#pragma once

#include <lt_pins.h>

// Choose the main UART output port
// ARMINO prints to UART_ID_0 by default (UART0, also used for flashing)
#ifndef LT_UART_DEFAULT_PORT
#if LT_HW_UART0
#define LT_UART_DEFAULT_PORT 0
#elif LT_HW_UART1
#define LT_UART_DEFAULT_PORT 1
#else
#error "No serial port is available"
#endif
#endif
