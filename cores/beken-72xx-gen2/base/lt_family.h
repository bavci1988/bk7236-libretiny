#pragma once

#include <lt_pins.h>

// Choose the main UART output port
// ARMINO prints to UART_ID_0 by default (P10/P11, SoC UART1)
#ifndef LT_UART_DEFAULT_PORT
#if LT_HW_UART1
#define LT_UART_DEFAULT_PORT 1
#elif LT_HW_UART2
#define LT_UART_DEFAULT_PORT 2
#else
#error "No serial port is available"
#endif
#endif
