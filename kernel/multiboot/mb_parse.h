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
} __attribute__ ((packed));

struct ACPISDTHeader {
	char Signature[4];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
};

struct MADTEntryHead {
	uint8_t EntryType;
	uint8_t Length;
};

struct MADTEntryType0 {
	uint8_t ACPIProcessor_id;
	uint8_t APIC_id;
	uint32_t Flags;	
};

struct MADTEntryType1 {
	uint8_t IOAPIC_id;
	uint8_t Reserved;
	uint32_t IOAPICAddr;
	uint32_t GSIB; // Global System Interrupt Base
}__attribute__((packed));

struct MADTEntryType2 {
	uint8_t BusSource;
	uint8_t IRQSource;
	uint32_t GSI;
	uint16_t Flags;
} __attribute__ ((packed));

struct multiboot_tag_pointers {
	struct multiboot_tag_mmap *mmap;
	struct multiboot_tag_framebuffer *framebuffer;
	struct multiboot_tag_old_acpi *old_acpi;
};	

void get_mmap(struct multiboot_tag_pointers* tags);
void* get_sdt_by_signature(char* signature);

uint8_t* get_pixel_addr(uint32_t x, uint32_t y);

void* parse_MADT(uint8_t entry_id, uint8_t count);
struct multiboot_tag_pointers init_multiboot(uint32_t *root_pd, uint32_t addr);
void init_framebuffer();

#endif
