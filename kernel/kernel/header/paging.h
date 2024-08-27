#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PGSIZE		1024

#define ERROR		0xF
#define FREE 		0
#define USED		0x01

#define F_PRES		0x001
#define F_RW		0x002

#define PTLOC(x)	(((uintptr_t)((x) >> 12) & 0x3FF))
#define PDLOC(x)	(((uintptr_t)(x) >> 22))
#define VIRTADDR(x)	(((uint32_t)(x)) + 0xC0000000)
#define PHYSADDR(x)	(((uint32_t)(x)) - 0xC0000000)

#define PGROUNDUP(s) 	(((s)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) 	(((a)) & ~(PGSIZE-1))

void pg_init(uintptr_t *entry_pd);

#endif
