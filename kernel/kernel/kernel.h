#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define PORT 		0x3f8
#define KSTART		((uintptr_t)&_kernel_start)
#define KEND		((uintptr_t)&_kernel_end - 0xC0000000)

void log_to_serial (char* number);
void log_integer_to_serial (unsigned int number);
#endif
