/* AHCI_C
 * WORDOS
 *
 * 11/19/2025
 */

#include "ahci.h"
#include "../kernel.h"
#include "../pci.h"
#include "../../memory/vmm.h"
#include "../../memory/pmm.h"
#include "../../memory/paging.h"
#include "../../memory/heap.h"
#include "../../memory/string.h"
#include "../../utils/arraylist.h"

static array_list_t *sata_ports; 

static inline uint32_t ata_disk_size(struct ahci_ram_ports *port)
{
	return (port->sector_words * 2) * port->lba_count;
}

<<<<<<< HEAD
unsigned int find_cmd_slot(struct ahci_ram_ports *port)
{	
	uint32_t slots = (port->port->pxsact | port->port->pxci);	

	for (int i = 0; i < 32; i++) { // Could add a number here to count ports...
		if ((slots&1) == 0)
			return i;

		slots >>= 1;
	}

	panic("No AHCI Free port found!\n");
}

=======
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
uint32_t check_device(struct HBA_ports *port)
{
	
	uint32_t ssts = port->pxssts;
	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (ipm != HBA_PORT_IPM_PRESENT) {
		return AHCI_DEV_NULL;
	}

	if (det != HBA_PORT_DET_PRESENT) {
		return AHCI_DEV_NULL;
	}

	switch (port->pxsig) {
		case SATA_SIG_ATA:
			return AHCI_DEV_SATA;
		case SATA_SIG_ATAPI:
			return AHCI_DEV_SATAPI;
		case SATA_SIG_SEMB:
			return AHCI_DEV_SEMB;
		case SATA_SIG_PM:
			return AHCI_DEV_PM;
		default:
			return AHCI_DEV_NULL;
	}
}

void start_commands(struct HBA_ports *port)
{
	while (port->pxcmd & PxCMD_CR) ;
	port->pxcmd |= PxCMD_FRE;
	port->pxcmd |= PxCMD_ST;
}

void stop_commands(struct HBA_ports *port)
{
	port->pxcmd &= ~PxCMD_ST;
	port->pxcmd &= ~PxCMD_FRE;

	while (1) {
		if (port->pxcmd & PxCMD_FR)
			continue;
		if (port->pxcmd & PxCMD_CR)
			continue;
		break;
	}
}

<<<<<<< HEAD
void post_ata_command(struct ahci_ram_ports *ram_port, uint64_t lba, uint32_t count, uint32_t *buffer, uint8_t command)
{
	int slot = find_cmd_slot(ram_port);
	uint32_t buf_offset = (uint32_t) buffer & 0xfff;
	uint64_t buf_phys = get_phys_from_virt(buffer) + buf_offset;

	struct HBA_CMD_TBL *cmd_tbl = (struct HBA_CMD_TBL *) (ram_port->ctlb_addr);
	cmd_tbl += slot;
	memset(cmd_tbl, 0, sizeof(struct HBA_CMD_TBL) + 0x20);
	
	struct FIS_REG_H2D *cmd = (struct FIS_REG_H2D *) &cmd_tbl->cfis;
	memset(cmd, 0, sizeof(struct FIS_REG_H2D));
	memset((void *) ram_port->fb_addr, 0, 0x200);

	int spin = 0;
	ram_port->port->pxis = 0xffffffff; // clear is bits
	ram_port->port->pxie = 0xffffffff;
						   
	struct HBA_CMD_HEADER *cmd_header = (struct HBA_CMD_HEADER *) ram_port->clb_addr;
	cmd_header += slot;
	cmd_header->FIS_flags = sizeof(struct FIS_REG_H2D) / sizeof(uint32_t);

	if (command == ATA_CMD_DMA_READ_EXT) {
	   	cmd_header->FIS_flags &= ~(1 << 6);	 // WRITE BIT
	} else if (command == ATA_CMD_DMA_WRITE_EXT) {
		logf("WRITE COMMAND\n");
		cmd_header->FIS_flags |= (1 << 6);
	} else {
		panic("Command not supported\n");
	
	}
	//cmd_header->FIS_flags &= ~(1 << 10); // C_OK when done
	// TODO: Change this to align with the numbers
	cmd_header->prdtl = ((count-1)>>4) + 1; // 512 Bytes per sector (hmmm) and 16 sectors per 8k bytes (max in prd) 
	cmd_header->prdbc = 0;

	cmd->fis_type = 0x27;
	cmd->command = command;
	cmd->device = 1 << 6;
	cmd->p_flags = 0;
	cmd->p_flags |= (1 << 7);

	cmd->lba0 = (uint8_t) lba & 0xFF;
	cmd->lba1 = (uint8_t) (lba >> 8) & 0xFF;
	cmd->lba2 = (uint8_t) (lba >> 16) & 0xFF;
	cmd->lba3 = (uint8_t) (lba>> 24) & 0xFF;
	cmd->lba4 = (uint8_t) 0;
	cmd->lba5 = (uint8_t) 0;
	cmd->countl = count;
	cmd->counth = 0;
	
	int entry_val = 0;
	for (;entry_val < cmd_header->prdtl; entry_val++) {
		cmd_tbl->entries[entry_val].dba = (uint32_t) (buf_phys & 0xffffffff);
		cmd_tbl->entries[entry_val].dbau = (uint32_t) (buf_phys >> 31);
		cmd_tbl->entries[entry_val].dw3 = (1024 * 8) - 1;
		
		buf_phys += 4 * 1024;
		count-=16;
	}
	cmd_tbl->entries[entry_val-1].dw3 = (0x2 << 10) | 1 << 31;

	logf("COMMAND TBL ENTRY %x\n", cmd_tbl->entries[0].dba);

	ram_port->port->pxci |= 1;

	int i =0;
	while (i < 10000000 && ram_port->port->pxci & (1 << slot)) {i++;}
	if (i == 10000000) { panic("SATA Port timed out"); return; }
}

