#!/bin/sh
echo --------------------------- CLEANED UP SYSTEM ---------------------------------
make
./iso.sh
./clean.sh
echo ---------------------------- SYSTEM STARTING -----------------------------------
#qemu-system-i386 -no-reboot -no-shutdown -serial file:words.log -cdrom wordos.iso -m 256M
#qemu-system-i386 -d int -no-reboot -no-shutdown -serial stdio -cdrom wordos.iso -m 256M
<<<<<<< HEAD
<<<<<<< HEAD
qemu-system-x86_64 -drive file=wordos.iso,media=cdrom -M type=pc-q35-9.2 -cpu Haswell-v3 \
   	-no-reboot -no-shutdown -serial stdio -m 256M\
	-drive file=testfs.fat32,if=none,format=raw,id=ahc\
=======
qemu-system-x86_64 -drive file=wordos.iso,media=cdrom -M type=pc-q35-9.2 -cpu Haswell-v3 \
   	-no-reboot -no-shutdown -serial stdio -m 256M\
	-drive file=testfs.fat32,if=none,id=ahc\
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
	-device ahci,id=ahci\
	-device ide-hd,drive=ahc,bus=ahci.0\
	-boot d


# NOTE: To add more cores, use -smp [num_cores]
<<<<<<< HEAD
=======
qemu-system-x86_64 -M type=pc-q35-9.2 -cpu Haswell-v3 -no-reboot -no-shutdown -serial stdio -cdrom wordos.iso -m 256M
>>>>>>> parent of 8fae1a0 (ahci stuff and acpi stuff. cant read disk yet)
=======
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
