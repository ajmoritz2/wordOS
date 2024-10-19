#ifndef PFADEFINE
#define PFADEFINE
#include <stddef.h>
#include <stdint.h>

#define NUM_FRAMES (10 * 1024 * 1024) / 4096

uint32_t kinit(uint32_t*);

void* pre_malloc(uint32_t size, uint32_t*);
void clear_frame(uint32_t);
uint32_t *alloc_phys_page();
uint32_t physical_to_frame(uint32_t* physical);
uint32_t *frame_to_physical(uint32_t);

#endif