=======
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
void init_ahci_cmd_port()
{
	// TODO: Devise a real method that wont waste pages on aligned physical addresses
	struct ahci_ram_ports *current = 0;
	for (int i = 0; i < sata_ports->count; i++) {
		current = ((struct ahci_ram_ports *)al_get(sata_ports, i));

		stop_commands(current->port);

		void *base_addr = pae_kalloc(4096 * 3, PG_PRES | PG_RW, 0); // 3 Pages needed for each port.
		void *cmd_list_page = base_addr + 4096;
		uint64_t cmd_list_phys = get_phys_from_virt(cmd_list_page);
		uint64_t base_phys = get_phys_from_virt(base_addr);

		current->port->pxclb = (uint32_t) base_phys;	
		current->clb_addr = (uint32_t) base_addr;
		current->ctlb_addr = (uint32_t) cmd_list_page;
		memset(base_addr, 0, 1024);

		current->port->pxfb = (uint32_t) (base_phys + 0x400); // Size of the headers
		current->fb_addr = (uint32_t) (base_addr + 0x400);
		memset((void *) (base_addr + 0x400), 0, 256);
		

		for (int j = 0; j < 32; j++) {
			struct HBA_CMD_HEADER *header = base_addr + (j * sizeof(struct HBA_CMD_HEADER));
			header->prdtl = 8;
			header->ctba = (uint32_t) (cmd_list_phys + (j * sizeof(struct HBA_CMD_TBL)));
			header->ctbau = 0;

			memset((void *)(cmd_list_page + (j * sizeof(struct HBA_CMD_TBL))), 0, 256);
		}

		start_commands(current->port);
	}
}

void handle_ahci_interrupt()
{	
	struct ahci_ram_ports *ident_port = al_get(sata_ports, 0);
	struct HBA_ports *port = ident_port->port;

	logf("PORT IS IS %x\n", port->pxis);
	logf("PORT IS ERR %x\n", port->pxserr);
	// TODO: Add if statement for types
	

}

void probe_ports(struct HBA_mem *hba)
{
	uint32_t pi = hba->GHC.Ports;

	struct HBA_ports *cur_port = 0;
	
	uint8_t i = 0;
	while (pi) {
		if (pi & 1) {
			cur_port = &(hba->ports[i]);
			// Check type of device at port i
			if (check_device(cur_port) == AHCI_DEV_SATA) {
				struct ahci_ram_ports storage = {cur_port, (uint32_t) 0};
				al_add_item(&sata_ports, &storage);
				logf("STOR %x\n", storage);
			}
			logf("Signature type: %x\n", check_device(cur_port));   	
		}	
		pi >>= 1;
		i++;
	}
}

