# Declare constants here
.set MAGIC_NUMBER, 0xE85250D6
.set ARCH, 0
.set PAGESIZE, 4096 # 4 MiB
.set BOOTLOC, 0xC0000000 # Loc of kernel


.align 64
.section .mb2_hdr, "aw"
# Start of header
mb2_hdr_begin:
.align 4
.long MAGIC_NUMBER
.long ARCH
.long (mb2_hdr_end - mb2_hdr_begin) 
.long -(MAGIC_NUMBER + (mb2_hdr_end - mb2_hdr_begin))
mb2_frame_buffer_req:
	.short 0x05 # Tag identifier
	.short 0x1 # Flag 
	.long (mb2_framebuffer_end - mb2_frame_buffer_req) # Should be 20 if I read the docs correct
	.long 800
	.long 600
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
	.skip  4096# 16 KiB More than enough for a nice stack
stack_top:

.section .idt_setup, "aw"
# GDT in here too because why not!
.align 16
gdtr_struct:
	.short 24
	.long gdt_start - BOOTLOC
gdt_start:
	.word 0, 0
	.byte 0, 0, 0, 0 # NULL DESCRIPTOR

	.word 0xffff, 0x0
	.byte 0, 0x9a, 0xcf, 0 # CS DESCRIPTOR

	.word 0xffff, 0
	.byte 0, 0x93, 0xcf, 0
gdt_end:

.align 16
idtr_struct:
	.short	264
	.long	idt_start - BOOTLOC
idt_start:
idt_irq1:
	.short	0x1234				#offset bottom
	.short 	0x10				#segment selector
	.byte 	0 					#reserved
	.byte	0x8e 				#gate descriptor + flags
	.short	0	#offset top
idt_end:
.skip 264

# create paging dir
.section .bss, "aw", @nobits
	.align 4096 # gotta align the dir for speed
boot_page_directory:
	.skip 4096 # Its 4 KiB long
boot_page_table1:
	.skip 4096  # I think this ends up being 1 pages allocated?

.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:	
	#in between bit 0x10012 is set
	# CR0: 0x10013
	# CR4: 0x648
	# CR0 - EDX < 0 WE DONT CRASH EDX IS LARGER THAN CR0
	mov $0, %ecx
	mov $(boot_page_directory - 0xC0000000), %eax
fill_garbage:
	movl $0x0, (%eax)
	movl $0x00000000, 4096(%eax)
	add $4, %eax

	add $1, %ecx
	cmp $1024, %ecx
	jl fill_garbage
end_garbage:
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

	#movl $(boot_page_table2 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 769 * 4
	# The 768 is the 0xC0000000/0x400000 because each pde is 4 MiB and the 4 is bytes in 32-bit int.
	# Set cr3 to the address of the boot_page_directory.
	


	# Loading the IDT
	mov $(idt_start - BOOTLOC), %esi
	mov $(idt_start + 256 - BOOTLOC), %edi

idt_entry_loop:
	# Loads the first 32 bits
	
	mov %esi, %eax
	mov $irq_asm_1, %ecx
	andl $0xffff, %ecx
	orl $0x00080000, %ecx
	mov %ecx, (%eax)

	addl $4, %eax
	
	mov $irq_asm_1, %ecx
	andl $0x00ff0000, %ecx
	orl $0x00008e00, %ecx
	mov %ecx, (%eax)
	
	addl $8, %esi
	cmpl %esi, %edi
	jg idt_entry_loop
idt_entry_loop_end:

	lidt idtr_struct - BOOTLOC
	lgdt gdtr_struct - BOOTLOC
	

	movl $(boot_page_directory - BOOTLOC + 0x3), boot_page_directory - BOOTLOC + (1023 * 4)

	movl $(boot_page_directory - BOOTLOC), %ecx
	movl %ecx, %cr3	

	mov %cr3, %eax
	mov 0x10b400, %eax
	cmpl $0x100003, %eax # if eax > num we do not crash
	#je test	#if memory stored at 0x10b400 > 0x100003 then we do not crash
	cli

	movl %cr0, %ecx
	orl $0x80000001, %ecx # Enables paging
	movl %ecx, %cr0
	
	# Set up idt
	lea 4f, %ecx # Loading the memory address of 4 into the register ecx
	jmp 4f	# I believe the * means long jump and we are jumping to the effective address of 4
	# Here we just made the virtual address to be in the 0xC0000000 range. We are still in the 0x100000 range physically! 
.section .text
4:
	# Unmap the identity paging we did
	movl $0, boot_page_directory + 0

	# Reload cr3 for a TLB flush
	movl %cr3, %ecx
	movl %ecx, %cr3


	pushl %ebx
	pushl $(boot_page_directory)

	# Transfer control to the main kernel
	call kernel_main

	# If there is an unexpected return, hang on
	cli
1:	hlt
	jmp 1b


.global irq_asm_1
.type irq_asm_1, @function
irq_asm_1:
	mov $0xcee0beef, %ebx

.global load_directory
.type load_directory, @function
load_directory:
	push %ebp
	mov %esp, %ebp
	mov 8(%ebp), %eax
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
	mov 4(%esp), %eax
        cli
        lgdt (%eax)
        movw $0x10, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs
        movw %ax, %ss
        jmp $0x08, $flush
flush:
	ret
