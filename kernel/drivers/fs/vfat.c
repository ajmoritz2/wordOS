/*
 *	VFAT filesystem
 *	WordOS - ajmoritz
 *
 *	02/19/2025
 */
#include "../../utils/arraylist.h"
#include "../../memory/heap.h"
#include "../../kernel/ahci/ahci.h"
#include "vfat.h"

struct array_list *fat_list;

FILE_D open(const char * filename, uint32_t flags) 
{

}

void close(FILE_D file)
{

}

void read(FILE_D file, char *buffer, uint32_t count)
{

}

void init_fs()
{
	// Since we only have AHCI implemented, no need for anything special...
	fat_list = create_array_list(sizeof(struct fat_fs));	

	struct fat_fs *boot_fs = kalloc(sizeof(struct fat_fs));

	al_add_item(&fat_list, boot_fs);

	boot_fs->read_drive = &ahci_read;
	boot_fs->write_drive = &ahci_write;
}


