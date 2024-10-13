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

void* get_sdt_by_signature(char* signature);

void init_multiboot(uint32_t addr);

#endif
