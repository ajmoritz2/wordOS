#include "kernel.h"
#include "header/string.h"
#include <stddef.h>
#include "header/paging.h"
#include "header/pfa.h"

// START PFA

struct run {
	struct run *next;
} freelist;


void freerange(uint32_t pa_start, void *pa_end)
{
	log_integer_to_serial((uint32_t)pa_start);
}

void kfree(void *pa)
{

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
	freerange((uint32_t)&_kernel_end, (uint32_t*)(PHYSTOP));
}
// END PFA
