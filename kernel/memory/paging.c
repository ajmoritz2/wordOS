#include "../kernel/kernel.h"
#include "../kernel/idt.h"
#include "../drivers/apic.h"
#include "pmm.h"
#include "vmm.h"
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
	if ((uint32_t)phys & 0x3FF) { panic("PHYSICAL ADDRESS NOT PAGE ALIGNED\n"); return;}

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
	if ((uint32_t)virt & 0x3FF) { logf("PHYSICAL ADDRESS %x NOT PAGE ALIGNED\n", virt); return;}
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
	memcpy(pd + (768 * sizeof(uint32_t)), kernel_pd + 768, 1024); // Copy the last mibibyte of the page dir
//	memcpy(pd, kernel_pd, 4096); // Copy the last mibibyte of the page dir
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

void init_pae(vmm *current_vmm)
{
	/*
	 * When we look at it, the kernel pages are mapped at 0b11...
	 * This means we are ALWAYS accessing the 4th pdpte, so the addr
	 * do not need to change for that.
	 * Framebuffer is the noteable exception.
	 */

	struct cpuid_status cpu01h = get_cpuid(0x01);

	if (cpu01h.eax & 1 << 6)
		logf("PAE is available!\n");
	else
		panic("No PAE supported...\n");

	struct cpuid_status cpu808h = get_cpuid(0x80000008);

	uint8_t max_phys_addr = cpu808h.eax & 0xff;

	logf("Max phys bit extensions: %d\n", max_phys_addr);

	uint32_t pdpt_phys = (uint32_t) alloc_phys_page();
	uint32_t *pdpt_page = page_kalloc(PGSIZE, 0x3, (uint32_t) pdpt_phys);

	uint64_t *new_pd_phys = (uint64_t *) alloc_phys_page();
	uint64_t *new_pd = page_kalloc(PGSIZE, 0x3, (uint32_t) new_pd_phys);

	uint64_t *identity_pd_phys = (uint64_t *) alloc_phys_page();
	uint64_t *identity_pd = page_kalloc(PGSIZE, 0x3, (uint32_t) identity_pd_phys);

	uint64_t *identity_pt_phys = (uint64_t *) alloc_phys_page();
	uint64_t *identity_pt = page_kalloc(PGSIZE, 0x3, (uint32_t) identity_pt_phys);
	pdpt_page[0] = (uint32_t) identity_pd_phys | 1;

	// Here we gotta copy over our kernel pages, but in a weird way because we now have 512 maps
	

	/* We need to identity map SPECIFICALLY this code page, because we will do something awesome
	 * and disable paging (in the middle of the fucking execution...
	 *
	 * Lucky for us we know everything (codewise) is currently 0xC.. ahead of its true addr
	 */

	uint32_t eip = 0;
	asm volatile ("		cli \n\
						jmp eip_get2 \n\
				eip_get1:		\n\
						jmp eip_get_fin	\n\
				eip_get2:		\n\
						call eip_get1	\n\
				eip_get_fin:	\n\
						pop %%eax		\n\
						mov %%eax, %0" : "=r" (eip));

	eip -= 0xC0000000;
	eip &= ~0x3ff;

	memory_map(current_vmm->root_pd, (uint32_t *) eip, (uint32_t *) (eip), 0x3);

	identity_pt[(eip >> 12) & 0x1ff] = (uint64_t) eip | 3;
	identity_pd[(eip >> 21) & 0x1ff] = (uint64_t) identity_pt_phys | 3;

	logf("Physical pdpt page is at %x\n", pdpt_phys);


	asm volatile ("mov $disable_paging, %%eax \n\
					mov %0, %%ecx	\n\
					sub $0xc0000000, %%eax \n\
					jmp %%eax \n\
			disable_paging: \n\
					mov $0x00000001, %%eax\n\
					mov %%eax, %%cr0\n\
					mov %%ecx, %%cr3\n\
					mov %%cr4, %%eax\n\
					orl $0x20, %%eax\n\
					mov %%eax, %%cr4\n\
					mov %%cr0, %%eax\n\
					or $0x80000000, %%eax\n\
					mov %%eax, %%cr0\n\
					1: jmp 1b" : : "m" (pdpt_phys));  // Disables paging. Should get a fault if i remove the loop...

	logf("EIP AT %x\n", eip);
	logf("PDPT LOCATED AT %x\n", pdpt_page);

}

uint32_t* pg_init(uintptr_t *entry_pd, uint32_t *tag_size)
{
	// Copy over tables into C code for easier access
	
	newinit_pd(kernel_pd);

	memory_map(entry_pd, (uint32_t *) PGROUNDDOWN((uint32_t) tag_size - 0xC0000000), (uint32_t *) PGROUNDDOWN((uint32_t) tag_size), 0x3);

	memset(kernel_pd, 0, 1024);
	memcpy(kernel_pt, (uint32_t*)(entry_pd + 0xC0000000), 1024 * sizeof(uint32_t));

	logf("tag size location at %x\n", tag_size); 

	init_kpt(kernel_pt, *tag_size); // PRAY TO GOD
	kernel_pd[768] = (uint32_t)PHYSADDR((uint32_t) kernel_pt) | 3;
	kernel_pd[1023] = (uint32_t)PHYSADDR((uint32_t) kernel_pd) | 3;	
	memory_map(kernel_pd, (uint32_t *) PGROUNDDOWN((uint32_t) tag_size - 0xC0000000), (uint32_t *) PGROUNDDOWN((uint32_t) tag_size), 0x3);
	// ENDING TABLE COPIES
	// To put a page table in the directory, its pd[virt >> 22] into phys addr of pt 
	load_page_directory((uint32_t*)PHYSADDR((uint32_t)kernel_pd));
	enable_paging_c();

	return kernel_pd;
}
