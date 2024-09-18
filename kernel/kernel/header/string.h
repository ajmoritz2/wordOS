
#ifndef STRINGH
#define STRINGH 

#include <stdint.h>
#include <stddef.h>

int memcmp(const void* p1, const void* p2, size_t n);

void * memcpy(void *dst0, const void *src0, size_t length);

void* memmove(void* s1, const void* s2, size_t n);

void* __attribute__((weak)) memset(void* dest, int c, size_t n);

uint32_t strlen(char *string);

#endif
