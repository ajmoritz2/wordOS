#!/bin/sh
./clean.sh
echo --------------------------- CLEANED UP SYSTEM ---------------------------------
make
./iso.sh
./clean.sh
echo ---------------------------- SYSTEM STARTING -----------------------------------
qemu-system-i386 -serial file:words.log -cdrom wordos.iso 
