#include <stdint.h>
#ifndef INTERUPT_H
#define INTERUPT_H

void set_idt_entry(uint8_t vector, void* handler, uint8_t dpl); 

void load_idt(void* idt_addr);

#endif
