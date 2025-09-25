#!/bin/sh
echo --------------------------- CLEANED UP SYSTEM ---------------------------------
make
./iso.sh
./clean.sh
echo ---------------------------- SYSTEM STARTING -----------------------------------
qemu-system-x86_64 -M type=pc-q35-9.2 -cpu Haswell-v3 -d int -no-reboot -no-shutdown -serial stdio -cdrom wordos.iso -m 256M
