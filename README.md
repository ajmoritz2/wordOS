This is a passion project I am working on currently. I have almost no experience being this low level
down and I also have limited C experience. This is pretty much my project to learn C and get more into
low level programming. The code is super unorganized and I dont have a single clue what I'm doing.
If your code is plagerized, please know I try to understand it before anything, I just liked the structure a lot.


Any help that is given will be appreciated!


To compile you just run the bab.sh command in bash.
This will also start qemu with the correct options.
I believe the ISO works as well, and the qemu command will work with that too.

Current implementations: 
AHCI -> Untested on hardward (I do not own a machine with AHCI :^) )
 ACPI -> No AML but I can parse the tables 
PCIe -> Horrible code, but it "works". Ill get around to fixing it up and supporting hotplugging later on I think 
Scheduler -> round robin. Ill do something better when I'm more read up on theory. 
Single cored -> Multicore soon ™️ 
Janky Terminal -> Wrote my own, janky as hell idc it works. I'll rewrite it to be better sometime

GDT [OK!]

Working on:
 FAT32 implementation for a fs 
VirtFS

Plan to work on: 
syscalls Better PCIe NVMe EXT4 if I feel good Multicore 
USB support 
Optimize rendering (not redraw entire screen everytime) 

Better theory :^)

