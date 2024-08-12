#include "header/paging.h"
#include "kernel.h"
#include "header/pfa.h"
#include "header/string.h"
#include <stddef.h>

static uintptr_t kernel_pd[1024] __attribute__((aligned(4096))); // Aligned to the 
static uintptr_t kernel_pt[1024] __attribute__((aligned(4096)));

void copy_kernel_map(uint32_t* new_kpd)
{
	int pdx = PDLOC(VIRTADDR(KSTART));
	new_kpd[pdx] = PHYSADDR(kernel_pd) | F_PRES | F_RW;
}

void* init_page_directory() {
	uintptr_t* pd = kalloc();	
	copy_kernel_map(pd);
	pd[1023] = (uintptr_t) pd;
	return (void*) pd;
}

void pg_init(uintptr_t *entry_pd)
{
	memcpy(kernel_pd, entry_pd, PGSIZE);
	
	kinit();
//	uint32_t page_dir = (uint32_t) init_page_directory();
}
