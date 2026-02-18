/*
 * ACPI Table header file
 *
 * WordOS
 *
 * 11/07/25
 */

#ifndef ACPI_PROG_H
#define ACPI_PROG_H

#include <stdint.h>
#include "tables.h"

extern struct MADTHeader *madt_header; 
extern struct MADTEntryCount madt_counts;
extern struct MCFGHeader *mcfg_header;

struct MADTEntryCount {
	uint8_t lapic;
	uint8_t ioapic;
	uint8_t iso;	// Interupt source Override
	uint8_t nmis;	// Non-maskable interrupt source
	uint8_t lapic_nmi;
	uint8_t lapic_addr_o; // Address Override
	uint8_t iosapic;
	uint8_t lsapic;
	uint8_t pis; // Platform Interrupt Source
	uint8_t x2lapic;
	uint8_t x2lapicnmi;
	uint8_t gicc;
	uint8_t gicd;
	uint8_t gicr;
	uint8_t its;
	uint8_t mpwu;
}; // Possible to combine some of these and reduce size by half?
   // Could make coding it annoying however... Its only an extra few bytes

uint8_t validate_table(struct ACPISDTHeader *h);
void load_rsdt_store(struct ACPISDTHeader *rsdt_addr);

void *cache_sdt_by_signature(char *signature);

void cache_tables();
void *get_madt_entry(uint8_t id, uint8_t num);
void *get_mcfg_entry(uint8_t num); // Note: Num does nothing right now

#endif
