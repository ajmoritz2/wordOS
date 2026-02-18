<<<<<<< HEAD
/* 
 * PCI Functionality
 *  WordOS
 *  ajmoritz
 *  11/12/2025
 */

#include "../programs/terminal.h"
#include "../memory/paging.h"
#include "../memory/vmm.h"
#include "../memory/pmm.h"
#include "../acpi/tables.h"
#include "../acpi/acpi.h"
#include "ahci/ahci.h"
#include "pci.h"
#include "kernel.h"

static struct PCIDevType0 *AHCIAddr = 0;

void handle_bus_header(struct PCIDevType1 *header)
{
}

void handle_device(struct PCIDevType0 *header)
{
	if (header->header.ClassCode == 0x1) {
		printf("MASS MEMORY Controller\n");
		if (header->header.SubClass == 0x6) {
			printf("SATA IDENTIFIED\n");
			if (header->header.ProgIF == 0x1) {
				printf("AHCI Controller HOORAYY!!!!\n");
				printf("Has ABAR: %x\n", header->BAR5 & ~0x3ff);
				AHCIAddr = header;
			} else {
				printf("ProgIF Found: %x\n", header->header.ProgIF);
			}
		}
	}
}

void ahci_action()
{
	if (!(AHCIAddr->header.Status & (1 << 4))) {
		logf("MSI Not supported by AHCI Device\n");
		return;
	}
		
	uint8_t cap_offset = AHCIAddr->CapabilitiesP & 0xfc;
	struct PCICapability *cap_pointer = (struct PCICapability *) ((uint32_t) AHCIAddr + cap_offset);
	struct PCICapability *base = (struct PCICapability *) ((uint32_t) AHCIAddr + cap_offset);

	struct PCIMSI32Header *msi_pointer = 0;
	struct PCIMSI64Header *msi64_pointer = 0;

	while (cap_pointer->Next) {
		if (cap_pointer->CapID == 0x5) {
			msi_pointer = (struct PCIMSI32Header *) cap_pointer;
			logf("MSI Found!\n");
			break;
		}
		cap_pointer = (struct PCICapability *) ((uint32_t) base + cap_pointer->Next);
	}
	
	if (!cap_pointer) {
		panic("NO MSI! INT instr not implemented!\n");
		return;
	}

	msi_pointer->MessageControl |= 0x1;

	logf("MSI AHCI: %x\n", msi_pointer);
	
	// TODO: Add 32 bit support, and create an interrupt page. Also create a vector allocator (easy enough just count up...)
	if (msi_pointer->MessageControl & (1 << 7)) {
		// 64-bit structure
		msi64_pointer = (struct PCIMSI64Header *) msi_pointer;	

		msi64_pointer->UMessageAddress = 0;
		msi64_pointer->XMessageData = 0;
		msi64_pointer->MessageData = 0x33; // Interrupt vector
		uint64_t phy_msg = alloc_phys_page();
		uint32_t mes_addr = (uint32_t) pae_kalloc(4096, 0x3, phy_msg);
		logf("Mes_addr: %x\n", mes_addr);
		msi64_pointer->MessageAddress = (uint32_t) 0xFEE00300;
	}
}

void read_ahci_status(){
	logf("AHCI Status = %x\n", AHCIAddr->header.Status);
}

void brute_force(struct MMECS *config, int bus)
{
	static uint32_t base_virt = 0xd0000000;
	uint32_t base = 0;

	for (uint8_t device = 0; device < 32; device++) {
		base = config->base_addr + (bus << 20 | device << 15);
		pae_mmap(kernel_vmm->root_pdpt, (uint64_t) base, (uint32_t *) base_virt, 0x13);
		struct PCIDevHeader *header = (struct PCIDevHeader *) base_virt;
		base_virt += 4096;

		if (header->VendorId == 0xffff) {
			pae_unmap((uint32_t *) (base_virt - 4096));
			continue;	
		}
		printf("\nVendor ID: %x Device ID: %x, Class Code: %x, SubClass: %x\n", header->VendorId, header->DeviceId, header->ClassCode
					, header->SubClass);
		if (header->HeaderType != 0) {
			printf("PCI - PCI Connection\n");
			struct PCIDevType1 *pci_con = (struct PCIDevType1 *) header;
			if (pci_con->SecondaryBusNum == 0) {
				printf("Connection does not connect!\n");
				continue;
			} else {
				printf("Continueing with bus %x\n", pci_con->SecondaryBusNum);
				brute_force(config, pci_con->SecondaryBusNum);
				printf("Disconnected from bus %x\n", pci_con->SecondaryBusNum);
			}
		} else {
			handle_device((struct PCIDevType0 *) header);
		}
	}
}

void *get_ahci_BAR5()
{
	return (void *) AHCIAddr->BAR5;
}

void test_pci()
{
	struct MMECS *config = get_mcfg_entry(0);

	brute_force(config, 0);

	ahci_action();
	init_ahci_controller();
	panic("Done with PCI");
}
=======
>>>>>>> parent of 8fae1a0 (ahci stuff and acpi stuff. cant read disk yet)
