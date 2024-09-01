#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

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
	uint32_t addr : 20;

} page_t;

// I don't see a need to create a pt and pd structure because
// they are just arrays.

void pg_init(uintptr_t *entry_pd);

#endif
