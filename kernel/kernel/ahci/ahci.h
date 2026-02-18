#ifndef AHCI_H
#define AHCI_H

#include <stdint.h>

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101	// Port multiplier
								//  IDK how these signatures are gotten lmaoo

#define ATA_CMD_DMA_WRITE_EXT	0x35 // TODO: REPLACE WITH VALUE
#define ATA_CMD_DMA_READ_EXT	0x25
#define ATA_CMD_DMA_READ		0x3c //TODO: FIND REAL VALUE

#define HBA_PORT_IPM_PRESENT	1
#define HBA_PORT_DET_PRESENT	0x3

#define AHCI_DEV_NULL	0
#define AHCI_DEV_SATA	1
#define AHCI_DEV_SATAPI	2
#define AHCI_DEV_SEMB	3
#define AHCI_DEV_PM		4

#define PxCMD_ST	(1 << 0)
#define PxCMD_SUD	(1 << 1)
#define PxCMD_POD	(1 << 2)
#define PxCMD_CLO	(1 << 3)
#define PxCMD_FRE	(1 << 4)
#define PxCMD_MPSS	(1 << 13)
#define PxCMD_FR	(1 << 14)
#define PxCMD_CR	(1 << 15)
#define PxCMD_CPS	(1 << 16)
#define PxCMD_ATAPI	(1 << 24)

#define PRDT_INT	(1 << 31)

enum FIS_TYPE {
	reg_h2d = 0x27,
	reg_d2h = 0x34,
	dma_d2h = 0x39,
	dma_setup = 0x41,
	data = 0x46,
	bist_act = 0x58,
	pio_d2h = 0x5f,
	sdb_d2h = 0xa1
};

struct HBA_GHS {
	uint32_t Capabilities;
	uint32_t GlobHostCtrl;
	uint32_t InterruptStatus;
	uint32_t Ports;
	uint32_t Version;
	uint32_t CC_CTL;
	uint32_t CC_PORTS;
	uint32_t EM_LOC;
	uint32_t EM_CTL;
	uint32_t XCapabilities;
	uint32_t BOHC;
};

struct HBA_ports {
	// Note: Serial ATA -> SATA
	uint32_t pxclb; // Port x Command List Base Address
	uint32_t pxclbu; // upper 32 ^
	uint32_t pxfb; // P x FIS base
	uint32_t pxfbu; // upper 32 ^
	uint32_t pxis; // P x Interrupt Stat
	uint32_t pxie; // P x Interrupt Enable
	uint32_t pxcmd; //P x Command and Stat
					
	uint32_t resv0;

	uint32_t pxtfd; // P x Task File Data
	uint32_t pxsig; // P x Signature
	uint32_t pxssts; //P x Serial ATA Stat
	uint32_t pxsctl; // P x Serial ATA Control
	uint32_t pxserr; // P x Serial ATA Err
	uint32_t pxsact; // Port x Serial ATA Active
	uint32_t pxci;	 // Port x Command Iss
	uint32_t pxsntf; // Port x Serial ATA Notif
	uint32_t pxfbs;  // Port x FIS-based switching ctrl
	uint32_t pxdevslp; // P x Device Sleep
	uint8_t resv1[11];
	uint32_t pxvs[4]; // Port x Vendor
};

struct HBA_mem {
	struct HBA_GHS GHC;

	uint8_t resv[0xA0-0x2C];

	uint8_t vendor_regs[0xFF-0xA0];

	struct HBA_ports ports[1]; // HBA Port registers 1 - 32
};

#define HBA_HEAD_ATAPI		(1 << 5)
#define HBA_HEAD_WRITE		(1 << 6)
#define HBA_HEAD_PF			(1 << 7)
#define HBA_HEAD_RESET 		(1 << 8)
#define HBA_HEAD_BIST		(1 << 9)
#define HBA_HEAD_CLEAR		(1 << 10)

volatile struct HBA_CMD_HEADER {
	uint16_t FIS_flags; /*
						0:4 - Command FIS Length
						5 - ATAPI
						6 - Write (1: H2D, 0: D2H)
						7 - prefetchable
						8 - reset
						9 - BIST
						10 - C busy on OK
						12:15 Port Multiplier
						*/
	uint16_t prdtl;
	volatile 
	uint32_t prdbc;

	uint32_t ctba;
	uint32_t ctbau;

	uint32_t rsv1[4];
};

struct HBA_PRDT_ENTRY {
	uint32_t dba;
	uint32_t dbau;
	uint32_t rsv0;
	uint32_t dw3;
};

volatile struct HBA_CMD_TBL {
	uint8_t cfis[64];

	uint8_t acmd[16];

	uint8_t rsv[48];

	// 0x80
	struct HBA_PRDT_ENTRY entries[8]; // We will define 8 entries	
};

#define ATA_IDENTIFY_SIGNATURE		0
#define ATA_IDENTIFY_CAP			49
#define ATA_IDENTIFY_MODEL			27
#define ATA_IDENTIFY_CMD_SET		82
#define ATA_IDENTIFY_SECTOR_WORDS	117
#define ATA_IDENTIFY_SECTOR_INFO	106
#define ATA_IDENTIFY_LBA_COUNT		60
#define ATA_IDENTIFY_LBA_COUNT_EXT	100

#define ATA_CMDSET_48SUPPORT		(1 << 26)

struct ahci_ram_ports {
	struct HBA_ports *port;
	uint32_t clb_addr;
	uint32_t ctlb_addr;
	uint32_t fb_addr;
	uint16_t sig;
	uint16_t cap;
	uint32_t cmd_set;
	uint32_t sector_words;
	uint64_t lba_count;
	char model[41];
};

// SATA FIS STRUCTS


#define FIS_H2D_CLEAR		(1 << 7)

volatile struct FIS_REG_H2D {
	uint8_t fis_type;
	uint8_t p_flags;

	uint8_t command;
	uint8_t featurel;

	// DWORD 1
	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;

	// DWORD 2	
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t featureh;

	// DWORD 3
	uint8_t countl;
	uint8_t counth;
	uint8_t icc;
	uint8_t control;

	uint8_t rsv1[4];
};

struct FIS_PIO_D2H {
	uint8_t fis_type;
	uint8_t p_flags;
	uint8_t status;
	uint8_t error;

	// DWORD 1
	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;

	// DWORD 2
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t rsv0;

	// DWORD 3
	uint8_t countl;
	uint8_t counth;
	uint8_t rsv1;
	uint8_t e_status;

	// DWORD 4
	uint16_t trans_count;
	uint16_t rsv2;
};

volatile struct FIS_DMA_D2H {
	uint8_t fis_type;
	uint8_t flags; // 0-3 : Port multiplier
				   // 4 : RSV
				   // 5 : 1 -> d2h, 0 -> h2d
				   // 6 : interrupt bit
				   // 7 : auto-activate
	uint8_t rsv0[2];

	// DWORD 1 & 2
	uint64_t DMA_buffer_id;

	// DWORD 3
	uint32_t rsvd;

	// DWORD 4
	uint32_t DMA_buffer_offset;

	// DWORD 5
	uint32_t trans_count;
	
	// DWORD 6
	uint32_t rsv1;
};

void init_ahci_controller();
void handle_ahci_interrupt();

static inline uint32_t ata_disk_size(struct ahci_ram_ports *port);
#endif
