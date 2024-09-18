#include "header/paging.h"
#include "kernel.h"
#include "header/pfa.h"
#include "header/string.h"
#include "interupts.h"
#include <stddef.h>

static uint32_t kernel_pd[1024] __attribute__((aligned(4096))); // Aligned to the 4 Ki for CPU reasons
static uint32_t kernel_pt[1024] __attribute__((aligned(4096)));
static uint32_t test_pt[1024] __attribute__((aligned(4096)));

void copy_kernel_map(uint32_t* new_kpd)
{
	// Remember that 768 and onward is pretty much reserved for kernel space, stack, and probably heap
	memcpy(new_kpd, kernel_pd, 4096);
	
}


void newinit_pd(uint32_t* pd)
{
	int i;
	for (i = 0; i < 1024; i++) {
		pd[i] = 0x3;
	}
	pd[1023] = (uint32_t) (pd - 0xC0000000) | 3;
}

void init_kpt(uint32_t* pt)
{
	// Just gonna keep it static here because its less of a headache for now.
	// We will see what I think of it later on...
	// Its really just a stupid hack
	uint32_t j = 0;
	uint32_t flag = 0x3;
	uint32_t init_pages = ((KEND - 0x100000) + (PGROUNDUP(NUM_FRAMES*4))) / 4096; // Some math, not very efficient,
										      // but makes my life easy...
	for (uint32_t i = 256; i < 1024; i++)
	{
		if (i > 257+init_pages) flag = 0x2;
		if (!(pt[i] &  3)) {
			pt[i] = (j * 0x1000) + 0x100000 | flag; // Mark kernel pages as present because I'm lazy af
		}
		j++;
	}
}

void load_page_directory(uint32_t* pdphys)
{
	asm volatile("mov %0, %%cr3" : : "r"(pdphys));
}

void enable_paging_c(void)
{
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

uint8_t handle_exception(struct isr_frame *frame)
{
	log_to_serial("WELCOME TO PAGINGFAULT\n");
	uint32_t code = frame->isr_err;
	if (code & 1) { log_to_serial("404: Page not found\n"); }
	if (code & 2) { log_to_serial("Write access\n"); }
	if (code & 4) { log_to_serial("CPL = 3 error, could be privilege\n"); }
	if (code & 8) { log_to_serial("Reserved Write\n"); }	
	if (code & 0x10) { log_to_serial("Instruction Fetch\n"); }
	if (code & 0x20) { log_to_serial("Protection Fault\n"); }
	if (code & 0x40) { log_to_serial("Shadow Stack\n"); }
	log_to_serial("CR2 Dump: ");
	print_hex((uint64_t)frame->cr2);
	log_to_serial("\n");
	return 0;
}

void pg_init(uintptr_t *entry_pd)
{
	// Copy over tables into C code for easier access
	newinit_pd(kernel_pd);
	memcpy(kernel_pd, entry_pd, 4096);
	memcpy(kernel_pt, (uint32_t*)((kernel_pd[768] & 0xFFFFF000) + 0xC0000000), 4096);
	init_kpt(kernel_pt); // PRAY TO GOD
	
	kernel_pd[768] = (uint32_t)PHYSADDR((uint32_t) kernel_pt) | 3;
	
	// ENDING TABLE COPIES
	// To put a page table in the directory, its pd[virt >> 22] into phys addr of pt 
	load_page_directory((uint32_t*)PHYSADDR((uint32_t)kernel_pd));
	enable_paging_c();
}
