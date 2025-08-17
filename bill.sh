#!/bin/sh
echo --------------------------- CLEANED UP SYSTEM ---------------------------------
make
./iso.sh
./clean.sh
echo ---------------------------- SYSTEM STARTING -----------------------------------
qemu-system-i386 -d int -no-reboot -no-shutdown -serial stdio -cdrom wordos.iso -m 256M
