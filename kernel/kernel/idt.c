#include <stdint.h>
#include <stdbool.h>
#include "idt.h"
#include "kernel.h"
#include "../memory/paging.h"
#include "../memory/string.h"
#include "../drivers/apic.h"
#include "../drivers/framebuffer.h"
#include "../drivers/keyboard.h"

extern uint32_t* glob_lapic_addr;

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
	memset(&idt, 0, sizeof(idt_entry)*256 - 1);
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

	idt_set_gate(0x30, (uint32_t)irq_stub_48, 0x80 | 0x0E); // LAPIC ID 0
	idt_set_gate(0x31, (uint32_t)irq_stub_49, 0x80 | 0x0E); // PIT
	idt_set_gate(0x32, (uint32_t)irq_stub_50, 0x80 | 0x0E); // Keyboard

	log_to_serial("Loaded IDT\n");
	asm volatile ("lidt %0" : : "m"(idtr)); // Loading new IDT
	asm volatile ("sti"); // Interupt flag
}

uint8_t exc_print(struct isr_frame *frame)
{
	logf("IP: %x\n", frame->eip);
	uint32_t code = 1;
	switch (frame->isr_no) {
		case 0x00:
			log_to_serial("Division by zero in kernel space!\n");
			break;
		case 0x06:
			log_to_serial("Invalid opcode in kernel space!\n");
			break;
		case 0x0D:
			logf("Err code: %x ", frame->isr_err);
			log_to_serial("Double fault in kernel space!\n");
			break;
		case 0x0E:
			code = handle_exception(frame);
			break;
		default:
			logf("Stack at %x, Interrupt occured: %d", frame->isr_err, frame->isr_no);
			log_to_serial("\n");
			break;
	}

	return code;
}

void irq_handler(int num) {
	switch (num) {
	case 0x30: // LAPIC Timer
		break;
	case 0x31: // PIT timer
		end_calibration();
		break;
	case 0x32:
		handle_keychange();
		break;
	case 0x80: // System calls for when that is added...
		break;
	}
	send_EOI(glob_lapic_addr); // THEY WILL QUEUE UP IF THIS ISN'T HERE!
}

void isr_handler(struct isr_frame frame)
{
	if (frame.isr_no < 32) {
		if (exc_print(&frame) == 0) {
			return;
		}
	}
	logf("Silly interrupt happened I guess!\n");
	asm volatile("cli\n\
			hlt");
}
