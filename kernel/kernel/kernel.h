#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

extern void enable_paging(uint32_t daddr);

#define PORT 		0x3f8
#define KSTART		((uintptr_t)&_kernel_start)
#define KEND		((uintptr_t)&_kernel_end - 0xC0000000)

#define KBASE &_kernel_end
#define PHYSTOP ((uint32_t)(KBASE + 128*1024*1024))

void log_to_serial (char* number);
void log_integer_to_serial (uint64_t number);
#endif
