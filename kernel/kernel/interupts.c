#include <stdint.h>
#include <stdbool.h>
#include "interupts.h"
#include "header/paging.h"
#include "header/string.h"
#include "kernel.h"


__attribute__((aligned(0x0010)))
static idt_entry idt[256];

static idtr_t idtr;


void idt_set_gate(uint8_t num, uint32_t base, uint8_t flags)
{

	idt[num].address_low = base & 0xFFFF;
	idt[num].selector = 0x08;
	idt[num].flags = flags;
	idt[num].address_high = (base >> 16) & 0xFFFF;
	idt[num].reserved = 0;
}

void init_idt() 
{
	idtr.base = (uint32_t)&idt;
	idtr.limit = sizeof(idt_entry) * 256 - 1;
	memset(&idt, 0, sizeof(idt_entry)*64);
	idt_set_gate(0x0, (uint32_t)isr_stub_0, 0x80 | 0x0E);
	idt_set_gate(0x1, (uint32_t)isr_stub_1, 0x80 | 0x0E);
	idt_set_gate(0x2, (uint32_t)isr_stub_2, 0x80 | 0x0E);
	idt_set_gate(0x3, (uint32_t)isr_stub_3, 0x80 | 0x0E);
	idt_set_gate(0x4, (uint32_t)isr_stub_4, 0x80 | 0x0E);
	idt_set_gate(0x5, (uint32_t)isr_stub_5, 0x80 | 0x0E);
	idt_set_gate(0x6, (uint32_t)isr_stub_6, 0x80 | 0x0E);
	idt_set_gate(0x7, (uint32_t)isr_stub_7, 0x80 | 0x0E);
	idt_set_gate(0x8, (uint32_t)isr_stub_8, 0x80 | 0x0E);
	idt_set_gate(0x9, (uint32_t)isr_stub_9, 0x80 | 0x0E);
	idt_set_gate(0xA, (uint32_t)isr_stub_10, 0x80 | 0x0E);
	idt_set_gate(0xB, (uint32_t)isr_stub_11, 0x80 | 0x0E);
	idt_set_gate(0xC, (uint32_t)isr_stub_12, 0x80 | 0x0E);
	idt_set_gate(0xD, (uint32_t)isr_stub_13, 0x80 | 0x0E);
	idt_set_gate(0xE, (uint32_t)isr_stub_14, 0x80 | 0x0E);
	idt_set_gate(0xF, (uint32_t)isr_stub_15, 0x80 | 0x0E);
	idt_set_gate(0x10, (uint32_t)isr_stub_16, 0x80 | 0x0E);
	idt_set_gate(0x11, (uint32_t)isr_stub_17, 0x80 | 0x0E);
	idt_set_gate(0x12, (uint32_t)isr_stub_18, 0x80 | 0x0E);
	idt_set_gate(0x13, (uint32_t)isr_stub_19, 0x80 | 0x0E);
	idt_set_gate(0x14, (uint32_t)isr_stub_20, 0x80 | 0x0E);
	idt_set_gate(0x15, (uint32_t)isr_stub_21, 0x80 | 0x0E);
	idt_set_gate(0x16, (uint32_t)isr_stub_22, 0x80 | 0x0E);
	idt_set_gate(0x17, (uint32_t)isr_stub_23, 0x80 | 0x0E);
	idt_set_gate(0x18, (uint32_t)isr_stub_24, 0x80 | 0x0E);
	idt_set_gate(0x19, (uint32_t)isr_stub_25, 0x80 | 0x0E);
	idt_set_gate(0x1A, (uint32_t)isr_stub_26, 0x80 | 0x0E);
	idt_set_gate(0x1B, (uint32_t)isr_stub_27, 0x80 | 0x0E);
	idt_set_gate(0x1C, (uint32_t)isr_stub_28, 0x80 | 0x0E);
	idt_set_gate(0x1D, (uint32_t)isr_stub_29, 0x80 | 0x0E);
	idt_set_gate(0x1E, (uint32_t)isr_stub_30, 0x80 | 0x0E);
	idt_set_gate(0x1F, (uint32_t)isr_stub_31, 0x80 | 0x0E);



	log_to_serial("Loaded IDT\n");
	asm volatile ("lidt %0" : : "m"(idtr)); // Loading new IDT
	asm volatile ("sti"); // Interupt flag
}

void exc_print(struct isr_frame *frame)
{
	log_integer_to_serial(frame->isr_no);
	log_to_serial("\nCALLED TWICE\n");
	switch (frame->isr_no) {
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
			handle_exception(frame);
			break;
		default:
			log_integer_to_serial(frame->isr_no);
			break;
	}
}



void isr_handler(struct isr_frame frame)
{
	if (frame.isr_no < 32) {
		exc_print(&frame);
	}
	asm volatile("cli\n\
			hlt");
}
