#pragma once

// Provide GPIO names to variant.cpp files
#define LT_VARIANT_INCLUDE "gpio_names.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern int port_disable_interrupts_flag(void);
extern void port_enable_interrupts_flag(int val);

#define interrupts()   port_enable_interrupts_flag(0)
#define noInterrupts() port_disable_interrupts_flag()

#ifdef __cplusplus
} // extern "C"
#endif