<<<<<<< HEAD
void reset_port(struct ahci_ram_ports *ram_port)
{
	struct HBA_ports *port = ram_port->port;

	struct HBA_CMD_HEADER *header1 = (struct HBA_CMD_HEADER *) ram_port->clb_addr;
	struct HBA_CMD_TBL *cmd_tbl1 = (struct HBA_CMD_TBL *) (ram_port->ctlb_addr);
	struct FIS_REG_H2D *cmd1 = (struct FIS_REG_H2D *) &cmd_tbl1->cfis;
	header1->FIS_flags = sizeof(struct FIS_REG_H2D) / sizeof(uint32_t);
	header1->FIS_flags |= (1 << 10);
	header1->FIS_flags |= (1 << 8);
	header1->prdtl = 0;

	struct HBA_CMD_HEADER *header2 = (struct HBA_CMD_HEADER *) ram_port->clb_addr + 1;
	struct HBA_CMD_TBL *cmd_tbl2 = (struct HBA_CMD_TBL *) (ram_port->ctlb_addr) + 1;
	struct FIS_REG_H2D *cmd2 = (struct FIS_REG_H2D *) &cmd_tbl2->cfis;
}

=======
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
void read_identify(uint16_t *data, struct ahci_ram_ports *ram_port)
{
	ram_port->sig = data[ATA_IDENTIFY_SIGNATURE];
	ram_port->cap = data[ATA_IDENTIFY_CAP];

	ram_port->cmd_set = data[ATA_IDENTIFY_CMD_SET + 0] << 0;
	ram_port->cmd_set |= data[ATA_IDENTIFY_CMD_SET + 1] << 16;

	if (ram_port->cmd_set & ATA_CMDSET_48SUPPORT) {
		logf("48 bit supported. EXT used\n");
		ram_port->lba_count = data[ATA_IDENTIFY_LBA_COUNT_EXT + 0] << 0;
		ram_port->lba_count |= data[ATA_IDENTIFY_LBA_COUNT_EXT + 1] << 16;
		ram_port->lba_count |= data[ATA_IDENTIFY_LBA_COUNT_EXT + 2] << 32;
		ram_port->lba_count |= data[ATA_IDENTIFY_LBA_COUNT_EXT + 3] << 48;
	} else {
		ram_port->lba_count = data[ATA_IDENTIFY_LBA_COUNT] << 0;
		ram_port->lba_count |= data[ATA_IDENTIFY_LBA_COUNT+ 1] << 16;
	}
	
	if (data[ATA_IDENTIFY_SECTOR_INFO] & (1 << 12)) {
		ram_port->sector_words = data[ATA_IDENTIFY_SECTOR_WORDS + 0] << 0;	
		ram_port->sector_words |= data[ATA_IDENTIFY_SECTOR_WORDS + 1] << 16;	
	} else {
		ram_port->sector_words = 256;
	}

	uint64_t total_bytes = ata_disk_size(ram_port);

	for (int i = 0; i < 20; i++) {
		uint16_t word = data[ATA_IDENTIFY_MODEL + i];
		ram_port->model[i * 2 + 0] = word >> 8;
		ram_port->model[i * 2 + 1] = word & 0xFF;
	}

	ram_port->model[40] = 0;

	uint16_t model_len = 40;
	while (model_len > 0 && ram_port->model[model_len - 1] == ' ') model_len--;
	ram_port->model[model_len] = 0;
	logf("Disk %s loaded with size %ld bytes (%ld MiB)\n", ram_port->model, total_bytes, total_bytes / 1024 / 1024);
}

void identify()
{
	uint64_t buf_phys = alloc_phys_page();
	char *buffer = pae_kalloc(4096, PG_PRES | PG_RW | PG_PCD, buf_phys);
	struct ahci_ram_ports *ram_port = al_get(sata_ports, 0);
	struct HBA_ports *port = ram_port->port;
	port->pxie = 0xffffffff;

	struct HBA_CMD_TBL *cmd_tbl = (struct HBA_CMD_TBL *) (ram_port->ctlb_addr);

	cmd_tbl->entries[0].dba = (uint32_t)buf_phys;
	cmd_tbl->entries[0].dbau = 0;
	cmd_tbl->entries[0].dw3 = 511 | (1 << 31);

	struct HBA_CMD_HEADER *header = (struct HBA_CMD_HEADER *) ram_port->clb_addr;
	logf("PHYS ADDR : %x\n", get_phys_from_virt(header));
	header->FIS_flags = sizeof(struct FIS_REG_H2D) / sizeof(uint32_t);
	header->FIS_flags &= ~(1 << 6);
	header->FIS_flags |= (1 << 10);
	header->prdtl = 1;


	struct FIS_REG_H2D *cmd = (struct FIS_REG_H2D *) &cmd_tbl->cfis;
	
	cmd->fis_type = 0x27;
	cmd->command = 0xEC;
	cmd->device = 0;
	cmd->p_flags |= (1 << 7);
	port->pxci |= 1;
	int i =0;
	while (i < 10000000 && port->pxci & 1) {i++;}
	if (i == 10000000) { panic("SATA Port timed out"); return; }

	struct FIS_PIO_D2H *identify = (struct FIS_PIO_D2H *) (ram_port->fb_addr + 0x20);
	read_identify((uint16_t *) buffer, ram_port);
	
}

