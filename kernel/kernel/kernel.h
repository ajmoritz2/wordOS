#ifndef KERNEL_H
#define KERNEL_H

#define KERNEL_VERSION  "0.0.5"

#include <stdint.h>

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

#define $1			(uint8_t *)
#define $2			(uint16_t *)
#define $4			(uint32_t *)
#define $8			(uint64_t *)

#define $1r			(uint8_t)
#define $2r			(uint16_t)
#define $4r			(uint32_t)
#define $8r			(uint64_t)

#define PORT 		0x3f8
#define KSTART		((uint32_t)&_kernel_start)
#define KEND		((uint32_t)&_kernel_end - 0xC0000000)

#define KBASE &_kernel_end
#define PHYSTOP ((uint32_t)(KBASE + 128*1024*1024))

extern uint8_t task_state;

extern void load_directory(uint32_t *page_dir);

void panic(char* reason);
unsigned char inportb(int portnum);
void outportb(int portnum, unsigned char data);
void log_to_serial (char* number);
void log_integer_to_serial (uint64_t number);
void print_hex (uint64_t number);
void logf(char*, ...);
uint32_t get_stackp();

void triple_fault();

static inline void io_wait(void)
{
    outportb(0x80, 0);
}

#endif
