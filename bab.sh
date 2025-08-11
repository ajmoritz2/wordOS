#!/bin/sh
echo --------------------------- CLEANED UP SYSTEM ---------------------------------
make
./iso.sh
./clean.sh
echo ---------------------------- SYSTEM STARTING -----------------------------------
qemu-system-i386 -no-reboot -no-shutdown -serial stdio -cdrom wordos.iso -m 256M
