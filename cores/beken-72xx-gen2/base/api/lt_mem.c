#include <libretiny.h>
#include <sdk_private.h>

uint32_t lt_ram_get_size() {
	return 640 * 1024;
}

uint32_t lt_heap_get_size() {
	return rtos_get_total_heap_size();
}

uint32_t lt_heap_get_free() {
	return rtos_get_free_heap_size();
}

uint32_t lt_heap_get_min_free() {
	return rtos_get_minimum_free_heap_size();
}
