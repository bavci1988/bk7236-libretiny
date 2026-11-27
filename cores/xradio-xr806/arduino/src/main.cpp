#include <ArduinoPrivate.h>

extern "C" {

// lt_main() already runs in the "main" thread of the SDK (8 KiB stack, started by
// project/common/startup/gcc/retarget_main.c), with the scheduler running.
bool startMainTask() {
	mainTask(NULL);
	return true;
}

} // extern "C"
