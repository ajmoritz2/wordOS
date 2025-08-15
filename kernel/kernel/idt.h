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

typedef struct isr_frame {
	uint32_t cr3;
	uint32_t cr2;
	uint32_t cr0;
	
	uint32_t esp;
	uint32_t edi;
	uint32_t esi;
	uint32_t edx;
	uint32_t ecx;
	uint32_t ebx;
	uint32_t eax;

	uint32_t isr_no;
	uint32_t isr_err;

	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;	
} __attribute__((packed)) cpu_status_t;

extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);
extern void isr_stub_20(void);
extern void isr_stub_21(void);
extern void isr_stub_22(void);
extern void isr_stub_23(void);
extern void isr_stub_24(void);
extern void isr_stub_25(void);
extern void isr_stub_26(void);
extern void isr_stub_27(void);
extern void isr_stub_28(void);
extern void isr_stub_29(void);
extern void isr_stub_30(void);
extern void isr_stub_31(void);

extern void irq_stub_48(void);
extern void irq_stub_49(void);
extern void irq_stub_50(void);

void exception_handler(struct isr_frame frame);

void idt_set_descriptor(uint8_t vector, uint32_t  isr, uint8_t flags);

void core_dump(struct isr_frame *frame);

void init_idt(void);


#endif
