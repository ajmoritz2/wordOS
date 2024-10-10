#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

#define VM_FLAG_NONE 0
#define VM_FLAG_WRITE (1 << 1)
#define VM_FLAG_USER (1 << 2)
#define VM_OBJ_FLAG_PRESENT (1 << 3)

typedef struct vm_obj {
	uintptr_t base;
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

vmm* create_vmm(uint32_t*);

void set_current_vmm(vmm*);

void* vmm_alloc(size_t length, size_t flags, void* args); // Allocate a vm obj and push it to the current vmm.

void free(uintptr_t addr);

#endif
