@echo off
adb wait-for-device
adb root
adb wait-for-device
adb remount
adb wait-for-device
adb shell rm -rf /data/app/NxpDTA/
adb shell setenforce 0
adb shell mkdir /system/app/NxpDTA/
adb push libosal.so /system/lib64/
adb push libdta.so /system/lib64/
adb push libdta_jni.so /system/lib64/
adb push libmwif.so /system/lib64/
::adb push libnfc-nci.so /system/lib64/
adb push NxpDTA.apk /system/app/NxpDTA/
adb shell sync
adb reboot
REM pause
