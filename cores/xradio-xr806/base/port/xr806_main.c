#include <libretiny.h>
#include <sdk_private.h>

#include <string.h>

extern uint8_t __bss_start__[];
extern uint8_t __bss_end__[];

// project/common/startup/gcc/retarget.c
extern void hardware_init_hook(void);
extern int __wrap_main(void);

extern int lt_main(void);

// api/lt_device.c
extern void xr806_save_boot_reason(void);

// crti.o/crtn.o are not linked (-nostartfiles); called by __libc_init_array()
void _init(void) {}

void _fini(void) {}

// normally provided by crtbegin.o, used by __cxa_atexit() for C++ static objects
void *__dso_handle = &__dso_handle;

// Called by Reset_Handler (startup.s) in place of the C library's crt0
void _start(void) {
	memset(__bss_start__, 0, __bss_end__ - __bss_start__);
	hardware_init_hook();
	// SystemInit() (in hardware_init_hook()) has read the reset source and saved the wake-up events
	xr806_save_boot_reason();
	// clocks, GPIO and stdout; starts the FreeRTOS "main" thread, which calls __real_main()
	__wrap_main();
}

// Called in the "main" thread (project/common/startup/gcc/retarget_main.c)
int __real_main(void) {
	platform_init();
	return lt_main();
}
