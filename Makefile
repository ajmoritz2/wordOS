ARCHDIR?=kernel/arch/i386
KERNELDIR?=kernel/kernel

CFLAGS:=-ffreestanding -std=gnu99
CC=i686-elf-gcc
AS=i686-elf-as

ASFILES?=boot.s
LDFILE?=linker.ld

KERNELC?=kernel.c

OBJS=\
     $(ARCHDIR)/bootNP.o \
     $(KERNELDIR)/gdt.o \
     $(KERNELDIR)/idtStub.o \
     $(KERNELDIR)/interupts.o \
     $(KERNELDIR)/kernel.o

.PHONY: all

all: word.bin

word.bin: $(OBJS) $(ARCHDIR)/linkerNP.ld
	$(CC) -T $(ARCHDIR)/linkerNP.ld -o $@ $(CFLAGS) -nostdlib $(OBJS) -lgcc
	grub-file --is-x86-multiboot2 word.bin

%.s: %.o
	$(CC) -std=gnu99 -ffreestanding $< -o $@

%.c: %.o
	$(CC) -c $< -o $@ $(CFLAGS)

