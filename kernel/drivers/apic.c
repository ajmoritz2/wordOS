/* APIC.c 10/05/24
 *
 * CREATED FOR WORDOS
 *
 * HEADER: header/apic.h
 */

#include "../kernel/kernel.h"
#include <stdint.h>
#include "../memory/paging.h"
#include "../memory/vmm.h"

#define PIC1		0x20
#define PIC2		0xA0
#define PIC1_COMMAND	PIC1
#define PIC2_COMMAND 	PIC2
#define PIC1_DATA	(PIC1+1)
#define PIC2_DATA	(PIC2+1)

#define	ICW_1		0x11
#define PIC1_ICW_2	0x20
#define PIC2_ICW_2	0x28
#define PIC1_ICW_3 	0x02
#define PIC2_ICW_3	0x04
#define ICW_4		0x01

#define SPUR_VEC 	0xF0
#define EOI		0xB0
#define TIMER_LVT	0x320
#define LOCAL_APIC_ID	0x20

uint32_t* get_apic_addr()
{
	uint32_t msr = 0x1b;
	uint32_t eax = 0; 
	uint32_t edx = 0;
	asm volatile ("mov %[msr], %%ecx \n \
			rdmsr \n \
			mov %%eax, %[eax] \n \
			mov %%edx, %[edx]" 
			: [eax] "=m"(eax),
			  [edx] "=m"(edx) 
			: [msr] "r"(msr));
	return (uint32_t*)(eax & ~0xFFF);
}

void init_apic(vmm* current_vmm)
{
	uint32_t* apic_addr = get_apic_addr();
	uint32_t* spur_vec = (uint32_t*) ((uint32_t)apic_addr + SPUR_VEC);
	logf("Spur vec at %x\n", spur_vec);
	memory_map(current_vmm->root_pd, apic_addr, (uint32_t*)apic_addr, 0x3); 
	*spur_vec = *spur_vec | 0xF0 | 0x100; // Map vec to 0xF0 entry and enable
	
	uint32_t* id_addr = (uint32_t*) ((uint32_t)apic_addr + 0x20);
	logf("LOCAL APIC ID: %d at %x\n", *id_addr, id_addr);
}

void disable_pic()
{

	outportb(PIC1_COMMAND, ICW_1);
	io_wait();
	outportb(PIC2_COMMAND, ICW_1);
	io_wait();
	outportb(PIC1_DATA, PIC1_ICW_2);
	io_wait();
	outportb(PIC2_DATA, PIC2_ICW_2);
	io_wait();
	outportb(PIC1_DATA, PIC1_ICW_3);
	io_wait();
	outportb(PIC2_DATA, PIC2_ICW_3);
	io_wait();
	outportb(PIC1_DATA, ICW_4);
	io_wait();
	outportb(PIC2_DATA, ICW_4);
	io_wait();
	outportb(PIC1_DATA, 0xFF);
	io_wait();
	outportb(PIC2_DATA, 0xFF);
}


