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
	memory_map(root_pd, alloc_phys_page(), (uint32_t*)0xFFBE8000, 0x3); 
	vmm *new_vmm = (vmm*)0xFFBE8000;

	new_vmm->root_pd = root_pd;
	new_vmm->head_vm_obj = NULL;
	new_vmm->vm_obj_store_addr = 0xFFBE8000;
	new_vmm->low = 0;
	new_vmm->high = 0;

	return new_vmm;
}

void set_current_vmm(vmm* new_vmm)
{
	current_vmm = new_vmm;
}

void* vmm_alloc(size_t length, size_t flags, void* args)
{
	if (current_vmm == NULL) { log_to_serial("NO VMM DEFINED!\n"); return 0; }
	length = PGROUNDUP(length);
	vm_obj* current = current_vmm->head_vm_obj;
	vm_obj* prev = NULL;
	uintptr_t found = 0;

	while (current != NULL) {
		if (current == NULL)
			break;
	}
}
