#ifndef VFAT_H
#define VFAT_H

#include <stdint.h>

struct fat_fs {
	void (*read_drive)(uint64_t lba, uint32_t count, uint32_t *buffer);
	void (*write_drive)(uint64_t lba, uint32_t count, uint32_t *buffer);
};

struct FAT_BPB {
	uint8_t jmpBoot[3];
	char OEMName[8];

	uint16_t BytsPerSec;
	uint8_t SecPerClus;
	uint16_t RsvdSecCnt;

	uint8_t NumFATs;
	uint16_t RootEntCnt;
	uint16_t TotSec16;

	uint8_t Media;
	
	uint16_t FATSz16;
	uint16_t SecPerTrk;
	uint16_t NumHeads;

	uint32_t HiddSec;
	uint32_t TotSec32;
};

struct FAT32_BS {
	uint32_t FATSz32;
	uint16_t ExtFlags;
	uint16_t FSVer;

	uint32_t RootClus;
	uint16_t FSInfo;
	uint16_t BkBootSec;

	uint8_t Rsv0[12];
	uint8_t DrvNum;
	uint8_t Rsv1;

	uint8_t BootSig;
	uint32_t VolID;

	uint8_t VolLab[11];
	char FilSysType[8];
};

typedef uint32_t FILE_D;

FILE_D open(const char * filename, uint32_t flags);
void close(FILE_D file);
void read(FILE_D file, char *buffer, uint32_t count);

#endif
