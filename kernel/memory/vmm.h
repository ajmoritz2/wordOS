#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

#define VM_FLAG_NONE 0
#define VM_FLAG_WRITE (1 << 1)
#define VM_FLAG_USER (1 << 2)
#define VM_OBJ_FLAG_PRESENT (1 << 3)

typedef struct vm_obj {
	uintptr_t base; // First bit is a present flag
	size_t length;
	size_t flags;
	struct vm_obj* next;
} vm_obj;

typedef struct {
	uint32_t* root_pd;
	vm_obj* head_vm_obj;
	uintptr_t vm_obj_store_addr;
	uint32_t low;	// UNUSED FOR NOW
	uint32_t high;	// Extra arguments just in case I want to seperate address spaces.
} vmm;

uint32_t convert_x86_32_vm_flags(size_t);

vmm* create_vmm(uint32_t*, uint32_t low, uint32_t high);

void set_current_vmm(vmm*);
void vmm_transfer_dynamic(uint32_t* root_pd);

void* page_kalloc(size_t length, size_t flags, uint32_t phys); // Allocate a vm obj and push it to the current vmm.
void page_free(void* addr);

#endif
