# Daisy: How to Use

1. Make sure that you have an Linux System Running with at least 6GB memory
2. > git clone git@github.com:DDST-NVM/Daisy.git
3. > cd Daisy/linux-3.12.49/
4. > sudo make mrproper
5. > sudo make localmodconfig (and then press enter for all options)
6. > sudo make -jN(N is the number of your cores)
7. > sudo make install_modules
8. > sudo make install
9. > sudo reboot

10. During your reboot phase, please press `F12` or `SHIFT` to choose the linux kernel version. You should choose linux-3.12.49 for running Daisy System.
11. If there is problem with the window showing during the boot phase, please reboot and edit the grub interface. 
12. If everything goes smoonthly, just return to the Daisy directory.
13. > cd pcmapi
14. > chmod u=rwx create_so.sh
15. > ./create_so.sh 
16. > make
17. > ./ptest
18. You can debug our user-mode codes with gdb.
