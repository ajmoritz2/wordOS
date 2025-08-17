/* VMM.C HEADER FILE vmm.h
 * CREATED 09/19/2024
 *
 * FOR WORDOS
 */

#include <stdint.h>
#include <stddef.h>
#include "../kernel/kernel.h"
#include "../kernel/scheduler.h"
#include "../memory/string.h"
#include "../programs/terminal.h"
#include "../utils/rbtree.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"

vmm* current_vmm = NULL;
vmm *kernel_vmm;

// TODO: Rewrite this entire file because this sucks...

uint32_t convert_x86_32_vm_flags(size_t flags)
{
	uint32_t value = 1;
	
	if (flags & VM_FLAG_WRITE)
		value |= (1 << 1);	
	if (flags & VM_FLAG_USER)
		value |= (1 << 2);
	return value;
}

vmm* create_vmm(uint32_t* root_pd, uint32_t low, uint32_t high, void *store_page)
{

	if (!store_page) {
		memory_map(root_pd, alloc_phys_page(), (uint32_t*)TEMP_PAGE_VIRT, 0x3); 
		store_page = (void*) TEMP_PAGE_VIRT;
	}

	// Interesting page error happening in here...
	logf("Store page located at :%x\n", store_page);
	
	memset((void*)store_page, 0, 4096);
	vmm *new_vmm = (vmm*)store_page;
	new_vmm->root_pd = (uint32_t*)root_pd;
	
	new_vmm->root = NULL;
	new_vmm->vm_obj_store_addr = (uintptr_t) (store_page + sizeof(vmm));
	logf("Low addr marked at %x. High marked at %x\n\n", BYTE_PGROUNDUP(low), high);
	new_vmm->low = BYTE_PGROUNDUP(low);
	new_vmm->high = high;

	make_node(new_vmm, new_vmm->low, new_vmm->high, 0); // Create first free entry
	logf("VMM Free tree created. Size %x\n", new_vmm->root->size);

	return new_vmm;
}

void *page_kalloc(size_t length, size_t flags, uint32_t phys)
{
	void *page_to_ret = vmm_page_alloc(kernel_vmm, length, flags, phys);
/*	process_t *current = get_process_head();
	logf("Current process_head: %x\n", current);

	while (current) {

		uint32_t *table = (uint32_t *) TEMP_PAGE_VIRT;

		memory_map(kernel_vmm->root_pd, (uint32_t *) current->context->cr3, table, 0x3); 

		uint32_t pd_index = (uint32_t) page_to_ret >> 22;

		uint32_t *pt_phys = (uint32_t *) table[pd_index]; 
		logf("PT PHYS on pagekfree: %x\n", pt_phys);

		memory_unmap(kernel_vmm->root_pd, table); 
		current = current->next;
	}*/

	return page_to_ret;
}

void page_kfree(void *addr)
{
	vmm_page_free(kernel_vmm, addr);
	
/*	process_t *current = get_process_head();

	while (current) {

		uint32_t *table = (uint32_t *) TEMP_PAGE_VIRT;

		memory_map(kernel_vmm->root_pd, (uint32_t *) current->context->cr3, table, 0x3); 

		uint32_t pd_index = (uint32_t) addr >> 22;

		uint32_t *pt_phys = (uint32_t *) table[pd_index]; 
		logf("PT PHYS on pagekfree: %x\n", pt_phys);

		memory_unmap(kernel_vmm->root_pd, table); 
		current = current->next;
	}*/
}

void *page_alloc(size_t length, size_t flags, uint32_t phys)
{
	return vmm_page_alloc(current_vmm, length, flags, phys);
}

void page_free(void *addr)
{
	vmm_page_free(current_vmm, addr);
}

void vmm_transfer_dynamic(vmm **to_trans, uint32_t* root_pd)
{
	virtual_t *new_vmm_sto = page_kalloc(4096, 0x3, 0);

	memcpy(new_vmm_sto, *to_trans, 4096);
	logf("Old VMM storage: %x, New VMM storage: %x\n", *to_trans, new_vmm_sto);

	uint32_t root_offset = (uint32_t) (*to_trans)->root - (uint32_t) *to_trans;

	virtual_t *old_vmm = (virtual_t *)to_trans;
	*to_trans = (vmm *)new_vmm_sto;

	(*to_trans)->vm_obj_store_addr = $4r *to_trans + ((*to_trans)->vm_obj_store_addr & 0x3FF);
	(*to_trans)->root = (rbnode_t *) ((uint32_t) *to_trans + root_offset);


	memory_unmap(root_pd, old_vmm);
}

void set_kernel_vmm(vmm *kvmm)
{
	kernel_vmm = kvmm;
}

void set_current_vmm(vmm* new_vmm)
{
	current_vmm = new_vmm;

}

void* vmm_page_alloc(vmm *cvmm, size_t length, size_t flags, uint32_t phys)
{
	rbnode_t *current = cvmm->root;
	length = BYTE_PGROUNDUP((uint32_t) length);

	logf("Allocating page in vmm stored at location: %x\n", cvmm);
	
	if (!current) {
		logf("Page not allocated. No free list\n");
		return NULL;
	}

	// We will find the BEST not first.
	
	rbnode_t *best = 0;
	
	while (current) {
		if (current->size == length) {
			best = current;
			break; // Found size
		}

		if (current->size <= length) {
			current = current->right;
		} else {
			best = current;
			current = current->left;
		}
	}

	if (!best) {
		logf("No free memory found.\n");
		return NULL;
	}

	if (best->size > length) {
		// Resizing operation
		make_node(cvmm, best->base + length, best->size - length, 0); // 0 is id. Debug feature
	}

	uint32_t base = best->base;
	
	delete_node(cvmm, &(kernel_vmm->root), best);

	if (!phys) {
		for (int i = 0; i < length; i += 4096) {
			memory_map(cvmm->root_pd, alloc_phys_page(), (uint32_t*) (base + i), flags);
		}
	} else {
		for (int i = 0; i < length; i += 4096) { // Caller responsibility to keep track of phys frames here
			memory_map(cvmm->root_pd, (uint32_t*) ((phys & ~0xFFF) + i), (uint32_t*) (base + i), flags);
		}
	}
	logf("Mapped phys is %x\n", phys);
	return (void*) (base + (phys & 0xFFF));
}

void vmm_page_free(vmm *cvmm, void* addr) {

	uint32_t *pd = cvmm->root_pd;
	
	uint32_t *page_table = (uint32_t *) (pd[((uint32_t) addr >> 22)] & ~0x3FF) ;


	if (!page_table)
		return;

	uint32_t page = page_table[((uint32_t) addr >> 12) & ~0x3FF];
	if (!(page & 1)) 
		return;
		
	make_node(cvmm, (uint32_t) addr, 4096, 0); 	
	memory_unmap(cvmm->root_pd, (uint32_t*)(addr));
}


void init_user_memory()
{
	void *store = page_kalloc(4096, 0x3, 0); // 1 page present writeable and phys doesnt matter.
	
	//user_vmm = create_vmm(current_vmm->root_pd, 0x100000, 0xC0000000, store);
}

void *alloc_stack(vmm *process_vmm)
{
	// We will allocate 1 page per process for the stack
	current_vmm = process_vmm;

	void *stack_top = page_alloc(0x1000, 0x3, 0);	// USER PAGE EXPERIMENT!

	return stack_top + 0xFFF;
}
