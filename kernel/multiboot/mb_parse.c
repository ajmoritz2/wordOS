/* MB_PARSE FILE
 *
 * 10/10/24
 *
 * HEADER FILE: mb_parse.h
 *
 * wordOS
 */
#include "multiboot2.h"
#include "mb_parse.h"
#include "../memory/vmm.h"
#include "../memory/pmm.h"
#include "../memory/paging.h"
#include "../memory/string.h"
#include "../kernel/kernel.h"
#include "../drivers/framebuffer.h"
#include "../programs/terminal.h"
#include "../acpi/tables.h"
#include "../acpi/acpi.h"
#include <stdint.h>

extern vmm* current_vmm;
extern uint32_t* fb_virt_addr;
struct ACPISDTHeader* rsdt_addr;

struct multiboot_tag_framebuffer* fb;
static uint32_t bypp = 0;
struct multiboot_tag_old_acpi* old_acpi;


// FRAMEBUFFER BALONEY TODO: Move to own file...
uint8_t* get_pixel_addr (uint32_t x, uint32_t y)
{

	if (x > fb->common.framebuffer_width)
		x = fb->common.framebuffer_width;
	if (y > fb->common.framebuffer_height)
		y = fb->common.framebuffer_height;
	uint32_t row = y * (fb->common.framebuffer_pitch);
	uint32_t column = x * (bypp);
	uint32_t final_addr = (uint32_t) fb_virt_addr + row + column;

	return (uint8_t*) final_addr;
}

void init_framebuffer() {
	// HIGH priority: Make more flexible to all values rather than assuming...
	uint64_t* fb_addr = (uint64_t*)(fb->common.framebuffer_addr);
	uint32_t num_pages = (fb->common.framebuffer_pitch * fb->common.framebuffer_height);

	logf("Type of FrameBuffer: %d\n", fb->common.framebuffer_type);
	logf("Framebuffer bpp: %d, framebuffer width: %d, framebuffer height: %d\n", fb->common.framebuffer_bpp, 
				fb->common.framebuffer_width, fb->common.framebuffer_height);
	logf("FB_ADDR: %x\n", fb_addr);

	// TODO: Add support for indexed color (although I can't test it...)
	switch (fb->common.framebuffer_type) {
		case 0:
			panic("Framebuffer in INDEXED_COLOR. Not supported but COOL!!!!\n");
			break;
		case 1:
			logf("Supported RGB mode for framebuffer!\n");
			break;
		case 2:
			panic("Framebuffer in TEXT_MODE. how did you get here??\n");
			break;
		default:
			triple_fault();
	}


	// Mapping framebuffer
	fb_virt_addr = (uint32_t*) pae_kalloc(num_pages, 0x3, (uint64_t) fb->common.framebuffer_addr);



	bypp = fb->common.framebuffer_bpp/8;
	fb_set_bpp(bypp);
	fb_set_width(fb->common.framebuffer_width);
	fb_set_height(fb->common.framebuffer_height);
}

// ACPI TABLES BALONEY
uint8_t validate_RSDP(char *byte_array, size_t size) {
       	uint32_t sum = 0;
	for(int i = 0; i < size; i++) {
		sum += byte_array[i];
	}
	return (sum & 0xFF) == 0;
}

void* get_sdt_by_signature(char* signature)
{
	struct RSDT {
		struct ACPISDTHeader h;
		uint32_t NextSDT[(rsdt_addr->Length - sizeof(struct ACPISDTHeader)) / 4];
	};
	struct RSDT* rsdt = (struct RSDT *) rsdt_addr;
	logf("rsdt: %x\n", rsdt);

	int entries = (rsdt->h.Length - sizeof(struct ACPISDTHeader)) / 4;
	struct ACPISDTHeader* potential = 0;
	for (int i = 0; i < entries; i++) {
		struct ACPISDTHeader* potential = (struct ACPISDTHeader *) rsdt->NextSDT[i];
		memory_map(current_vmm->root_pd, (uint32_t*) PGROUNDDOWN((uint32_t) potential), \
				(uint32_t*) PGROUNDDOWN((uint32_t) potential), 0x3);
		if (strcmp(signature, potential->Signature, 4)) {
			logf("Potential: %x\n", potential);
			return (void*) potential;
		}
	}


	panic("Signature not found in ACPI tables\n");
	return NULL;

}

