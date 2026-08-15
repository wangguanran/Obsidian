:loop

for %%x in (1,7,8,14,15) do (
adb wait-for-device && adb root

:: sleep 55 sec
ping 1.1.1.1 -n 80 -w 1000 > nul

:: sleep 3 sec
ping 1.1.1.1 -n 3 -w 1000 > nul
adb wait-for-device && adb shell "echo 390000 >  /proc/gpufreq/gpufreq_opp_freq"
adb wait-for-device && adb shell "echo 0 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_ddr_opp"
adb wait-for-device && adb shell "echo 0 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_vcore_opp"
adb wait-for-device && adb shell "echo 1 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_enable"
adb wait-for-device && adb shell "echo 15 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_ddr_opp"
adb wait-for-device && adb shell "echo 15 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_vcore_opp"
adb wait-for-device && adb shell "echo %%x > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_force_vcore_dvfs_opp"
adb wait-for-device && adb shell "cat /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_dump | grep -e Vcore -e khz"

:: sleep 1 sec
ping 1.1.1.1 -n 1 -w 1000 > nul
adb wait-for-device && adb shell "reboot"
)

goto loop
pause

