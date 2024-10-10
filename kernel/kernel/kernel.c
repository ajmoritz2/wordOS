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
#include "../drivers/apic.h"
#include "../multiboot/mb_parse.h"



extern inline unsigned char inportb (int portnum)
{
	unsigned char data=0;
	__asm__ __volatile__ ("inb %%dx, %%al" : "=a" (data) : "d" (portnum));
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
				case 'd':
					log_integer_to_serial(va_arg(params, uint32_t));
					break;
				case 's':
					log_to_serial(va_arg(params, char *));
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

uint32_t get_stackp() 
{
	return 0;
}

// MAIN SECTION OF KERNEL

void kernel_main(uintptr_t *entry_pd, uint32_t multiboot_loc) 
{
	// TODO: Set the PIC properly. Timer is running out, so 0x08 is being called!
	if (multiboot_loc & 7)
	{
		log_to_serial("ERROR\n");
		return;
	}
	
	disable_pic();
	uint32_t* kpd = pg_init(entry_pd);
	gdt_install();
	init_idt();
	kinit();
	
	vmm* kvmm = create_vmm(kpd);
	set_current_vmm(kvmm);
	
	init_apic(kvmm);

//	asm volatile("int $0x7");
	init_multiboot(multiboot_loc + 0xC0000000);

	log_to_serial("\nPROGRAM TO HALT! \n");

}
