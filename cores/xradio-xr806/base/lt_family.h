#pragma once

#include <lt_pins.h>

// Choose the main UART output port
// UART2 (PA21/PA22) is the console of the SDK boards and of Tuya WXU modules;
// UART0 (PB00/PB01) is the download port and the Tuya MCU UART.
#ifndef LT_UART_DEFAULT_PORT
#if LT_HW_UART2
#define LT_UART_DEFAULT_PORT 2
#elif LT_HW_UART1
#define LT_UART_DEFAULT_PORT 1
#elif LT_HW_UART0
#define LT_UART_DEFAULT_PORT 0
#else
#error "No serial port is available"
#endif
#endif
