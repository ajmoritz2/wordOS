#ifndef PFADEFINE
#define PFADEFINE
#include <stddef.h>

#define NUM_FRAMES (10 * 1024 * 1024) / 4096

void kinit();

void kfree(void *pa);
void* pre_malloc(uint32_t size, uint32_t*);

#endif
