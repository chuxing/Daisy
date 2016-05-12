qemu-system-x86_64 -hda rootfs.img -kernel linux-3.12.49/arch/x86/boot/bzImage -append "root=/dev/sda rdinit=/bin/sh console=ttyS0" -nographic -m 5000

