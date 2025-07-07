/* VMM.C HEADER FILE vmm.h
 * CREATED 09/19/2024
 *
 * FOR WORDOS
 */

#include <stdint.h>
#include <stddef.h>
#include "../kernel/kernel.h"
#include "../memory/string.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"

vmm* current_vmm = NULL;

uint32_t convert_x86_32_vm_flags(size_t flags)
{
	uint32_t value = 1;
	
	if (flags & VM_FLAG_WRITE)
		value |= (1 << 1);	
	if (flags & VM_FLAG_USER)
		value |= (1 << 2);
	return value;
}

vmm* create_vmm(uint32_t* root_pd, uint32_t low, uint32_t high)
{

	memory_map(root_pd, alloc_phys_page(), (uint32_t*)TEMP_PAGE_VIRT, 0x3); 
	
	memset((void*)TEMP_PAGE_VIRT, 0, 4096);
	vmm *new_vmm = (vmm*)TEMP_PAGE_VIRT;
	new_vmm->root_pd = (uint32_t*)root_pd;
	new_vmm->head_vm_obj = NULL;
	new_vmm->vm_obj_store_addr = TEMP_PAGE_VIRT + sizeof(vmm);
	logf("Low addr marked at %x. High marked at %x\n\n", BYTE_PGROUNDUP(low), high);
	new_vmm->low = BYTE_PGROUNDUP(low);
	new_vmm->high = high;

	return new_vmm;
}

void vmm_transfer_dynamic(uint32_t* root_pd)
{
	virtual_t *new_vmm_sto = page_kalloc(4096, 0x3, 0);

	memcpy(new_vmm_sto, current_vmm, 4096);
	logf("Old VMM storage: %x, New VMM storage: %x\n", current_vmm, new_vmm_sto);


	virtual_t *old_vmm = (virtual_t *)current_vmm;
	current_vmm = (vmm *)new_vmm_sto;

	current_vmm->vm_obj_store_addr = $4r current_vmm + (current_vmm->vm_obj_store_addr & 0x3FF);

	memory_unmap(root_pd, old_vmm);
}

void set_current_vmm(vmm* new_vmm)
{
	current_vmm = new_vmm;

}

void* alloc_vm_obj()
{
	// Storage page: 0xFF7E8000 - 0xFF7E9000. Should be noted this is kernel only.
	// When userland is created, must add dynamic allocation for the vmms.
	// Should be fine with Kernel VMM being hardcoded into memory though for jumpstarting.
	// Can expand this later, but each vmm can hold 2048 allocs	so probably wont need to.

	void* start_page = (void*) current_vmm;
	uint32_t end_page = (uint32_t) (start_page + 4096);
	start_page += sizeof(vmm); // Puts us at the first vm_obj
	
	uint32_t potential = current_vmm->vm_obj_store_addr;

	if (potential < end_page) { // Alloc as far as you can before you start searching.
		current_vmm->vm_obj_store_addr += sizeof(vm_obj);
		logf("Allocated vm space at %x\n", potential);
		return (void*)potential;
	}
	uint32_t found = 0;
	while (start_page < end_page) {
		vm_obj* candidate = (vm_obj*) start_page;
		
		if (!(candidate->base & 1)) {
			found = (uint32_t) candidate;
			current_vmm->vm_obj_store_addr = found + sizeof(vm_obj);
			break;
		}

		start_page += sizeof(vm_obj);
	}
	
	if (!found)
		panic("Out of VMM storage space!");
	return (void*)found;
}

void* page_kalloc(size_t length, size_t flags, uint32_t phys)
{
	uint32_t vm_obj_flag_bits = ~1;
	vm_obj* current = current_vmm->head_vm_obj;
	vm_obj* prev = NULL;

	length = BYTE_PGROUNDUP((uint32_t)length); 
	uint32_t found = 0;

	if (length > (current_vmm->high - current_vmm->low)){
		logf("No thanks! VMM_HIGH: %x, VMM_LOW: %x\n", current_vmm->high, current_vmm->low);
		return 0;
	}
	if (current == NULL) {
		found = current_vmm->low;
		vm_obj* latest = (vm_obj*) alloc_vm_obj();

		latest->base = found | 1;
		latest->length = length;
		latest->flags = flags;
		latest->next = NULL;
		current_vmm->head_vm_obj = latest;

		goto map_pages;
	}

	while (current != NULL) {
		if (current == NULL)
			break;
		
		uint32_t base_comp = (prev == NULL ? current_vmm->high : prev->base & vm_obj_flag_bits);
		uint32_t candidate = (current->base & vm_obj_flag_bits) + current->length;
		if (candidate + length <= base_comp){
			found = candidate;
			break;
		}

		prev = current;
		current = current->next;
	}
	
	if (current == NULL) {
		if (current_vmm->low + length <= (prev->base & vm_obj_flag_bits)) {
			found = current_vmm->low;
		}
	}

	if (!found) {
		logf("No space!\n");
		return NULL;
	}

	vm_obj* new_vmm_obj = (vm_obj*) alloc_vm_obj();

	new_vmm_obj->base = found | 1;
	new_vmm_obj->length = length;
	new_vmm_obj->flags = flags;
	new_vmm_obj->next = current;
	if (prev) 
		prev->next = new_vmm_obj;
	else
		current_vmm->head_vm_obj = new_vmm_obj;

map_pages:
	if (!phys) {
		for (int i = 0; i < length; i += 4096) {
			memory_map(current_vmm->root_pd, alloc_phys_page(), (uint32_t*) (found + i), flags);
		}
	} else {
		for (int i = 0; i < length; i += 4096) { // Caller responsibility to keep track of phys frames here
			memory_map(current_vmm->root_pd, (uint32_t*) ((phys & ~0xFFF) + i), (uint32_t*) (found + i), flags);
		}
	}
	logf("Mapped phys is %x\n", phys);
	return (void*) (found + (phys & 0xFFF));
}

void page_free(void* addr) {
	vm_obj* current = current_vmm->head_vm_obj;
	vm_obj* prev = NULL;
	logf("--------------FREED---------------\n\n\n");
	while (current != NULL) {
		if (current == NULL) 
			break;
		if ((void*)(current->base & ~1) == addr)
			break;
			
		prev = current;
		current = current->next;
	}

	if (current == NULL) {
		logf("No vm obj found at base %x\n", addr);
		return;
	}
	if (prev)
		prev->next = current->next;
	else 
		current_vmm->head_vm_obj = current->next;
	
	for (int i = 0; i < current->length; i+=4096) {
		memory_unmap(current_vmm->root_pd, (uint32_t*)(addr + i));
	}
	memset(current, 0, sizeof(vm_obj));
}
