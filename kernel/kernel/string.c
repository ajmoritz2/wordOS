#include <stdint.h>
#include <stddef.h>
#include "header/string.h"

int memcmp(const void* p1, const void* p2, size_t n)
{
	size_t i;

	if (p1 == p2) 
		return 0;

	for (i = 0; (i < n) && (*(uint8_t*) p1 == *(uint8_t*) p2); i++, p1 = 1 + (uint8_t *) p1,
			p2 = 1 +  (uint8_t *)p2);

	return (i == n) ? 0 : (*(uint8_t *) p1 - *(uint8_t *)p2);
}

// I dont understand a lick of this, but its optimized apparently

void* memcpy (void *dest, const void *src, size_t len)
{
	char *d = dest;
	const char *s = src;
	while (len--)
		*d++ = *s++;
	return dest;
}

void* memmove(void* s1, const void* s2, size_t n)
{
	return memcpy(s1, s2, n);
}

void* memset(void * dest, int c, size_t n)
{
	uint32_t i;
	uint32_t *p = dest;
	i = 0;
	while (n > 0) {
		*p = c;
		p++;
		n--;
	}
	return (dest); 
}

