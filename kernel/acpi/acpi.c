/* ACPI_C
 *
 * Holds ACPI table info
 *
 * WordOS
 * 11/07/2025
 */

#include "acpi.h"
#include "tables.h"
#include "../kernel/kernel.h"
#include "../memory/vmm.h"
#include "../memory/paging.h"
#include "../memory/heap.h"
#include "../memory/string.h"

/*
 * All tables should be stored in kernel heap. This is to prevent weird page boundry issues when searching and grabbing them.
 * My laptop hates me.
 *
 */

static struct ACPISDTHeader* rsdt_store;

struct MADTHeader *madt_header; 
struct MADTEntryCount madt_counts = {0};

struct MCFGHeader *mcfg_header;

uint8_t validate_table(struct ACPISDTHeader *h)
{
	uint32_t total = 0;
	for (uint8_t i = 0; i < h->Length; i++) {
		total += ((char*) h)[i];
	}

	return total == 0;
}

void load_rsdt_store(struct ACPISDTHeader *rsdt_addr)
{
	rsdt_store = rsdt_addr;
}

void *cache_sdt_by_signature(char *signature)
{
	struct RSDT {
		struct ACPISDTHeader h;
		uint32_t NextSDT[(rsdt_store->Length - sizeof(struct ACPISDTHeader)) / 4];
	};
	struct RSDT* rsdt = (struct RSDT *) rsdt_store;

	int entries = (rsdt->h.Length - sizeof(struct ACPISDTHeader)) / 4;
	struct ACPISDTHeader* found = 0;
	for (int i = 0; i < entries; i++) {
		struct ACPISDTHeader* potential = (struct ACPISDTHeader *) rsdt->NextSDT[i];
		memory_map(current_vmm->root_pd, (uint32_t*) PGROUNDDOWN((uint32_t) potential), \
				(uint32_t*) PGROUNDDOWN((uint32_t) potential), 0x3);
		if (strcmp(signature, potential->Signature, 4)) {
			found = potential;
			break;
		}
	}

	if (!found) {
		panic("Signature not found in ACPI tables\n");
		return NULL;
	}

	uint32_t *store = kalloc(found->Length);
	memcpy(store, found, found->Length);

	return (void *) store;
}

void count_madt_entries()
{	
	uint8_t *start = (uint8_t *) &madt_counts;
	for (struct MADTEntryHead *entry = &(madt_header->entries); 
			(uint32_t) entry < (uint32_t) madt_header + madt_header->header.Length;
			entry = (struct MADTEntryHead *) ((uint32_t) entry + entry->Length)) {
		*(start + entry->EntryType) += 1;
	}	
}

void *get_madt_entry(uint8_t id, uint8_t num)
{
	uint8_t count = 0;
	
	for (struct MADTEntryHead *entry = &(madt_header->entries); 
			(uint32_t) entry < (uint32_t) madt_header + madt_header->header.Length;
			entry = (struct MADTEntryHead *) ((uint32_t) entry + entry->Length)) {
		if (entry->EntryType == id) {
			if (count == num) {
				return (void *) entry + sizeof(struct MADTEntryHead);
			}
			count++;
		}
	}	

	logf("No MADT Entry found with id %x and count %x\n", id, count);
	return NULL;
}

void *get_mcfg_entry(uint8_t num)
{
	struct MMECS *ret_entry = 0;
	for (struct MMECS *entry = &(mcfg_header->config_space);
		(uint64_t) entry < (uint64_t) mcfg_header + mcfg_header->header.Length;
		entry = (struct MMECS *) ((uint64_t) entry + sizeof(struct MMECS))) {
		ret_entry = entry;
		logf("Entry found with addr: %x\n", entry->base_addr); 
	}	
		return ret_entry;
}

void cache_tables()
{
	madt_header = cache_sdt_by_signature("APIC");	
	logf("MADT_HEADER LENGTH: %x\n", &(madt_header->entries));

	count_madt_entries();
	logf("0x2 found %x times\n", madt_counts.iso);

	logf("MADT cached into memory\n");

	mcfg_header = cache_sdt_by_signature("MCFG");

	logf("MCFG cached into memory\n");
	get_mcfg_entry(0);
}
