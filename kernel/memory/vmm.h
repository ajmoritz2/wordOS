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
} vm_obj;

typedef struct vmm{
	uint32_t* root_pd;
	uint64_t *root_pdpt;
	uint64_t pd_low; // We can perhaps save a headache here by keepinf pd phys addrs in here.
	uint64_t pd_mid; // We will have to map to the kernel page for a quick sec while we map these
	uint64_t pd_high; // And then finally recursively map them. After, unmap from kernel.
	struct rbtree_node* root;
	uintptr_t vm_obj_store_addr;
	uint32_t low;	// UNUSED FOR NOW
	uint32_t high;	// Extra arguments just in case I want to seperate address spaces.
} vmm;

extern vmm *current_vmm;
extern vmm *kernel_vmm;

uint32_t convert_x86_32_vm_flags(size_t);

vmm* create_vmm(uint32_t *root_pd, uint32_t low, uint32_t high, void *store_page);
//void init_user_memory();

void set_kernel_vmm(vmm *kvmm);
void set_current_vmm(vmm*);
void vmm_transfer_dynamic(vmm **to_trans, uint32_t* root_pd);

void *alloc_stack();
void init_user_memory();


void* vmm_page_alloc(vmm *cvmm, size_t length, size_t flags, uint64_t phys, uint32_t pae_enable);
void vmm_page_free(vmm *cvmm, void* addr);
void pae_page_free(vmm *cvmm, void *addr);


void *pae_kalloc(size_t length, size_t flags, uint64_t phys);
void pae_kfree(void *addr);
void *pae_alloc(size_t length, size_t flags, uint64_t phys);
void pae_free(void *addr);

void *page_kalloc(size_t length, size_t flags, uint32_t phys);

void page_kfree(void *addr);

void *page_alloc(size_t length, size_t flags, uint32_t phys);

void page_free(void *addr);

#endif
