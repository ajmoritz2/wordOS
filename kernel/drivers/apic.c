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
#include "../multiboot/mb_parse.h"
#include "apic.h"

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

extern vmm* current_vmm;

uint32_t* glob_apic_addr = 0; // TODO: Create a core struct to keep track of core specific stuff
uint32_t volatile* glob_ioapic_addr = 0;

// Assembly Instructions START
uint32_t* get_apic_addr()
{
	// TODO: change this to a get_msr function instead...
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
	logf("MSR EAX: %x\n", eax);
	return (uint32_t*)(eax & 0xFFFFF000);
}

void get_cpuid(int eax_val)
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

	asm volatile ("mov %[eax_val], %%eax \n \
					cpuid \n \
					mov %%eax, %[eax] \n \
					mov %%ebx, %[ebx] \n \
				   	mov %%ecx, %[ecx] \n \
					mov %%edx, %[edx]"
					: [eax] "=m"(eax),
						[ebx] "=m"(ebx),
						[ecx] "=m"(ecx),
						[edx] "=m"(edx)
					: [eax_val] "r"(eax_val));

	logf("CPU_ID: %d EAX EBX ECX EDX\n%x\n%x\n%x\n%x\nEND...\n", eax_val, eax, ebx, ecx, edx);	
}

// TIMER Instructions START

void set_pit_count(uint16_t count)
{
	// PIT runs at 1,193,180 Hz
	outportb(0x43, 0x34); // 0x43 is the PIT command port. 0x34 is a bitfield.
	outportb(0x40, count & 0xFF);
   	outportb(0x40, count >> 8);	
}

void timer_init(void* lapic_addr)
{
	volatile uint32_t* lvt_reg = (uint32_t*) ((uint32_t) lapic_addr + 0x320);

	*lvt_reg = 0x30; // Just setting up the vector for the timer

	set_initial_timer_count(lapic_addr, 0xFFFFFFFF);

	uint32_t volatile *timer_count = (uint32_t*) ((uint32_t) lapic_addr + 0x390);
	logf("APIC Timer initialized\n");
}

void set_initial_timer_count(void* lapic_addr, uint32_t count)
{
	volatile uint32_t* init_reg = (uint32_t*) ((uint32_t) lapic_addr + 0x380);
	*init_reg = count;

}

// IOAPIC Baloney
uint32_t read_ioapic_register(uint32_t reg)
{
	*glob_ioapic_addr = reg;
	uint32_t* access_point = (uint32_t*) ((uint32_t) glob_ioapic_addr + 0x10);

	return *access_point;
}

void write_ioapic_register(uint32_t reg, uint32_t data)
{	
	*glob_ioapic_addr = reg;

	uint32_t* access_point = (uint32_t*) ((uint32_t) glob_ioapic_addr + 0x10);
	*access_point = data;
}

void init_ioapic()
{
	logf("Running...\n");
	struct MADTEntryType1* ioapicStruct = (struct MADTEntryType1*)parse_MADT(1);

	void* io_mapping = page_kalloc(1024, 0x3, ioapicStruct->IOAPICAddr);

	glob_ioapic_addr = (uint32_t*) (io_mapping);

	// Read Registers at addr + 0x10. Insert reg to read at addr + 0
	logf("IOAPIC found at addr %x\n", glob_ioapic_addr);

	read_ioapic_register(1);	
}

// LAPIC Stuff
void send_EOI(void* apic_addr) 
{
	uint32_t* eoi_reg = (uint32_t*) ((uint32_t) apic_addr + EOI);

	*eoi_reg = 0;
}

void init_apic()
{
	uint32_t* apic_addr = (uint32_t*)0xC1000000; // TODO: assign dynamically once paging is fixed
	glob_apic_addr = apic_addr; // TODO: Create a global core struct (stated earlier as well)
	uint32_t* apic_addr_phys = get_apic_addr();
	uint32_t volatile* spur_vec = (uint32_t*) ((uint32_t)apic_addr + SPUR_VEC);
	logf("Spur vec at %x\n", spur_vec);
	memory_map(current_vmm->root_pd, apic_addr_phys, (uint32_t*)apic_addr, 0x113); // Flags are R/W Cache disable, Global and pres! 
	*spur_vec = *spur_vec | 0x1FF; // Map vec to 0xF0 entry and enable
	
	timer_init(apic_addr);
	set_pit_count(1193);

	init_ioapic();
}



// PIC Legacy stuff
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