void init_rsdt_v1()
{
	if (!old_acpi)
		return;
	// Super weird hack to get the struct for the RSDP
	struct RSDPDescriptor* rsdp_d = (struct RSDPDescriptor*) old_acpi->rsdp;
	logf("RSDP Found at %x\n", rsdp_d->RsdtAddress);
	logf("RSDP Revision %x\n", rsdp_d->Checksum);
	if (rsdp_d->Revision >= 2) {
		if (!validate_table((struct ACPISDTHeader *) rsdp_d)) {
			panic("XSDT NOT VALIDATED \n");
		}

		panic("Xsdt validated :^) (No support)\n");
	} else {
		if (validate_RSDP((char *) rsdp_d, 19)) { // Needs to be hardcode 20 for legacy
			panic("RSDP NOT VALIDATED >><< \n");
			return;
		}
	}
	// Easiest to just identity map here...
	memory_map(current_vmm->root_pd, (uint32_t*)((uint32_t)rsdp_d->RsdtAddress & ~0xFFF), (uint32_t*)((uint32_t)rsdp_d->RsdtAddress & ~0xFFF), 0x1);
	
	rsdt_addr = (struct ACPISDTHeader *) rsdp_d->RsdtAddress;

	if (rsdt_addr->Length + (uint32_t) rsdt_addr > PGROUNDUP((uint32_t) rsdt_addr)) {
		// We use 4096 because it is in uint32_t form; not a pointer...
		memory_map(current_vmm->root_pd, (uint32_t*)((uint32_t)rsdp_d->RsdtAddress & ~0xFFF + 4096), (uint32_t*)((uint32_t)rsdp_d->RsdtAddress & ~0xFFF + 4096), 0x1);
	}
	load_rsdt_store( rsdt_addr);
}


// INIT MACARONI
// This code is spaghetti as hell...
struct multiboot_tag_pointers init_multiboot(uint32_t *root_pd, uint32_t addr)
{
	struct multiboot_tag_pointers all_tags = {0};
	struct multiboot_tag *tag;
	unsigned size;
	size = *(unsigned *) addr;

	if ((uint32_t) addr + size > PGROUNDUP((uint32_t) addr)) {
		memory_map(root_pd, (uint32_t *) PGROUNDDOWN((uint32_t) addr - 0xC0000000),(uint32_t *) PGROUNDDOWN((uint32_t) addr), 0x3);
		memory_map(root_pd, (uint32_t *) PGROUNDDOWN((uint32_t) addr - 0xC0000000 + size),(uint32_t *) PGROUNDDOWN((uint32_t) addr + size), 0x3);
	}


	uint32_t test = *((uint32_t *)(addr + size));

	for (tag = (struct multiboot_tag *) (addr + 8);
			tag->type != MULTIBOOT_TAG_TYPE_END;
			tag = (struct multiboot_tag *) ((multiboot_uint8_t *)  tag + ((tag->size + 7) & ~7)))
	{
		switch (tag->type)
		{
			case MULTIBOOT_TAG_TYPE_MMAP:
				logf("Multiboot MMAP tag found...\n");
				all_tags.mmap = (struct multiboot_tag_mmap *) tag;
				break;
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER	:
				all_tags.framebuffer = (struct multiboot_tag_framebuffer*) tag;
				break;
			case 14:
				all_tags.old_acpi = (struct multiboot_tag_old_acpi*) tag;
				break;
			case 15:
				break;
		}
	}

	fb = all_tags.framebuffer;
	old_acpi = all_tags.old_acpi;

	if (old_acpi)
		init_rsdt_v1();	
	//set_memory_map(all_tags.mmap);

	return all_tags;
}
