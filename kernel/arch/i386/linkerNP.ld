ENTRY(_start)

/* Organization of sections */

SECTIONS
{
	/* Gonna begin at 1 MiB because I want to be more safe */
	. = 0x00100000;
	/* Kernel lives ata 3GB + 1MB in the VAS, mapped to 1MB phyiscally */

	_kernel_start = .;

	/* Start by putting the header in */
	.mb2_hdr : 
	{
		*(.mb2_hdr)
	}

	.multiboot.text : {
		*(.multiboot.text)
	}

	. += 0xC0000000;

	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
	}

	/* Read only data. */
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	}

	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}

	/* Read write data and stack! */
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bootstrap_stack)
		*(.bss)
	}

	_kernel_end = .;
	
	/* If there are other sections put em here! Not really sure how to, but I'll figure it out */
}
