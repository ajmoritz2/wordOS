#include "header/multiboot2.h"
#include "kernel.h"
#include "header/vmm.h"
#include "header/paging.h"
#include <stdint.h>

struct multiboot_tag_framebuffer* fb;

void init_multiboot(vmm* current_vmm, uint32_t addr)
{
	struct multiboot_tag *tag;
	unsigned size;
	size = *(unsigned *) addr;
	for (tag = (struct multiboot_tag *) (addr + 8);
			tag->type != MULTIBOOT_TAG_TYPE_END;
			tag = (struct multiboot_tag *) ((multiboot_uint8_t *)  tag + ((tag->size + 7) & ~7)))
	{
		switch (tag->type)
		{
			case 8:
				fb = (struct multiboot_tag_framebuffer*) tag;
				break;
		}
	}
	
	uint32_t* fb_addr = (uint32_t*)(fb->common.framebuffer_addr);
	uint32_t color = 0xffffff; 
//	logf("Blue mask size: %d Blue position: %x\n", fb->framebuffer_blue_mask_size, fb->framebuffer_blue_field_position);	
	logf("Width: %x, Height: %x, BPP: %x Type: %d\n\n", fb->common.framebuffer_width,
			fb->common.framebuffer_height, fb->common.framebuffer_bpp, fb->common.framebuffer_type);

	int row = 1 * (fb->common.framebuffer_pitch);
	int column = 800;
	uint32_t* pixel = (uint32_t*) (fb_addr +  row);
	logf("Final pixel kept at %x\n", PGROUNDDOWN((uint32_t)pixel));
	memory_map(current_vmm->root_pd, (uint32_t*)PGROUNDDOWN((uint32_t)pixel), (uint32_t*)0xB0000000, 0x3);
	memory_map(current_vmm->root_pd, (uint32_t*)PGROUNDDOWN((uint32_t)pixel), (uint32_t*)0xB0001000, 0x3);
	pixel = (uint32_t*)0xB0000000;
	*pixel = color;
	logf("pixel_loc = %x\n", pixel);
	pixel = pixel + (fb->common.framebuffer_pitch/4);
	*pixel = color;
	logf("pixel_loc = %x\n", pixel);
//	memory_map(current_vmm->root_pd, fb_addr, fb_addr, 0x3);
	return;
}
