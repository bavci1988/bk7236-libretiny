#pragma once

#include_next "FreeRTOSConfig.h"

// Tickless idle: don't enter low-voltage sleep while UART TX is running (port/pm_hooks.c)
#ifndef __ASSEMBLER__
void lt_pre_sleep_processing(void *idle_time);
#endif
#define configPRE_SLEEP_PROCESSING(x) lt_pre_sleep_processing(&(x))
