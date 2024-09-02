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
		pd[i] = 0x3;
	}
	pd[1023] = (uint32_t) pd;
}

void init_pt(uint32_t* pt)
{
	uint32_t i;
	for (i = 0; i < 1024; i++)
	{
		pt[i] = (i * 0x1000) + 0x100000 | 3;
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
	log_integer_to_serial((uint64_t)frame->isr_err);
	log_to_serial("\n");
	return 0;
}

void pg_init(uintptr_t *entry_pd)
{
//	kinit();
	newinit_pd(kernel_pd);
	memcpy(kernel_pd, entry_pd, 4096);
	init_pt(kernel_pt);
	//kernel_pd[767] = (uint32_t)PHYSADDR((uint32_t)kernel_pt) | 3;
	// To put a page table in the directory, its pd[virt >> 22] into phys addr of pt 
	load_page_directory((uint32_t*)PHYSADDR((uint32_t)kernel_pd));
	enable_paging_c();
}
