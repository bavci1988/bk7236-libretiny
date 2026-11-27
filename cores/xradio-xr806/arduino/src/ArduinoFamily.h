#pragma once

// Provide GPIO names to variant.cpp files
#define LT_VARIANT_INCLUDE "sdk_private.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern void vPortClearInterruptMask(uint32_t ulNewMaskValue);
extern uint32_t ulPortSetInterruptMask(void);

#define interrupts()   vPortClearInterruptMask(0)
#define noInterrupts() ulPortSetInterruptMask()

#ifdef __cplusplus
} // extern "C"
#endif
