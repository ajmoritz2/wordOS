ARCHDIR?=kernel/arch/i386
KERNELDIR?=kernel/kernel
MEMDIR?=kernel/memory
MBDIR?=kernel/multiboot
DRIVERDIR?=kernel/drivers

CFLAGS:=-ffreestanding -std=gnu99
CC=i686-elf-gcc
AS=i686-elf-as

ASFILES?=boot.s
LDFILE?=linker.ld

KERNELC?=kernel.c


OBJS+=\
     $(ARCHDIR)/bootNP.o \
     $(MEMDIR)/pmm.o \
     $(MEMDIR)/vmm.o \
     $(MBDIR)/mb_parse.o \
     $(KERNELDIR)/gdt.o \
     $(KERNELDIR)/idt.o \
     $(MEMDIR)/paging.o \
     $(KERNELDIR)/kernel.o \
     $(MEMDIR)/string.o \
	 $(MEMDIR)/heap.o \
     $(KERNELDIR)/idtStub.o \
     $(DRIVERDIR)/apic.o \
	 $(DRIVERDIR)/timer.o \
	 $(DRIVERDIR)/keyboard.o \
	 $(DRIVERDIR)/framebuffer.o \
	 $(DRIVERDIR)/ps2.o		\
	 $(KERNELDIR)/scheduler.o \

include kernel/programs/local.mk
include kernel/utils/utils.mk

.PHONY: all, clean

all: word.bin


clean:   
	rm -rf $(OBJS)
word.bin: $(OBJS) $(ARCHDIR)/linkerNP.ld
	$(CC) -T $(ARCHDIR)/linkerNP.ld -o $@ $(CFLAGS) -nostdlib $(OBJS) fonts/font.o -lgcc
	grub-file --is-x86-multiboot2 word.bin

%.s: %.o
	$(AS) -std=gnu99 -ffreestanding $< -o $@

%.c: %.o
	$(CC) -g -c $< -o $@ $(CFLAGS)
