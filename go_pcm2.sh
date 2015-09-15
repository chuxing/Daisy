qemu-system-x86_64 -hda ~/rootfs.img -kernel ~/git/Daisy/linux-3.18/arch/x86/boot/bzImage -append "root=/dev/sda rdinit=sbin/init noapic" -m 1024

