#include <stdint.h>
#include <stddef.h>
#include "string.h"

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
	// Len is in BYTES HERE
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
	// Size is in bytes
	uint8_t *p = dest;
	while (n > 0) {
		*p = c;
		p++;
		n--;
	}
	return (dest); 
}

uint32_t strlen(char *string)
{
	char *ptr = &string[0];

	while (*ptr)
		ptr++;

	return ptr-string;
}

// Returns 1 if they are the same, and 0 is they are different
uint8_t strcmp(char *s1, char *s2, size_t size)
{
	for (int i = 0; i < size; i++) {
		if (s1[i] != s2[i])
			return 0;
	}

	return 1;
}

// Copy s2 into s1
void strncpy(char *dst, char *src, size_t n)
{
	for (int i = 0; i < n; i++) {
		src[n] = dst[n];
		if (!src[n])
			break;
	}
}
