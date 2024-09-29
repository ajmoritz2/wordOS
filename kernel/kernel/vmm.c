/* VMM.C HEADER FILE header/vmm.h
 * CREATED 09/19/2024
 *
 * FOR WORDOS
 */

#include <stdint.h>
#include <stddef.h>
#include "kernel.h"
#include "header/paging.h"
#include "header/pfa.h"
#include "header/vmm.h"

vmm* current_vmm = NULL;

uint32_t convert_x86_32_vm_flags(size_t flags)
{
	uint32_t value = 0;
	
	if (flags & VM_FLAG_WRITE)
		value |= (1 << 1);	
	if (flags & VM_FLAG_USER)
		value |= (1 << 2);
	return value;
}

vmm* create_vmm(uint32_t* root_pd)
{

	memory_map(root_pd, alloc_phys_page(), (uint32_t*)0xFF7E8000, 0x3); 

	vmm *new_vmm = (vmm*)0xFF7E8000;
	new_vmm->root_pd = (uint32_t*)root_pd;
	new_vmm->head_vm_obj = NULL;
	new_vmm->vm_obj_store_addr = 0xFF7E8000 + sizeof(vmm);
	new_vmm->low = 0;
	new_vmm->high = 1024;

	return new_vmm;
}

void set_current_vmm(vmm* new_vmm)
{
	current_vmm = new_vmm;

}

void* vmm_alloc(size_t length, size_t flags, void* args)
{
	vm_obj* prev = current_vmm->head_vm_obj;
	length = PGROUNDUP(length)*4;
	uint32_t found = 0;
	uint32_t mem_top = 0xB0000000;
	
	vm_obj* latest;
	if (prev == NULL) {
		latest = (vm_obj*) current_vmm->vm_obj_store_addr;
		current_vmm->head_vm_obj = latest;
		latest->base = found;
		latest->length = length;
		latest->flags = flags;

		goto map_pages;
	}

	if (prev->base > 0) {
		if (prev->base >= length) {
			latest = (vm_obj*) current_vmm->vm_obj_store_addr;
			current_vmm->head_vm_obj = latest;
			latest->base = 0;
			latest->length = length;
			latest->flags = flags;

			goto map_pages;
		}
	}

	vm_obj* current = (prev == NULL ? NULL : prev->next);
	
	while (current != NULL) {
		uint32_t base = (uint32_t) (prev->base + prev->length);

		if (current->base - base >= length) {
			found = base;
			break;
		}

		prev = current;
		current = current->next;
	}
	
	latest = (vm_obj*) current_vmm->vm_obj_store_addr;
	if (current == NULL) {
		if (mem_top - (prev->base + prev->length) < length) { 
			logf("No virtual room for allocation of size %x!\n", length);
			return (void*)0xFFFFFFFF;
		}

		found = prev->base + prev->length;
		prev->next = latest;
	} else {
		current->next = latest;
	}

	latest->base = found;
	latest->length = length;
	latest->flags = flags;
	
	
map_pages:
	current_vmm->vm_obj_store_addr += sizeof(vm_obj);
	
	for (int i = found; i < found + length; i+=4096) {
		memory_map(current_vmm->root_pd, alloc_phys_page(), (uint32_t*)i, 0x3); // TODO: Change to flags
	}

	return (void*)found;
}

void free(uintptr_t addr) {
	vm_obj* current = current_vmm->head_vm_obj;
	vm_obj* prev = NULL;
	logf("--------------FREED---------------\n\n\n");
	while (current->base != addr) {
		if (current == NULL) 
			break;
			
		prev = current;
		current = current->next;
	}

	if (current == NULL) {
		logf("No vm obj found at base %x\n", addr);
		return;
	}

	if (prev == NULL) {
		current_vmm->head_vm_obj = current->next;
		memory_unmap(current_vmm->root_pd, (uint32_t*)addr);
		return;
	}

	prev->next = current->next;
	memory_unmap(current_vmm->root_pd, (uint32_t*)addr);
}
