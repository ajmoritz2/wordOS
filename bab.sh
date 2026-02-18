#!/bin/sh
echo --------------------------- CLEANED UP SYSTEM ---------------------------------
make
./iso.sh
./clean.sh
echo ---------------------------- SYSTEM STARTING -----------------------------------
#qemu-system-i386 -no-reboot -no-shutdown -serial file:words.log -cdrom wordos.iso -m 256M
#qemu-system-i386 -d int -no-reboot -no-shutdown -serial stdio -cdrom wordos.iso -m 256M
qemu-system-x86_64 -drive file=wordos.iso,media=cdrom -M type=pc-q35-9.2 -cpu Haswell-v3 \
   	-no-reboot -no-shutdown -serial stdio -m 256M\
	-drive file=testfs.fat32,if=none,format=raw,id=ahc\
	-device ahci,id=ahci\
	-device ide-hd,drive=ahc,bus=ahci.0\
	-boot d


# NOTE: To add more cores, use -smp [num_cores]
