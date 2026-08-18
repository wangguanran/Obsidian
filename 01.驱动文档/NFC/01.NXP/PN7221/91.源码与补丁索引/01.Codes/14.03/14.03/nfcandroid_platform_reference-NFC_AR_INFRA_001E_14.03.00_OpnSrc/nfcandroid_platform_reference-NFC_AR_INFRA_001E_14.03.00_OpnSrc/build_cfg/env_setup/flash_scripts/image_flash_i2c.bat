adb reboot bootloader
fastboot flash boot boot_uefi_pn7160_i2c.img
fastboot flash dts dt_pn7160_i2c.img
fastboot flash system system.img
fastboot flash userdata userdata.img
fastboot flash vendor vendor_pn7160_i2c.img
fastboot flash cache cache.img
fastboot reboot
pause
