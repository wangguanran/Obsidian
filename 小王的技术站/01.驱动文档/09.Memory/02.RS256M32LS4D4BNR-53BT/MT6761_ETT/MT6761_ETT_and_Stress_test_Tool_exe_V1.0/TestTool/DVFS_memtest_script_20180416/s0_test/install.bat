adb remount

adb shell rm -f /data/vcorefs_*.sh

adb push vcorefs_main.sh /data/vcorefs_main.sh
adb shell chmod 777 /data/vcorefs_main.sh

adb push vcorefs_ate.sh /data/vcorefs_ate.sh
adb shell chmod 777 /data/vcorefs_ate.sh

adb shell sync
pause
