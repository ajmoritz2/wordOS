/* Header file for the used tables of ACPI
 *
 * WordOS
 *
 * 11/07/2025
 */
#ifndef ACPI_TABLES_H
#define ACPI_TABLES_H

#include <stdint.h>

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
} __attribute__((packed)); // Ends at 32 bits


struct MADTEntryType0 {
	uint8_t ACPIProcessor_id;
	uint8_t APIC_id;
	uint32_t Flags;	
	uint32_t filler;
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
	uint16_t filler;
} __attribute__ ((packed));

struct MADTEntryHead {
	uint8_t EntryType;
	uint8_t Length;
};

struct MADTHeader {
	struct ACPISDTHeader header;
	uint32_t LocalICA; // Local Interupt controller address whatever that means
	uint32_t Flags;
	struct MADTEntryHead entries;
};

// PCIe
struct MMECS {
	uint64_t base_addr;
	uint16_t seg_group;
	uint8_t start_bus;
	uint8_t end_bus;
	uint32_t reserved;
};

struct MCFGHeader {
	struct ACPISDTHeader header;
	uint64_t reserved;
	struct MMECS config_space;
};


#endif
