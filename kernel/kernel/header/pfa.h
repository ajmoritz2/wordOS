#ifndef PFADEFINE
#define PFADEFINE
#include <stddef.h>

void kinit();

void kfree(void *pa);
void* kalloc(void);

#endif
