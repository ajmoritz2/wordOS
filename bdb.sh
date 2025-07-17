#!/bin/sh
echo --------------------------- CLEANED UP SYSTEM ---------------------------------
make
./iso.sh
./clean.sh
echo ---------------------------- SYSTEM STARTING -----------------------------------
qemu-system-i386 -s -S -d int -no-reboot -no-shutdown -serial file:words.log -cdrom wordos.iso -m 256M
