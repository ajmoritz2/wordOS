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
#include "../memory/paging.h"
#include "../memory/string.h"
#include "../kernel/kernel.h"
#include <stdint.h>

extern vmm* current_vmm;
struct ACPISDTHeader* rsdt_addr;

struct multiboot_tag_framebuffer* fb;
struct multiboot_tag_old_acpi* old_acpi;

// FRAMEBUFFER BALONEY TODO: Move to own file...
uint32_t* pixel_addr (uint32_t* fb_addr, uint32_t x, uint32_t y, uint32_t pitch, uint32_t bypp)
{
	uint32_t row = y * (pitch);
	uint32_t column = x * (bypp);

	uint32_t final_addr = (uint32_t) fb_addr + row + column;

	return (uint32_t*) final_addr;
}

void fill_framebuffer() {
	uint32_t* fb_addr = (uint32_t*)(fb->common.framebuffer_addr);
	uint32_t num_pages = (fb->common.framebuffer_pitch * fb->common.framebuffer_height) / 4096;

	logf("Type of FrameBuffer: %d\n", fb->common.framebuffer_type);

	// Mapping framebuffer
	for (int i = 0; i < num_pages; i++) {

		memory_map(current_vmm->root_pd, fb_addr + (i*0x1000/4), (uint32_t*)(0xD0000000 + (i * 0x1000)), 0x3);
	}

	uint32_t bypp = fb->common.framebuffer_bpp/8;

	for (int x = 0; x < 1279; x++) {
		for (int y = 0; y < 800; y++) {
//			uint32_t* pix = pixel_addr((uint32_t*)0xD0000000, x, y, fb->common.framebuffer_pitch, bypp);
//			*pix = 0x00FFFF;
		}
	}

}
// MADT Parsing
void* parse_MADT(uint8_t entry_id)
{
	struct ACPISDTHeader* madt = (struct ACPISDTHeader*) get_sdt_by_signature("APIC");
	
	uint16_t offset = sizeof(struct ACPISDTHeader) + 8; // The +8 is for the 8 bytes of other data here
	uint8_t* mem_p = (uint8_t*) ((uint32_t) madt + offset);
	while (offset <= madt->Length) {
		struct MADTEntryHead* entry_head = (struct MADTEntryHead*) mem_p;
		if (entry_head->EntryType == entry_id) {
			logf("Length %x\n", (mem_p+2));
			return (void*)(mem_p + 2);
		}
		offset+=entry_head->Length;
		mem_p+=entry_head->Length;
	}
	
	logf("MADT entry with id %d NOT FOUND!\n", entry_id);
	return NULL;	
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

	int entries = (rsdt->h.Length - sizeof(struct ACPISDTHeader)) / 4;

	for (int i = 0; i < entries; i++) {
		struct ACPISDTHeader* potential = (struct ACPISDTHeader *) rsdt->NextSDT[i];
		if (strcmp(signature, potential->Signature, 4))
			return (void*) potential;
	}
	logf("Signature not found in ACPI tables");
	return NULL;

}

void init_rsdt_v1()
{
	if (!old_acpi)
		return;
	// Super weird hack to get the struct for the RSDP
	struct RSDPDescriptor* rsdp_d = (struct RSDPDescriptor*) old_acpi->rsdp;
	if (validate_RSDP((char *) rsdp_d, sizeof(rsdp_d))) {
		logf("RSDP NOT VALIDATED >><< \n");
		return;
	}
	rsdt_addr = (struct ACPISDTHeader *) rsdp_d->RsdtAddress;
	memory_map(current_vmm->root_pd, (uint32_t*)((uint32_t)rsdt_addr & ~0xFFF), (uint32_t*)((uint32_t) rsdt_addr & ~0xFFF), 0x1);	
	logf("APIC Table found at %x \n", get_sdt_by_signature("APIC"));
}


// INIT MACARONI
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
			case 14:
				old_acpi = (struct multiboot_tag_old_acpi*) tag;
				break;
			case 15:
				break;
		}
	}


	if (old_acpi)	
		init_rsdt_v1();	
	

	return;
}
