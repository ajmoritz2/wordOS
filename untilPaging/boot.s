# Declare constants here
.set MAGIC_NUMBER, 0xE85250D6
.set ARCH, 0
.set PAGESIZE, 4096 # 3 MiB
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


.section .bss, "aw", @nobits
	.align 4096 # 4KiB aligned 
boot_page_dir:
	.skip 4096
boot_page_table1:
	.skip 20480
# give stuff to kernel

.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
	# Phys addr of boot_page_table1 is boot_page_table1 - 0xC0000000
	# 0xC0000000 is the address of the kernel because it is a higher half
	movl $(boot_page_table1 - 0xC0000000), %edi

	# First addr to map is addr 0
	movl $0, %esi
	# Map 1024 pages
	movl $1023, %ecx
1:
	# Only map the kernel
	cmpl $_kernel_start, %esi
	jl 2f
	cmpl $(_kernel_end - BOOTLOC), %esi
	jge 3f

	# Map physical address as present, writable
	# Write .text and .rodaata as well. Map those as non-writeable
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	# Size of each page is 4096
	addl $PAGESIZE, %esi
	# Size of entries are 4 bytes
	addl $4, %edi
	# Loop to next entry is not done
	loop 1b;

3:      movl $0xB8003, boot_page_table1 - 0xC0000000 + 1023 * 4

        movl $boot_page_table1, %ecx
        subl $0xC0000000, %ecx
        orl $0x003, %ecx
        movl %ecx, boot_page_dir - 0xC0000000
        movl %ecx, boot_page_dir - 0xC0000000 + 4 * 768

        movl $boot_page_dir, %ecx
        subl $0xC0000000, %ecx
        movl %ecx, %cr3

        movl %cr0, %ecx
        orl $0x80000000, %ecx
        movl %ecx, %cr0

	# Jump to higher half w/ absolute jump
	lea 4f, %ecx
	jmp *%ecx
.section .text
4:
	# Paging should be fully set up

	# Unmap the identity mapping
	
	# reload crc3 to force a TLB flush?
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up stack
	movl $(stack_top), %esp # Stack is 16 KiB long?

	# Transfer control to the main kernel
	call kernel_main
	pushl $boot_page_dir
	# If there is an unexpected return, hang on
	cli
1:	hlt
	jmp 1b

.global enable_paging
.type enable_paging, @function
enable_paging:
        pushl %ebp
        movl %esp, %ebp

        movl 8(%esp), %eax
        movl %eax, %cr3

        movl %cr0, %eax
        orl $0x80000001, %eax
        movl %eax, %cr0

        movl $0, %eax
        popl %ebp
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
        jmp $0x08, $.flush
.flush:
        ret
