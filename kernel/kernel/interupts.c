#include <stdint.h>
#include <stdbool.h>
#include "interupts.h"
#include "kernel.h"


__attribute__((aligned(0x0010)))
static idt_entry idt[256];

static idtr_t idtr;


void idt_set_gate(uint8_t num, void(*isr)(void), uint8_t flags)
{
	idt_entry* desc = &idt[num];

	desc->address_low = (uint32_t)isr & 0xFFFF;
	desc->selector = 0x8;
	desc->flags = flags;
	desc->address_high = (uint32_t)isr >> 16;
	desc->reserved = 0;
}

void init_idt() 
{
	log_to_serial("HELLO WORLD\n");
	idtr.base = (uint32_t)idt;
	idtr.limit = (uint16_t)sizeof(idt_entry) * 256 - 1;

	idt_set_gate(0x0, isr_stub_0, 0x80 | 0x0E);
	idt_set_gate(0x1, isr_stub_1, 0x80 | 0x0E);
	idt_set_gate(0x2, isr_stub_2, 0x80 | 0x0E);
	idt_set_gate(0x3, isr_stub_3, 0x80 | 0x0E);
	idt_set_gate(0x4, isr_stub_4, 0x80 | 0x0E);
	idt_set_gate(0x5, isr_stub_5, 0x80 | 0x0E);
	idt_set_gate(0x6, isr_stub_6, 0x80 | 0x0E);
	idt_set_gate(0x7, isr_stub_7, 0x80 | 0x0E);
	idt_set_gate(0x8, isr_stub_8, 0x80 | 0x0E);
	idt_set_gate(0x9, isr_stub_9, 0x80 | 0x0E);
	idt_set_gate(0xA, isr_stub_10, 0x80 | 0x0E);
	idt_set_gate(0xB, isr_stub_11, 0x80 | 0x0E);
	idt_set_gate(0xC, isr_stub_12, 0x80 | 0x0E);
	idt_set_gate(0xD, isr_stub_13, 0x80 | 0x0E);
	idt_set_gate(0xE, isr_stub_14, 0x80 | 0x0E);
	idt_set_gate(0xF, isr_stub_15, 0x80 | 0x0E);
	idt_set_gate(0x10, isr_stub_16, 0x80 | 0x0E);
	idt_set_gate(0x11, isr_stub_17, 0x80 | 0x0E);
	idt_set_gate(0x12, isr_stub_18, 0x80 | 0x0E);
	idt_set_gate(0x13, isr_stub_19, 0x80 | 0x0E);
	idt_set_gate(0x14, isr_stub_20, 0x80 | 0x0E);
	idt_set_gate(0x15, isr_stub_21, 0x80 | 0x0E);
	idt_set_gate(0x16, isr_stub_22, 0x80 | 0x0E);
	idt_set_gate(0x17, isr_stub_23, 0x80 | 0x0E);
	idt_set_gate(0x18, isr_stub_24, 0x80 | 0x0E);
	idt_set_gate(0x19, isr_stub_25, 0x80 | 0x0E);
	idt_set_gate(0x1A, isr_stub_26, 0x80 | 0x0E);
	idt_set_gate(0x1B, isr_stub_27, 0x80 | 0x0E);
	idt_set_gate(0x1C, isr_stub_28, 0x80 | 0x0E);
	idt_set_gate(0x1D, isr_stub_29, 0x80 | 0x0E);
	idt_set_gate(0x1E, isr_stub_30, 0x80 | 0x0E);
	idt_set_gate(0x1F, isr_stub_31, 0x80 | 0x0E);



	log_to_serial("Loaded IDT\n");
	asm volatile ("lidt %0" : : "m"(idtr)); // Loading new IDT
	//asm volatile ("sti"); // Interupt flag
}

void exc_print(struct isr_frame *frame)
{
	switch (frame->isr_vector) {
		case 0x00:
			log_to_serial("Division by zero in kernel space!\n");
			break;
		case 0x06:
			log_to_serial("Invalid opcode in kernel space!\n");
			break;
		case 0x0D:
			log_to_serial("Double fault in kernel space!\n");
			break;
		case 0x0E:
			log_to_serial("Page fault error not coded in yet. :/ \n");
			break;
		default:
			log_to_serial("Yea no clue pal\n");
	}
}
void exception_handler(struct isr_frame frame)
{
	if (frame.isr_vector < 32) {
		exc_print(&frame);
	}
	asm volatile("cli\n\
			hlt");
}
