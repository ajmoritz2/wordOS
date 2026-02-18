#ifndef MULTIBOOT_PARSE
#define MULTIBOOT_PARSE
#include <stdint.h>
#include "../memory/vmm.h"
#include "multiboot2.h"

struct RSDPDescriptor {
	char Signature[8];
	uint8_t Checksum;
	char OEMID[6];
	uint8_t Revision;
	uint32_t RsdtAddress;
	uint32_t Length;
	uint64_t XsdtAddress;
	uint8_t XChecksum;
	uint16_t Reserved;
	uint16_t Reserved2;
} __attribute__ ((packed));


struct multiboot_tag_pointers {
	struct multiboot_tag_mmap *mmap;
	struct multiboot_tag_framebuffer *framebuffer;
	struct multiboot_tag_old_acpi *old_acpi;
};	

void get_mmap(struct multiboot_tag_pointers* tags);
void* get_sdt_by_signature(char* signature);

uint8_t* get_pixel_addr(uint32_t x, uint32_t y);

struct multiboot_tag_pointers init_multiboot(uint32_t *root_pd, uint32_t addr);
void init_framebuffer();

#endif
