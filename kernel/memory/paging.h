#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include "../kernel/idt.h"

#define PGSIZE		1024

#define ERROR		0xF
#define FREE 		0
#define USED		0x01

#define F_PRES		0x001
#define F_RW		0x002

#define GET_PTE(x)	(((uintptr_t)((x) >> 12) & 0x3FF))
#define GET_PDE(x)	(((uintptr_t)(x) >> 22))
#define VIRTADDR(x)	(((uint32_t)(x)) + 0xC0000000)
#define PHYSADDR(x)	(((uint32_t)(x)) - 0xC0000000)

#define PGROUNDUP(s) 	(((s)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) 	(((a)) & ~(PGSIZE-1))
#define BYTE_PGROUNDDOWN(a) (((uint32_t)((a) & ~(4096-1))))
#define BYTE_PGROUNDUP(a) (((uint32_t)((a + (4096-1)) & ~(4096-1))))

#define TEMP_PAGE_VIRT 0xFF000000
#define TEMP_PAGE_PHYS 0x400000

typedef uintptr_t physical_t;
typedef uintptr_t virtual_t;

typedef struct {
	uint32_t present : 1;
	uint32_t rw : 1;
	uint32_t user : 1;
	uint32_t reserved1 : 2; // The PWT and PCD, I'm not touching those
	uint32_t accessed : 1;
	uint32_t dirty : 1;
	uint32_t PAT : 1; // Should be 0
	uint32_t global : 1;
	uint32_t avl : 3;
	physical_t addr : 20;

} page_t;

// I don't see a need to create a pt and pd structure because
// they are just arrays.

uint8_t handle_exception(struct isr_frame *frame);

uint32_t* pg_init(uintptr_t *entry_pd, uint32_t tag_size);

void memory_map(uint32_t* root_pd, uint32_t* phys, uint32_t* virt, size_t flags);
void memory_unmap(uint32_t*, uint32_t*);

uint32_t* create_new_pt(uint32_t*, uint32_t);

#endif
