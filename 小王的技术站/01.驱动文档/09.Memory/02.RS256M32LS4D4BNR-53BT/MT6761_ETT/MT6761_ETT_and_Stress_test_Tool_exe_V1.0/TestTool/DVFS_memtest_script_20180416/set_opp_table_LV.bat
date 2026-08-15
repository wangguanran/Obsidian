@echo off
setlocal EnableDelayedExpansion
set deviceid=-s 0123456789ABCDEF
set memtester=/data/memtester
set chk_status=/data/memtest_check_status.sh
set test_rank=no
set test_num=6
set /a test_sz=64*1024*1024
set debug_script=0
set logprefix=/data/rshmoo_log_

echo "=== setup LV opp table ==="
adb %deviceid% shell "echo 15 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_force_vcore_dvfs_opp"
adb %deviceid% shell "echo 0 756250 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_set_vcore_uv"
adb %deviceid% shell "echo 1 662500 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_set_vcore_uv"
adb %deviceid% shell "echo 2 662500 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_set_vcore_uv"
adb %deviceid% shell "echo 3 612500 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_set_vcore_uv"
adb %deviceid% shell "echo 0 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_force_vcore_dvfs_opp"


