#include <libretiny.h>
#include <sdk_private.h>

#include <malloc.h>

// linker script: the newlib heap spans from the end of .bss to the MSP stack
extern uint8_t __heap_start__[];
extern uint8_t __heap_end__[];
extern void *_sbrk(int incr);

// newlib-nano malloc keeps the freed chunks below the top of the heap in this list
typedef struct malloc_chunk {
	long size;
	struct malloc_chunk *next;
} malloc_chunk_t;

extern malloc_chunk_t *__malloc_free_list;

// chunk header and alignment of newlib-nano malloc
#define MALLOC_CHUNK_OVERHEAD 8

// the heap does not record its low-water mark; this is the lowest value lt_heap_get_*() has seen
static uint32_t heap_min_free = UINT32_MAX;

uint32_t lt_ram_get_size() {
	return 320 * 1024;
}

uint32_t lt_heap_get_size() {
	return __heap_end__ - __heap_start__;
}

uint32_t lt_heap_get_free() {
	struct mallinfo info = mallinfo();
	uint32_t free		 = (uint32_t)(__heap_end__ - (uint8_t *)_sbrk(0)) + info.fordblks;
	if (free < heap_min_free)
		heap_min_free = free;
	return free;
}

uint32_t lt_heap_get_min_free() {
	lt_heap_get_free();
	return heap_min_free;
}

uint32_t lt_heap_get_max_alloc() {
	// the same lock as the SDK malloc wrappers (sys/sys_heap.h)
	OS_ThreadSuspendScheduler();
	uint32_t largest = (uint32_t)(__heap_end__ - (uint8_t *)_sbrk(0));
	for (malloc_chunk_t *chunk = __malloc_free_list; chunk != NULL; chunk = chunk->next) {
		if ((uint32_t)chunk->size > largest)
			largest = chunk->size;
	}
	OS_ThreadResumeScheduler();
	return largest > MALLOC_CHUNK_OVERHEAD ? largest - MALLOC_CHUNK_OVERHEAD : 0;
}
