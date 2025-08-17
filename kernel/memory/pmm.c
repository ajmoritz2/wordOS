#include "../kernel/kernel.h"
#include "string.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"
#include "../multiboot/multiboot2.h"
#include <stddef.h>

// START PFA
#define KERNEL_RESERVED 0x400000

static uint32_t mem_top = 0x2000000, mem_bottom = 0x1000000; // Initial values to jumpstart the code

uint32_t pre_mem = KEND + 0xC0000000;

uint32_t* frames;
uint32_t num_frames;
uint32_t last_frame = 0;

void set_memory_map(struct multiboot_tag_mmap *mmap)
{
	int entries = (mmap->size - sizeof(struct multiboot_tag_mmap)) / mmap->entry_size;
	
	for (int i = 0; i < entries; i++) {
		multiboot_memory_map_t entry = mmap->entries[i];

		if (entry.type == MULTIBOOT_MEMORY_AVAILABLE) {
			mem_bottom = (uint32_t) entry.addr;
			mem_top = (uint32_t) (entry.addr + entry.len);
			logf("Available memory from %x to %x\n", mem_bottom, mem_top);	
		}
	}

}	

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
	uint32_t i = 0;
	for (; i < num_frames; i++) {
		if (frames[i] != 0xffffffff) {
			for (int j = 0; j < 32; j++) {

				uint32_t mask = 0x1 << j;
				if (!(frames[i] & mask)) {last_frame = i*32 + j; return last_frame;}
			}
		}
	}

	return -1;
}

void clear_frame(uint32_t frame)
{
	frames[frame/32] &= ~(0x1 << frame%32);
}

// Sets a frame as taken
void set_frame(uint32_t frame)
{
	uint32_t index = frame/32;
	uint32_t offset = frame%32;

	frames[index] = frames[index] | (0x1 << offset);
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
	// How can we make this smart with the memory frames? We will do this until we run into problems.
	return ((uint32_t)physical-0x100000) / 4096;
}

uint32_t *alloc_phys_page()
{
	uint32_t first_frame = get_first_frame();
	if (first_frame == -1)
	{
		//log_to_serial("OUT OF FRAMES!!!!!\n");
		return 0;
	}
	set_frame(first_frame);
	if (frame_to_physical(first_frame) > 0x1000000) {
		panic("Out of memory!");
	}
	return (uint32_t*) frame_to_physical(first_frame);
}

/*
 * Call this function once the VMM is set up.
 * This should transfer the legacy static code
 * to the better dynamic memory.
 *
 */
void transfer_dynamic()
{
	// This will be the frame storage pages.
	virtual_t *new_addr = page_kalloc(num_frames, 0x3, 0);	
	
	logf("Old Frame storage at %x ", frames);

	memcpy(new_addr, frames, num_frames); 

	frames = new_addr;

	logf("New frame storage at %x\n", $4 frames);
}

uint32_t kinit(uint32_t* after_mb) 
{
	logf("after_mb: %x\n", after_mb);
	// Start the page frames
	if ((uint32_t)after_mb >= &_kernel_end) {
		pre_mem += (uint32_t) after_mb;
	}


	logf("KEND %x, after_mb %x, PREMEM: %x\n", &_kernel_end, (uint32_t) after_mb, pre_mem);
	logf("NUM FRAMES %x\n", NUM_FRAMES);
	num_frames = NUM_FRAMES; // 10 MiB of pages
	frames = pre_malloc(num_frames, 0);
	memset(frames, 0, num_frames);
	// Because I'm lazy and I hope they wont change,
	// I am just going to hardcode the initial kernel frames.
	// We start at frame 0 bit 0 and go until bit 17, but I will give the first 32
	for (int i = 0; i < 48; i++)
	{
		set_frame(i);
	}
	
	return pre_mem;	
}
// END PFA
