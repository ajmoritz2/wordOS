# Declare constants here
.set MAGIC_NUMBER, 0xE85250D6
.set ARCH, 0
.set PAGESIZE, 4096 # 4 MiB
.set BOOTLOC, 0xC0000000 # Loc of kernel

.section .mb2_hdr, "aw"
# Start of header
mb2_hdr_begin:
.align 4
.long MAGIC_NUMBER
.long ARCH
.long (mb2_hdr_end - mb2_hdr_begin) 
.long -(MAGIC_NUMBER + (mb2_hdr_end - mb2_hdr_begin))

mb2_frame_buffer_req:
	.short 5 # Tag identifier
	.short 1 # Flag 
	.long (mb2_framebuffer_end - mb2_frame_buffer_req) # Should be 20 if I read the docs correct
	.long 0
	.long 0
	.long 0
mb2_framebuffer_end: # Puts in an address space
.align 8
# the end tag: type = 0, size = 8
.long 0
.long 8

mb2_hdr_end:

# END OF THE HEADER FILE

# STARTING SHIM PORTION
# init all the stack
.section .bootstrap_stack, "aw", @nobits
boot_stack_base:
	.skip  16384 # 16 KiB
stack_top:

# create paging dir
.section .bss, "aw", @nobits
	.align 4096 # gotta align the dir for speed
boot_page_directory:
	.skip 4096 # Its 3 KiB long
boot_page_table1:
	.skip 20480  # I think this ends up being 5 pages allocated?

.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:

	mov $stack_top, %esp
	movl $(boot_page_table1 - 0xC0000000), %edi # Here we are just placing values into registers to properly map
	# ^ is the phys addr of the boot page table
	movl $0, %esi
	movl $1024, %ecx # Mapping 1024 pages

1:
	cmpl $_kernel_start, %esi # Doing esi (0) - _kernel_start (0x00100000)
	jl 2f			  # If esi - _kernel_start < 0 jmp 2
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge 3f 			  # If esi - _kernel_end (Physical addr) >= 0 jmp 3	

	# Map address as "P and R/W".
	movl %esi, %edx # Moving copying esi to edx so we can set those flags
	orl $0x003, % edx # setting bits 0 and 1
		
	# Putting the present bits and read write into the correct spot in the boot_page_tables
	movl %edx, (%edi) # (%edi) means the memory at the addr of what is stored in %edi

2: 
	addl $4096, %esi # 4096 is the size of 1 page
	# Each entry is 4 bytes long
	addl $4, %edi

	# Loop to the next entry if not complete
	loop 1b
3:
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

	# Set cr3 to the address of the boot_page_directory.
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3	

	movl %cr0, %ecx
	orl $0x80000000, %ecx # Enables paging
	movl %ecx, %cr0
	
	lea 4f, %ecx # Loading the memory address of 4 into the register ecx
	jmp *%ecx	# I believe the * means long jump and we are jumping to the effective address of 4
.section .text
4:
	# Unmap the identity paging we did
	movl $0, boot_page_directory + 0

	# Reload cr3 for a TLB flush
	movl %cr3, %ecx
	movl %ecx, %cr3



	pushl $(boot_page_directory)
	# Transfer control to the main kernel
	call kernel_main

	# If there is an unexpected return, hang on

	cli
1:	hlt
	jmp 1b


.global load_directory
.type load_directory, @function
load_directory:
	push %ebp
	mov %esp, %ebp
	mov 8(%esp), %eax
	mov %eax, %cr3
	mov %ebp, %esp
	pop %ebp
	ret
.global enable_paging
.type enable_paging, @function
enable_paging:
	push %ebp
	mov %esp, %ebp
	mov %cr0, %eax
	or $0x80000000, %eax
	mov %eax, %cr0
	mov %ebp, %esp
	pop %ebp
	ret

.global flush_gdt
.type flush_gdt, @function
flush_gdt:
        cli
        lgdt (gp)
        movw $0x10, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs
        movw %ax, %ss
        jmp $0x08, $flush
flush:
        ret
