#include "../kernel/kernel.h"
#include "../kernel/idt.h"
#include "pmm.h"
#include "string.h"
#include "paging.h"
#include <stddef.h>

#define RECURSIVE_ADDR 0xFFC00000

uint32_t kernel_pd[1024] __attribute__((aligned(4096))); // Aligned to the 4 Ki for CPU reasons
uint32_t kernel_pt[1024] __attribute__((aligned(4096)));

// mmap function!!
// Only use once pmm has been set up!
void memory_map(uint32_t* root_pd, uint32_t* phys, 
		uint32_t* virt, size_t flags)
{
	/*
	 * Page tables are located at (RECURSIVE_ADDR | (pd_index << 12))
	 * Check presence at the PD entry
	 *  TODO: Check to make sure VM spot isnt already taken, and if not, use the vmm
	 *  TODO: Create a second function to map to specific physical region rather than next
	 */
	uint32_t pd_index = (uint32_t) ((uint32_t)virt >> 22);
	uint32_t pt_index = (uint32_t) (((uint32_t)virt >> 12) & 0x3FF);

	if (!(root_pd[pd_index] & 1)) {
	       	create_new_pt(root_pd, pd_index);
	}
		
	uint32_t* page_table = (uint32_t*) (RECURSIVE_ADDR + (pd_index * 4096)); // Should be correct no matter what
	if ((uint32_t)phys & 0x3FF) { log_to_serial("PHYSICAL ADDRESS NOT PAGE ALIGNED\n"); return;}

	page_table[pt_index] = (uint32_t)phys | flags;	
}

void memory_unmap(uint32_t* root_pd, uint32_t* virt) {
	// TODO: Interface with the vmm here to keep track!
	uint32_t pd_index = (uint32_t) ((uint32_t)virt >> 22);
	uint32_t pt_index = (uint32_t) (((uint32_t)virt >> 12) & 0x3FF);

	uint32_t* page_table = (uint32_t*) (RECURSIVE_ADDR | (pd_index << 12));
	if (root_pd[pd_index] == 0) {
		return;
	}
	if ((uint32_t)virt & 0x3FF) { log_to_serial("PHYSICAL ADDRESS NOT PAGE ALIGNED\n"); return;}
	uint32_t phys = page_table[pt_index] & ~0xFFF;

	clear_frame(physical_to_frame((uint32_t*)phys));
	
	page_table[pt_index] = 0;	
}

uint32_t* create_new_pt(uint32_t* root_pd, uint32_t pd_index)
{
	// Recursive paging :3c
	
	log_to_serial("CREATING NEW PT\n");
	
	uint32_t* page_table = (uint32_t*) root_pd;

	uint32_t* new_pt = (uint32_t *) alloc_phys_page();

	root_pd[pd_index] = (uint32_t) new_pt | 3;
	
	uint32_t* virt_pt = (uint32_t*) (RECURSIVE_ADDR | (pd_index << 12));
	
	memset(virt_pt, 0, 1024);

	
	logf("New PT created at entry %d\n", pd_index);
	return virt_pt;
}	

void copy_kernel_map(uint32_t* new_kpd)
{
	// Remember that 768 and onward is pretty much reserved for kernel space, stack, and probably heap
	memcpy(new_kpd, kernel_pd, 4096);
	
}

void copy_higher_half_kernel_pd(void *pd)
{
	//memcpy(pd + (768 * sizeof(uint32_t)), kernel_pd + (768 * sizeof(uint32_t)), 1024); // Copy the last mibibyte of the page dir
	memcpy(pd, kernel_pd, 4096); // Copy the last mibibyte of the page dir
}

void copy_kernel_pd_index(void *pd, int index)
{	
	uint32_t *pd_int = (uint32_t *) pd;	
	pd_int[index] = kernel_pd[index];
}

void newinit_pd(uint32_t* pd)
{
	int i;
	for (i = 0; i < 1024; i++) {
		pd[i] = 0x2;
	}
	pd[1023] = (uint32_t) (pd - 0xC0000000) | 3;
}

void init_kpt(uint32_t* pt, uint32_t tag_size)
{
	// Just gonna keep it static here because its less of a headache for now.
	// We will see what I think of it later on...
	// Its really just a stupid hack
	uint32_t j = 0;
	uint32_t flag = 0x3;
	uint32_t init_pages = ((KEND - 0x100000) + (PGROUNDUP(NUM_FRAMES)) + tag_size) / 4096; // Some math, not very efficient,
	logf("PAGES: %d\n", init_pages);							      // but makes my life easy...
	for (uint32_t i = 256; i < 1024; i++)
	{
		if (i > 260+init_pages) flag = 0x2;
		//logf("Kernel page is: %d at %d\n", pt[i] & 3, i);
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
	//log_to_serial("WELCOME TO PAGINGFAULT\n");
	uint32_t code = frame->isr_err;
	if (code & 1) { log_to_serial("404: Page not found\n"); }
	if (code & 2) { logf("Write access at %x\n", (uint32_t) frame->cr2); }
	if (code & 4) { log_to_serial("CPL = 3 error, could be privilege\n"); }
	if (code & 8) { log_to_serial("Reserved Write\n"); }	
	if (code & 0x10) { log_to_serial("Instruction Fetch\n"); }
	if (code & 0x20) { log_to_serial("Protection Fault\n"); }
	if (code & 0x40) { log_to_serial("Shadow Stack\n"); }
	if (!(code & ~2)) {
		// TODO: Think of something else to do here...
		if (frame->cr2 == 0) {
			logf("Null pointer access?\n");
		}
		logf("Page fault at %x\n", frame->cr2);
		panic("MEMORY PAGE FAULT\n");
		//memory_map(kernel_pd, alloc_phys_page(), (uint32_t*)frame->cr2, 0x3);
		return 0;
	}

	return 1;
}

uint32_t* pg_init(uintptr_t *entry_pd, uint32_t tag_size)
{
	// Copy over tables into C code for easier access
	
	newinit_pd(kernel_pd);
	memset(kernel_pd, 0, 1024);
	memcpy(kernel_pt, (uint32_t*)((entry_pd[768] & 0xFFFFF000) + 0xC0000000), 4096);
	init_kpt(kernel_pt, tag_size); // PRAY TO GOD
	kernel_pd[768] = (uint32_t)PHYSADDR((uint32_t) kernel_pt) | 3;
	kernel_pd[1023] = (uint32_t)PHYSADDR((uint32_t) kernel_pd) | 3;	
	// ENDING TABLE COPIES
	// To put a page table in the directory, its pd[virt >> 22] into phys addr of pt 
	load_page_directory((uint32_t*)PHYSADDR((uint32_t)kernel_pd));
	enable_paging_c();

	return kernel_pd;
}
