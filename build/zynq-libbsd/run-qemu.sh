qemu-system-arm -no-reboot -serial null -serial mon:stdio -net nic,model=cadence_gem,macaddr=0e:b0:ba:5e:ba:11 -net user,hostfwd=tcp:127.0.0.1:2000-:23 -nographic -M xilinx-zynq-a9 -m 256M -kernel rki.elf 
