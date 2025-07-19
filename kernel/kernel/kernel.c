#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "idt.h"
#include "gdt.h"
#include "kernel.h"
#include "../memory/string.h"
#include "../memory/pmm.h"
#include "../memory/paging.h"
#include "../memory/vmm.h"
#include "../memory/heap.h"
#include "../drivers/apic.h"
#include "../drivers/timer.h"
#include "../drivers/framebuffer.h"
#include "../drivers/keyboard.h"
#include "../multiboot/mb_parse.h"
#include "../programs/terminal.h"

extern char _binary_font_psf_start;

uint8_t task_state = 0;

extern inline unsigned char inportb (int portnum)
{
	unsigned char data=0;
	__asm__ volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (portnum) : "memory");
	return data;
}

extern inline void outportb (int portnum, unsigned char data)
{
	__asm__ __volatile__ ("outb %%al, %%dx" :: "a" (data), "d" (portnum));
}

static int init_serial() {
	outportb(PORT + 1, 0x00); 	// Disables interrupts
	outportb(PORT + 3, 0x80);	// Enable DLAB
	outportb(PORT + 0, 0x03);
	outportb(PORT + 1, 0x00);
	outportb(PORT + 3, 0x03);
	outportb(PORT + 2, 0xC7);
	outportb(PORT + 4, 0x0B);
	outportb(PORT + 4, 0x1E);
	outportb(PORT + 0, 0xAE);

	if (inportb(PORT + 0) != 0xAE) {
		return 1;
	}

	outportb(PORT + 4, 0x0F);
	return 0;
}

void log_to_serial (char *string) {
	for (char character = *string; character != '\0'; character = *++string)
	{
		outportb(PORT, character);
	}
}

void log_integer_to_serial (uint64_t number) {
	char final[64]; // Allow only 31 digits, because we have the null terminator here
	uint32_t i = 0;
	if (number < 10) {
		final[0] = '0' + number;
		final[1] = 0;
		log_to_serial(final);
		return;
	}

	for (uint32_t num = number; num != 0; num = num/10) {
		final[i] = '0' + num%10;
		i++;
	}

	for (uint32_t j = 0, k = i - 1; j <= k; j++, k--) {
		char c = final[j];
		final[j] = final[k];
		final[k] = c;
	}
	final[i] = 0;	
	log_to_serial(final);
}

void print_hex(uint64_t number)
{
	if (number == 0) { // Hack
		log_to_serial("0x0");
		return;
	}
	char final[64];
	final[0] = '0';
	final[1] = 'x';
	uint32_t i = 2;
	for (uint32_t num = number; num != 0; num = num/16) {
	 	uint32_t value = num%16;
		if (value < 10) {
			final[i] = '0' + value;
		} else {
			final[i] = 'a' + (value-10);
		}

		i++;
	}
	
	for (uint32_t j = 2, k = i - 1; j <= k; j++, k--) {
		char c = final[k];
		final[k] = final[j];
		final[j] = c;
	}

	final[i] = 0;
	log_to_serial(final);
}

void logf(char *string, ...)
{
	va_list params;
	va_start(params, string); 
	while (*string) {
		if (*string == '%') {
			*string++;
			switch (*string) {
				case 'x':
					print_hex(va_arg(params, uint32_t));
					break;
				case '%':
					outportb(PORT, *string);
					break;
				case 't':
					string+=2;
					break;
				case 'd':
					log_integer_to_serial(va_arg(params, uint32_t));
					break;
				case 's':
					log_to_serial(va_arg(params, char *));
					break;
				case 'c':
					char character = (char) va_arg(params, uint32_t);
					outportb(PORT, character);
					break;
				default:
					log_to_serial("UNKNOWN OPTION %");
					outportb(PORT, *string);
					log_to_serial("!\n");
					break;
			}
			*string++;
			continue;
		}

		outportb(PORT, *string);

		*string++;
	}

	va_end(params);
}

void panic(char* reason) {
	logf(" << KERNEL PANIC >> Reason: %s\n", reason);
// Will need to change for before framebuffer is setup...
	asm volatile ("cli \n\
					hlt");
}

void kernel_main(uintptr_t *entry_pd, uint32_t multiboot_loc) 
{

	/* Memory locations which are hardcoded for kernel use
	 * 0x7FE2000 -> 0x7FE2000 								:: I assume this is for the RSDT
	 * 0xC0100000 - 0xC0114000 -> 0x00100000 - 0x00114000 	:: This is the kernel itself. 
	 * 0xCC000000 - 0xFFE00000 -> ANY						:: This is the range of dynamic memory
	 * 0xFF7E8000 -> DYNAMIC								:: Start of the vmm organizer
 	 * 0xFF7E8000 - 0xFFFFF000 -> page table entrys			:: This is for recursive paging I believe...
 	 */

	if (multiboot_loc & 7)
	{
		log_to_serial("ERROR\n");
		return;
	}
	uint32_t* tag_size = (uint32_t*) (multiboot_loc + 0xC0000000);
	uint32_t kernel_size = ((uint32_t) &_kernel_end - ((uint32_t)&_kernel_start + 0xC0000000));
	logf("Kernel Size %x, multiboot_loc: %x, tag_size: %x\n", kernel_size, multiboot_loc, *tag_size);
	disable_pic();
	uint32_t* kpd = pg_init(entry_pd, *tag_size);
	gdt_install();
	init_idt();
	uint32_t kalloc_bottom = kinit((uint32_t *) (multiboot_loc + *tag_size)); // Bandaid fix done here. Will bite me in the ass later...
	vmm* kvmm = create_vmm(kpd, 0xCC000000, 0xFFE00000);
	set_current_vmm(kvmm);

	logf("PSF START: %x\n", *&_binary_font_psf_start);
	struct multiboot_tag_pointers tags = init_multiboot(multiboot_loc + 0xC0000000); // Must call before init_apic to properly parse. Must also be done after paging
	
	transfer_dynamic();
	vmm_transfer_dynamic(kpd);
	// Do everything you want with the multiboot tags before this point. Past here it will be overwritten.	
	init_apic(kvmm);
	init_framebuffer();
	init_font();
	// Can use text now!
	
	logf("KERNEL STARTING LOC: %x KERNEL ENDING LOC: %x SIZE: %x\n", &_kernel_start, &_kernel_end, kernel_size); 
	init_heap();
	init_terminal();
	init_keyboard();
	printf("Welcome to WordOS. Home of the METS!\n");
	printf("Fuck the yankees... %t30May the dodgers win!%t10\n");

	printf("Basic kernel function %t30OK!%t10\n");

	printf("Starting terminal...\n");
	start_terminal();
	while (1) {
		terminal_loop();

		if (!task_state)
			asm volatile ("hlt");
	}
	log_to_serial("\nPROGRAM TO HALT! \n");

}
