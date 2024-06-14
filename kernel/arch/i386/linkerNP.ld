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


	.text BLOCK (4K) : ALIGN(4K)
	{
		*(.text)
	}

	/* Read only data. */
	.rodata BLOCK(4K) : ALIGN(4K) 
	{
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read write data and stack! */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}

	_kernel_end = .;
	
	/* If there are other sections put em here! Not really sure how to, but I'll figure it out */
}
