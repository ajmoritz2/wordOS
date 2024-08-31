#include "kernel.h"
#include "header/string.h"
#include <stddef.h>
#include "header/paging.h"
#include "header/pfa.h"

// START PFA

struct run {
	struct run *next;
} freelist;


void freerange(void *pa_start, void *pa_end)
{
	uint32_t *p;
	p = (uint32_t*)PGROUNDUP((uint32_t)pa_start);
	
	uint32_t *pt = (uint32_t*)PGROUNDUP((uint32_t)pa_start);
	log_integer_to_serial((uint32_t)PGROUNDUP((uint32_t) pa_start));
	for(; (uint32_t)(p + 1024) <= (uint32_t)pa_end; p += 1024)
		kfree(p);
}

void kfree(void *pa)
{
	struct run *r;
	// ***********************************************
	// TODO: Properly page the memory before using it.
	//
	// What I believe is happening is the page is not present
	// so it is creating a page fault, which then defaults
	// to a triple fault because I do not have a page fault
	// handler! 
	// ***********************************************
	//TODO: Add some checks here to be safer I guess
//	log_integer_to_serial((uint32_t) pa);	
//	3222321254
	//memset((uint32_t*) 3222321254, 1, 10);

	//log_integer_to_serial((uint32_t)&pap);
	//log_integer_to_serial(pap[0]);
	// Adding the stuff to the linked list
	r = (struct run*)pa;
	//log_to_serial("\nNEXT\n");
	r->next = freelist.next; 
	//freelist.next = r;
}


// ALLOCATE A PAGE
void* kalloc(void) 
{
	struct run *r;
	r = freelist.next;
	if (r)
		freelist.next = r->next;

	if (r)
		memset((char*)r, 5, PGSIZE);
	return (void*)r;
}

void kinit() 
{
	//log_integer_to_serial((uint64_t)PHYSTOP);
	//log_to_serial(" is the PHYSTOP\n");	
	freerange((uint32_t*)&_kernel_end, (uint32_t*)(PHYSTOP));
}
// END PFA
