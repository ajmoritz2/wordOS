#ifndef HEAP_C
#define HEAP_C
#include <stddef.h>

#define HEAP_USED	1
#define HEAP_FREE	0

#define HEAP_START_SIZE		40960 //40 KiB of heap to start
#define HEAP_MIN_SPLIT_SIZE 0x20 + sizeof(struct mem_header)

struct mem_header {
	size_t size;
	uint8_t status;
	struct mem_header *prev;
	struct mem_header *next;
};

void init_heap();

void *kalloc(size_t size);
void kfree(void *mem);
void *rekalloc(void *ptr, size_t size);

#endif
