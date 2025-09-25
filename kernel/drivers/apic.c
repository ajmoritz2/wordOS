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
#include "../programs/terminal.h"
#include "keyboard.h"
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

uint32_t* glob_lapic_addr = 0; // TODO: Create a core struct to keep track of core specific stuff
uint32_t volatile* glob_ioapic_addr = 0;
uint32_t ticks_per_ms = 0;

uint32_t calibrating = 0;
struct MADTEntryType2 test_try = {2, 2, 2, 2};

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
	return (uint32_t*)(eax & 0xFFFFF000);
}

struct cpuid_status get_cpuid(int eax_val)
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

	struct cpuid_status ret_val = {eax, ebx, ecx, edx};

	logf("CPU_ID: %x EAX EBX ECX EDX\n%x\n%x\n%x\n%x\nEND...\n", eax_val, eax, ebx, ecx, edx);	

	return ret_val;
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

uint8_t ioapic_pit_irq()
{
	uint8_t count = 1;

	struct MADTEntryType2* candidate = NULL;

	for (candidate = (struct MADTEntryType2*)parse_MADT(2, count); 
			candidate != NULL; 
			candidate = (struct MADTEntryType2*) parse_MADT(2, count), count++) {
		if (candidate->BusSource == 0)
			break;
	}

	return candidate->GSI;
}



void init_ioapic()
{
	struct MADTEntryType1* ioapicStruct = (struct MADTEntryType1*)parse_MADT(1, 1);
	
	glob_ioapic_addr = page_kalloc(1024, 0x113, ioapicStruct->IOAPICAddr); // I Pray strong Uncacheable

	// Set up PIT timer on the ioapic
	uint8_t pit_reg = (ioapic_pit_irq() * 2) + 0x10;
	write_ioapic_register(pit_reg, 49); // Setting the PIT irq vec
	write_ioapic_register(pit_reg+1, 0);

	struct MADTEntryType2 *ISOS = (struct MADTEntryType2 *) parse_MADT(2, 3);

	if (ISOS) {
		test_try = *ISOS;
		logf("Test try at %x\n", &test_try);
	}

}

void print_test_try()
{
	printf("BUS: %x\nSource: %x\nGSI: %x\nFlags: %x\n", test_try.BusSource, test_try.IRQSource, test_try.GSI, test_try.Flags);
}
// LAPIC Stuff
// TIMER Instructions START

void lapic_write_reg(uint32_t reg, uint32_t data)
{
	volatile uint32_t* lapic_reg = (uint32_t*) ((uint32_t) glob_lapic_addr + reg);

	*lapic_reg = data;
}

uint32_t lapic_read_reg(uint32_t reg)
{
	volatile uint32_t* lapic_reg = (uint32_t*) ((uint32_t) glob_lapic_addr + reg);
	return *lapic_reg;
}

void lapic_timer_init()
{
	// Real init is done in calibration ig
	volatile uint32_t* lvt_reg = (uint32_t*) ((uint32_t) glob_lapic_addr + 0x320);

	*lvt_reg = 0x30 | 1 << 16; // Just setting up the vector for the time
}

void set_initial_lapic_timer_count(uint32_t count)
{
	volatile uint32_t* init_reg = (uint32_t*) ((uint32_t) glob_lapic_addr + LAPIC_TIMER_SET_REG);
	*init_reg = count;
}

uint32_t get_lapic_timer_count()
{
	return lapic_read_reg(0x390); 
}

void send_EOI(void* apic_addr) 
{
	uint32_t* eoi_reg = (uint32_t*) ((uint32_t) apic_addr + EOI);

	*eoi_reg = 0;
}

void init_apic()
{
	logf("\n----Starting APIC Initializations----\n");
	glob_lapic_addr = (uint32_t*)page_kalloc(4096, 0x3, (uint32_t)get_apic_addr());; // TODO: Create a global core struct (stated earlier as well)
	uint32_t volatile* spur_vec = (uint32_t*) ((uint32_t)glob_lapic_addr + SPUR_VEC);
	*spur_vec = *spur_vec | 0x1FF; // Map vec to 0xF0 entry and enable
								   //
	uint32_t apic_id = 0;
	apic_id = lapic_read_reg(0x20);
	logf("APIC ID IS: %x\n\n", apic_id);
	
	lapic_timer_init();

	init_ioapic();
	calibrate_timer();
}

// Calibration
void calibrate_timer()
{
	uint32_t sum = 0;
	lapic_write_reg(0x320, 1 << 16);	
	for (int i = 0; i < 10; i++) { // Calibrate 10 times to get decent accuracy
		calibrating = 1;
		set_pit_one_shot(1197 * 10);
		set_initial_lapic_timer_count(0xFFFFFFFF);

		while (calibrating); // Pause Code execution

		uint32_t count_in_10_ms = 0xFFFFFFFF - lapic_read_reg(0x390);
		sum += count_in_10_ms;
	}

	ticks_per_ms = sum / (100); // 50 because its runs at 2 cylces per tick.

	logf("Calibration complete! Ticks per ms: %d\n", ticks_per_ms);
	lapic_write_reg(0x320, 0x30 | 1 << 17);
	//lapic_write_reg(0x3E0, 0x3); // running at 16 cycles per tick.
}

uint32_t end_calibration()
{
	calibrating = 0;
}

// PIT
void set_pit_one_shot(uint16_t count) {
	outportb(0x43, 0x30);
	outportb(0x40, count & 0xFF); //low-byte
	outportb(0x40, count >> 8); //high-byte
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


