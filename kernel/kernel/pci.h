/*
 * PCI_H
 * wordOS
 * 08/18/25
 *
 * 	PCI BUS HEADER FILE
 */
<<<<<<< HEAD

#ifndef PCI_H
#define PCI_H

#include <stdint.h>

struct PCIDevHeader {
	uint16_t VendorId;
	uint16_t DeviceId;
	uint16_t Command;
	uint16_t Status;
	uint8_t RevisionId;
	uint8_t ProgIF;
	uint8_t SubClass;
	uint8_t ClassCode;
	uint8_t CacheLineSize;
	uint8_t LatencyTimer;
	uint8_t HeaderType;
	uint8_t BIST;
}__attribute__((packed));

struct PCIDevType0 {
	struct PCIDevHeader header;
	uint32_t BAR0;
	uint32_t BAR1;
	uint32_t BAR2;
	uint32_t BAR3;
	uint32_t BAR4;
	uint32_t BAR5;
	uint32_t CarbusCISP;
	uint16_t SubsysVendorId;
	uint16_t SubsysId;
	uint32_t XROMBase;
	uint8_t CapabilitiesP;
	uint8_t reserved;
	uint16_t reserved1;
	uint32_t reserved2;
	uint8_t InterruptLine;
	uint8_t InterruptPin;
	uint8_t Min_Gnt;
	uint8_t Max_Lat;
}__attribute__((packed));

struct PCIDevType1 {
	struct PCIDevHeader header;
	uint32_t BAR0;
	uint32_t BAR1;
	uint8_t SecondaryLatTimer;
	uint8_t SubordinateBusNum;
	uint8_t SecondaryBusNum;
	uint8_t PrimaryBusNum;
	uint16_t SecondaryStatus;
	uint8_t IOLimit;
	uint8_t IOBase;
	uint16_t MemoryLimit;
	uint16_t MemoryBase;
	uint16_t PrefetchMemLimit;
	uint16_t PrefetchMemBase;
	uint32_t PrefetchMemBaseHi;
	uint32_t PrefetchMemLimitHi;
	uint16_t IOBaseLimit16;
	uint16_t IOBaseUpper16;
	uint16_t reserved;
	uint8_t reserved1;
	uint8_t CapabilitiesP;
	uint32_t XROMBase;
	uint16_t BridgeControl;
	uint8_t InterruptPin;
	uint8_t InterruptLine;
}__attribute__((packed));

struct PCICapability {
	uint8_t CapID;
	uint8_t Next;
	uint16_t CapReg;
	uint32_t DevCap;
	uint16_t DevControl;
	uint16_t DevStatus;
	uint32_t LinkCap;
	uint16_t LinkControl;
	uint16_t LinkStatus;
	uint32_t SlotCap;
	uint16_t SlotControl;
	uint16_t SlotStatus;
	uint16_t RootControl;
	uint16_t RootCap;
	uint32_t RootStatus;
	uint32_t DevCap2;
	uint16_t DevControl2;
	uint16_t DevStatus2;
	uint32_t LinkCap2;
	uint16_t LinkControl2;
	uint16_t LinkStatus2;
	uint32_t SlotCap2;
	uint16_t SlotControl2;
	uint16_t SlotStatus2;
} __attribute__((packed));

struct PCIMSI32Header {
	uint8_t CapID;
	uint8_t Next;
	uint16_t MessageControl;
	uint32_t MessageAddress;
	uint16_t MessageData;
	uint16_t XMessageData;
} __attribute__((packed));

struct PCIMSI64Header {
	uint8_t CapID;
	uint8_t Next;
	uint16_t MessageControl;
	uint32_t MessageAddress;
	uint32_t UMessageAddress;
	uint16_t MessageData;
	uint16_t XMessageData;
} __attribute__((packed));


void test_pci();

void *get_ahci_BAR5();
void read_ahci_status();

#endif
=======
>>>>>>> parent of 8fae1a0 (ahci stuff and acpi stuff. cant read disk yet)
