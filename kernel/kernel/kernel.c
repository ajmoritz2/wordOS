#include <stddef.h>
#include <stdint.h>
#include "interupts.h"
#include "gdt.h"
#include "kernel.h"

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

void log_integer_to_serial (unsigned int number) {
	char final[32]; // Allow only 31 digits, because we have the null terminator here
	int i = 0;
	for (int num = number; num != 0; num = num/10) {
		final[i] = '0' + num%10;
		i++;
	}


	for (int j = 0, k = i - 1; j <= k; j++, k--) {
		char c = final[j];
		final[j] = final[k];
		final[k] = c;
	}	
	final[i] = 0;	

	log_to_serial(final);
}


// MAIN SECTION OF KERNEL

void kernel_main(uintptr_t *entry_pd) 
{
	gdt_install();
	init_serial();
	asm volatile("sti");
	log_to_serial("Entries hopefully loaded here!\n");
	log_to_serial("Maybe this will help me eyes.");
	//asm volatile("int $0x3");	
}
