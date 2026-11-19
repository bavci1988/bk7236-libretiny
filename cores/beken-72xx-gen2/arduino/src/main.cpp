/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <ArduinoPrivate.h>

extern "C" {

#include <os/os.h>

static beken_thread_t mainThread;

bool startMainTask() {
	// the FreeRTOS scheduler is already started by ARMINO
	bk_err_t ret = rtos_create_thread(
		&mainThread,
		BEKEN_APPLICATION_PRIORITY,
		"main",
		(beken_thread_function_t)mainTask,
		8192,
		NULL
	);
	return ret == BK_OK;
}

} // extern "C"
