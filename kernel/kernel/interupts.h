#include <stdint.h>
#ifndef INTERUPT_H
#define INTERUPT_H

typedef struct 
{
	uint16_t address_low;
	uint16_t selector;
	uint8_t reserved;
	uint8_t flags;
	uint16_t address_high;
} __attribute__((packed)) idt_entry;

typedef struct
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idtr_t;

__attribute__ ((noreturn))
void exception_handler(void);

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

void init_idt();


#endif
