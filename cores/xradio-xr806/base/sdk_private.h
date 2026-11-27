#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// PinNames
#include "PinNames.h"

// SDK configuration, then the project configuration; the SDK headers depend on both
#include "xr_config.h"

#include "common/prj_conf_opt.h"

#include "common/board/board.h"
#include "common/framework/platform_init.h"
#include "driver/chip/hal_chip.h"
#include "driver/chip/hal_flash.h"
#include "driver/chip/hal_gpio.h"
#include "driver/chip/hal_rtc.h"
#include "driver/chip/hal_uart.h"
#include "image/flash.h"
#include "kernel/os/os.h"

#ifdef __cplusplus
} // extern "C"
#endif
