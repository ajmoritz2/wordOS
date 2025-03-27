#!/bin/sh
if grub-file --is-x86-multiboot2 word.bin; then
	echo multiboot is great!
	mkdir -p isodir
	mkdir -p isodir/boot
	mkdir -p isodir/boot/grub
	cp word.bin isodir/boot/word.bin
	cat > isodir/boot/grub/grub.cfg << EOF
	menuentry "wordos" {
			multiboot2 /boot/word.bin
			boot
	}	

EOF
	grub-mkrescue -o wordos.iso isodir
	rm -rf word.bin
else
	echo This file is NOT multiboot!
fi
