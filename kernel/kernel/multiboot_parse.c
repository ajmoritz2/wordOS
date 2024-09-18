#include "header/multiboot2.h"
#include "kernel.h"
#include <stdint.h>

struct multiboot_tag_framebuffer* fb;

void init_multiboot(uint32_t addr)
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
//	log_integer_to_serial(fb->common.framebuffer_type);
	uint32_t color = ((1 << fb->framebuffer_blue_mask_size) - 1)
		<< fb->framebuffer_blue_field_position;

	for (uint32_t i = 0; i < 100; i++)
	{
			
		uint32_t *pixel = fb_addr + fb->common.framebuffer_pitch * i + 4 * i;
		print_hex((uint32_t) fb->common.framebuffer_addr);
		log_to_serial("\n");
		*pixel = color;		
		
		
	}

	return;
}
