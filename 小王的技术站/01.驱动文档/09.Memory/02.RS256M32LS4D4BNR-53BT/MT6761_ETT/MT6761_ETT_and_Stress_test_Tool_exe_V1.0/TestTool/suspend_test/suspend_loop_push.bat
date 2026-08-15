adb wait-for-device
adb root
adb wait-for-device

adb push busybox /cache/
adb shell chmod 755 /cache/busybox

adb push suspend_loop.sh /data/
adb shell "chmod +rwx /data/suspend_loop.sh"

adb shell "setprop log.tag S"
adb shell "echo 1 > /proc/mtprintk"
adb shell "echo 8 8 8 8 > /proc/sys/kernel/printk"

pause