.section .text

.macro isr_err_stub num
.global isr_stub_\num
.type isr_stub_\num, @function
isr_stub_\num:
	cli
	pushl $\num
	jmp isr_frame_as
.endm

.macro isr_no_err_stub num
.global isr_stub_\num
.type isr_stub\num, @function
isr_stub_\num:
	cli
	pushl $0
	pushl $\num
	jmp isr_frame_as
.endm

.macro irq_stub num 
.global irq_stub_\num
.type irq_stub_\num, @function
irq_stub_\num:
	cli
	pushl $10
	pushl $\num
	jmp irq_frame
.endm

isr_frame_as:
	pushl %ebp
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	pushl %ebp

	movl %cr0, %eax
	pushl %eax
	movl %cr2, %eax
	pushl %eax
	movl %cr3, %eax
	pushl %eax

	call isr_handler
	
	popl %eax
	movl %eax, %cr3
	popl %eax
	movl %eax, %cr2
	popl %eax
	movl %eax, %cr0

	popl %ebp
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	popl %ebp
	addl $8, %esp
	sti
	iret

irq_frame:
	pushl %ebp
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	movl %esp, %eax
	addl $0x30, %eax
	pushl %eax 	#Reset ESP

	movl %cr0, %eax
	pushl %eax
	movl %cr2, %eax
	pushl %eax
	movl %cr3, %eax
	pushl %eax 


	pushl %esp # different because we are 32 bit i guess, so its all just pushed onto the stack
	call irq_handler
	movl (%eax), %ebx

	movl 0xc(%eax), %esp
	
	pushl 0x3c(%eax) # EFLAGS 
	pushl 0x38(%eax) # CS
	pushl 0x34(%eax) # EIP
	pushl 0x24(%eax) # eax
	movl 0x4(%eax), %ebx
	movl %ebx, %cr2
	movl 0x8(%eax), %ebx
	movl %ebx, %cr0

	movl 0x10(%eax), %edi	
	movl 0x14(%eax), %esi	
	movl 0x18(%eax), %edx	
	movl 0x1c(%eax), %ecx	
	movl 0x20(%eax), %ebx	
	movl 0x28(%eax), %ebp
	popl %eax
	sti
	iret

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

irq_stub 48
irq_stub 49
irq_stub 50
