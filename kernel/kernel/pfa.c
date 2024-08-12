#include "kernel.h"
#include "header/string.h"
#include <stddef.h>
#include "header/paging.h"
#include "header/pfa.h"

// START PFA

struct run {
	struct run *next;
};

struct {
	struct run *freelist;
} kmemory;

void freerange(void *pa_start, void *pa_end)
{
	char *p;
	p = (char*)PGROUNDUP((uint64_t)pa_start);
	for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
		kfree(p);
}

void kfree(void *pa)
{
	struct run *r;

	//TODO: Add some checks here to be safer I guess
	
	memset(pa, 1, PGSIZE);

	// Adding the stuff to the linked list
	r = (struct run*)pa;

	r->next = kmemory.freelist; 
	kmemory.freelist = r;
}


// ALLOCATE A PAGE
void* kalloc(void) 
{
	struct run *r;
	r = kmemory.freelist;
	if (r)
		kmemory.freelist = r->next;

	if (r)
		memset((char*)r, 5, PGSIZE);
	return (void*)r;
}

void kinit() 
{
	freerange((uint32_t*)_kernel_end, (void*)PHYSTOP);
}
// END PFA
