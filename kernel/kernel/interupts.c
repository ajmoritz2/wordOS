#include <stdint.h>
#include <stdbool.h>
#include "interupts.h"
#include "kernel.h"


__attribute__((aligned(0x1000)))
static idt_entry idt[256];

static idtr_t idtr;

static bool vectors[256];

extern void* isr_stub_table[];

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags)
{
	idt_entry* desc = &idt[vector];

	desc->address_low = (uint32_t)isr & 0xFFFF;
	desc->selector = 0x8;
	desc->flags = flags;
	desc->address_high = (uint32_t)isr >> 16;
	desc->reserved = 0;
}

void init_idt() 
{
	idtr.base = (uintptr_t)&idt[0];
	idtr.limit = (uint16_t)sizeof(idt_entry) * 256;

	for (uint8_t vector = 0; vector < 32; vector++) {
		idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
		vectors[vector] = true;
	}

	log_to_serial("Loaded IDT");
	asm volatile ("lidt %0" : : "m"(idtr)); // Loading new IDT
	asm volatile ("sti"); // Interupt flag
}

__attribute__((noreturn))
void exception_handler() 
{
	log_to_serial("INTERUPT ACHIEVED"); 
	asm volatile("cli; hlt");
}
