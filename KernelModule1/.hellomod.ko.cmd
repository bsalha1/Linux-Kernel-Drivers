cmd_/root/Documents/kernel/KernelModule1/hellomod.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000 -T /usr/src/linux-headers-5.2.0-kali3-common/scripts/module-common.lds  --build-id  -o /root/Documents/kernel/KernelModule1/hellomod.ko /root/Documents/kernel/KernelModule1/hellomod.o /root/Documents/kernel/KernelModule1/hellomod.mod.o ;  true
