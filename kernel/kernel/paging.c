#include "header/paging.h"
#include "kernel.h"
#include "header/pfa.h"
#include "header/string.h"
#include <stddef.h>

static uint32_t kernel_pd[1024] __attribute__((aligned(4096))); // Aligned to the 
static uint32_t kernel_pt[1024] __attribute__((aligned(4096)));
static uint32_t test_pt[1024] __attribute__((aligned(4096)));

void copy_kernel_map(uint32_t* new_kpd)
{
	uint32_t pdx = PDLOC(VIRTADDR(KSTART));
	new_kpd[pdx] = PHYSADDR(kernel_pd) | F_PRES | F_RW;
}

void* init_page_directory() {
	uint32_t* pd = kalloc();
	//log_integer_to_serial((uint32_t)pd);	
//	copy_kernel_map((uint32_t)&pd);
	//pd[1023] = (uintptr_t)&pd;
	return (void*) pd;
}

void newinit_pd(uint32_t* pd)
{
	int i;
	for (i = 0; i < 1024; i++) {
		pd[i] = 0x2;
	}
	pd[1023] = (uint32_t) pd;
}

void init_pt(uint32_t* pt)
{
	uint32_t i;
	for (i = 0; i < 1024; i++)
	{
		pt[i] = (i * 0x1000) + 0x200000 | 2;
	}
}

void load_page_directory(uint32_t* pdphys)
{
	asm volatile("mov %0, %%cr3" : : "r"(pdphys));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

void pg_init(uintptr_t *entry_pd)
{
//	kinit();
	newinit_pd(kernel_pd);
	memcpy(kernel_pd, entry_pd, 4096);
	log_to_serial("Hello World!\n");
	init_pt(kernel_pt);
	kernel_pt[0] = kernel_pt[0] | F_PRES;
	kernel_pd[1020] = ((uint32_t)PHYSADDR((uint32_t)kernel_pt)) | 3;
	kernel_pd[1023] = (uint32_t) kernel_pd;
	log_integer_to_serial((uint32_t) entry_pd);
	load_page_directory((uint32_t*)PHYSADDR((uint32_t)kernel_pd));
}
