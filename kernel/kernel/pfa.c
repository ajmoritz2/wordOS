#include "kernel.h"
#include "header/string.h"
#include <stddef.h>
#include "header/paging.h"
#include "header/pfa.h"
#include "kernel.h"

// START PFA
#define KERNEL_RESERVED 0x400000

uint32_t pre_mem = KEND + 0xC0000000;

uint32_t* frames;
uint32_t num_frames;
uint32_t last_frame = 0;

// ALLOCATE A PAGE except dumb
void *pre_malloc(uint32_t size, uint32_t* physical)
{
	
	uint32_t mem_loc = PGROUNDUP(pre_mem);
	pre_mem += PGROUNDUP(size);
	if (physical) *physical = mem_loc - 0xC0000000;
	return (void*) mem_loc;	
}

// Gets first frame available
uint32_t get_first_frame()
{
	uint32_t i = last_frame;
	for (; i < num_frames; i++) {
		if (!(i & 0xffffffff)) {
			for (int j = 0; j < sizeof(uint32_t); j++) {
				uint32_t mask = 0x1 << j;
				if (!(frames[i] & mask)) return i*32 + j;
			}
		}
	}

	return -1;

}


// Bitmap is the frame*32 + offset
uint32_t *frame_to_physical(uint32_t bitmap)
{
	// Lets start a 1MiB
	return (uint32_t*) (0x100000 + (bitmap*4096));
}

uint32_t physical_to_frame(uint32_t* physical)
{
	// We know starting is at 1MiB
	return ((uint32_t)physical-0x100000) / 4096;
}

void kinit() 
{
	// Start the page frames
	num_frames = NUM_FRAMES; // 10 MiB of pages
	frames = pre_malloc(num_frames, 0);
	memset(frames, 0, num_frames);

	// Because I'm lazy and I hope they wont change,
	// I am just going to hardcode the initial kernel frames.
	// We start at frame 0 bit 0 and go until bit 17
	
}
// END PFA