<<<<<<< HEAD
void *ata_read(struct ahci_ram_ports *ram_port, uint32_t startl, uint32_t starth, uint32_t count, void *buf)
{
//	uint64_t buf_phys = alloc_phys_page();
//	char *buf = pae_kalloc(4096, PG_PRES | PG_RW | PG_PCD, buf_phys);
	uint32_t buf_offset = (uint32_t) buf & 0xfff;
	uint64_t buf_phys = get_phys_from_virt(buf) + buf_offset;
	memset((void *) ram_port->fb_addr, 0, 0x200);
	logf("FB AT %x\n", ram_port->fb_addr);

	logf("PORT SSTS: %x\nPORT TFD: %x\nPORT CMD: %x\n", ram_port->port->pxssts, ram_port->port->pxtfd, ram_port->port->pxcmd);
=======
void ata_read(struct ahci_ram_ports *ram_port, uint32_t startl, uint32_t starth, uint32_t count, void *buf)
{
	memset((void *) ram_port->fb_addr, 0, 0x200);
	uint32_t buf_offset = (uint32_t) buf & 0xfff;
	uint64_t buf_phys = get_phys_from_virt(buf) + buf_offset;
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
	
	int spin = 0;
	ram_port->port->pxis = 0xffffffff; // clear is bits
	ram_port->port->pxie = 0xffffffff;
						   //
	struct HBA_CMD_HEADER *cmd_header = (struct HBA_CMD_HEADER *) ram_port->clb_addr;
<<<<<<< HEAD
	cmd_header->FIS_flags = sizeof(struct FIS_REG_H2D) / sizeof(uint32_t);
	cmd_header->FIS_flags |= (1 << 6);	 // WRITE BIT
	//cmd_header->FIS_flags &= ~(1 << 10); // C_OK when done
	// TODO: Change this to align with the numbers
	cmd_header->prdtl = 2;
=======
	//memset(cmd_header, 0, sizeof(struct HBA_CMD_HEADER));
	cmd_header->FIS_flags = sizeof(struct FIS_REG_H2D) / sizeof(uint32_t);
	cmd_header->FIS_flags &= ~(1 << 6);
	cmd_header->FIS_flags |= (1 << 10);
	// TODO: Change this to align with the numbers
	cmd_header->prdtl = 1;
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
	cmd_header->prdbc = 0;
	
	struct HBA_CMD_TBL *cmd_tbl = (struct HBA_CMD_TBL *) (ram_port->ctlb_addr);
	memset(cmd_tbl, 0, sizeof(struct HBA_CMD_TBL) + 0x20);
<<<<<<< HEAD
	int entry_val = 0;
	for (;entry_val < cmd_header->prdtl; entry_val++) {
		cmd_tbl->entries[entry_val].dba = (uint32_t) (buf_phys & 0xffffffff);
		cmd_tbl->entries[entry_val].dbau = (uint32_t) (buf_phys >> 31);
		cmd_tbl->entries[entry_val].dw3 = (1024 * 8) - 1;
		
		buf_phys += 4 * 1024;
		count-=16;
	}
		//cmd_tbl->entries[0].dba = (uint32_t) (buf_phys & 0xffffffff);
		//cmd_tbl->entries[0].dbau = (uint32_t) (buf_phys >> 31);
		cmd_tbl->entries[0].dw3 = 0x2 << 10;

	logf("COMMAND TBL ENTRY %x\n", &cmd_tbl->entries[1].dw3);
=======
	cmd_tbl->entries[0].dba = (uint32_t) (buf_phys + buf_offset);
	cmd_tbl->entries[0].dbau = (buf_phys >> 31);
	cmd_tbl->entries[0].dw3 = (0x100 - 1) | (1 << 31);
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
		
	struct FIS_REG_H2D *cmd = (struct FIS_REG_H2D *) &cmd_tbl->cfis;
	memset(cmd, 0, sizeof(struct FIS_REG_H2D));

	cmd->fis_type = 0x27;
<<<<<<< HEAD
	cmd->command = 0x35;
	cmd->device = 1 << 6; // LBA MODE
	cmd->p_flags = 0;
	cmd->p_flags |= (1 << 7); // Command Flag
=======
	cmd->command = 0xc8;
	cmd->device = 1 << 6;
	cmd->p_flags = 0;
	cmd->p_flags |= (1 << 7);
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f

	cmd->lba0 = (uint8_t) startl;
	cmd->lba1 = (uint8_t) (startl >> 8);
	cmd->lba2 = (uint8_t) (startl >> 16);
	cmd->lba3 = (uint8_t) (startl >> 24);
	cmd->lba4 = (uint8_t) 0;
	cmd->lba5 = (uint8_t) 0;
<<<<<<< HEAD
	cmd->countl = 2;
	cmd->counth = 0;

=======
	cmd->countl = 1;
	cmd->counth = 1;

	logf("Current pxpis %x\n", ram_port->port->pxtfd);
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
	ram_port->port->pxci |= 1;
	int i =0;
	while (i < 10000000 && ram_port->port->pxci & 1) {i++;}
	if ( i== 10000000) panic("HELD DISK\n");
<<<<<<< HEAD
	struct FIS_PIO_D2H *identify = (struct FIS_PIO_D2H *) (ram_port->fb_addr);
	
	logf("PORT SSTS: %x\nPORT TFD: %x\nPORT CMD: %x\n", ram_port->port->pxssts, ram_port->port->pxtfd, ram_port->port->pxcmd);
	logf("FB AT %x\n", identify);
	logf("FIS AT %x\n", cmd);
=======
	struct FIS_PIO_D2H *identify = (struct FIS_PIO_D2H *) (ram_port->fb_addr + 0x40);
	
	logf("FB AT %x\n", ram_port->port->pxssts);
	logf("PRDTB %x\n", cmd_tbl);
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
}

void init_ahci_controller()
{
	sata_ports = create_array_list(sizeof(struct ahci_ram_ports));
	uint32_t ahci_head = (uint32_t) get_ahci_BAR5();
	
	uint32_t *page = pae_kalloc(4096, PG_RW | PG_PCD | PG_PRES, (uint32_t) ahci_head);	
	
	struct HBA_mem *header = (struct HBA_mem *) page;		

	// Something we can do is look at GHS.CAP.PO to check, but im #lazy

	header->GHC.GlobHostCtrl |= 1 << 31 | 1 << 1; // Sets into AHCI Mode

	probe_ports(header);
	init_ahci_cmd_port();
	identify();

<<<<<<< HEAD
	logf("Header Caps: %x\n", header->GHC.Capabilities);

	header->GHC.GlobHostCtrl |= 1 << 31 | 1 << 1; // Sets into AHCI Mode

	struct ahci_ram_ports *ram_port = al_get(sata_ports, 0);

	reset_port(ram_port);
	char *read_buffer = kalloc(0x500);
	post_ata_command(ram_port, 0, 1, (void* )read_buffer, ATA_CMD_DMA_READ_EXT);

	read_ahci_status();
	//logf("PER SECTOR: %x\n", ram_port->sector_words);
	for (int i = 0; i < 0x500; i++) {
		if (*((uint8_t *) read_buffer + i))
			logf("%c",*((uint8_t*) read_buffer + i));
=======
	struct ahci_ram_ports *ram_port = al_get(sata_ports, 0);
	void *read_buffer = kalloc(0x500);
	ata_read(ram_port, 0, 0, 5, read_buffer); 
	ata_read(ram_port, 0, 0, 5, read_buffer); 
	logf("PER SECTOR: %x\n", ram_port->sector_words);
	logf("Buffer addr: %x\n", read_buffer);
	for (int i = 0; i < 0x200; i++) {
		logf("%x ",*((uint8_t*) read_buffer + i));
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
	}
}


