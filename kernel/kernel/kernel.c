#include <stddef.h>
#include <stdint.h>
#include "interupts.c"

#define PORT 0x3f8

// Start with creating the GDT I think
uint64_t gdt_entries[5]; // I think we keep this out of the function?
uint64_t num_gdt_entries = 5;


struct __attribute__((packed, aligned(4))) GDTR
{
	uint64_t limit;
	uint64_t address;
};

void load_GDT_entries (void)
{
	// Null descriptor
	gdt_entries[0] = 0;

	// Kernel code selector
	uint64_t kernel_code = 0;
	kernel_code |= 0b1011 << 8;
	kernel_code |= 1 << 12;
	kernel_code |= 0 << 13;
	kernel_code |= 1 << 15;

	gdt_entries[1] = kernel_code << 32;

	// Kernel data selector?
	uint64_t kernel_data = 0;
	kernel_data |= 0b0011 << 8;
	kernel_data |= 1 << 12;
	kernel_data |= 0 << 13;
	kernel_data |= 1 << 15;
	gdt_entries[2] = kernel_data << 32;
	
	// User code selector
	uint64_t user_code = kernel_code | (3 << 13);
	gdt_entries[3] = user_code;

	uint64_t user_data = kernel_data | (3 << 13);
	gdt_entries[4] = user_data;
	
	struct GDTR word_gdtr = 
	{
		num_gdt_entries * sizeof(uint64_t) - 1,
		(uint64_t) *gdt_entries
	};

	asm("lgdt %0" : : "m"(word_gdtr));
}

void flush_gdt()
{
	asm volatile("\
        mov $0x10, %ax \n\
        mov %ax, %ds \n\
        mov %ax, %es \n\
        mov %ax, %fs \n\
        mov %ax, %gs \n\
        mov %ax, %ss \n\
        \n\
	pop %edi \n\
	push $0x8 \n\
	push %edi \n\
        ret \n\
	");
}

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

void kernel_main() {	
	testFile();
	log_to_serial("Entries hopefully loaded here!");
	
}
