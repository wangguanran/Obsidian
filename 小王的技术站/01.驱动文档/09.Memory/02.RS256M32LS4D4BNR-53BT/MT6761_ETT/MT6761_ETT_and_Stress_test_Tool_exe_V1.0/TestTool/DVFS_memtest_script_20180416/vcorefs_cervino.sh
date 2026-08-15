#!/bin/bash

###disable wdk
echo 0 20 30 0 0 > /proc/wdk

setprop persist.keep.awake true

#echo soidle 0 > /d/cpuidle/soidle_state
#echo soidle3 0 > /d/cpuidle/soidle3_state
#echo dpidle 0 > /d/cpuidle/dpidle_state
#echo test > /sys/power/wake_lock
#cat /d/cpuidle/idle_state

### enable QoS (for Cervino)
echo 0 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_ddr_opp
echo 0 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_vcore_opp

echo 1 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_enable

echo 15 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_ddr_opp
echo 15 > /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_vcore_opp

#### disable UART log
## aee -k 2

#### enable 0~-2% SSC
echo '1004 0 5 0 9 0 0 2' > /proc/freqhopping/freqhopping_debug

#### disable multi core
#echo 0 > /sys/devices/system/cpu/cpu1/online
#echo 0 > /sys/devices/system/cpu/cpu2/online
#echo 0 > /sys/devices/system/cpu/cpu3/online
#echo 0 > /sys/devices/system/cpu/cpu4/online
#echo 0 > /sys/devices/system/cpu/cpu5/online
#echo 0 > /sys/devices/system/cpu/cpu6/online
#echo 0 > /sys/devices/system/cpu/cpu7/online

echo " ***** STARTING DVFS STRESS ***** "

while [ 1 ]
do
	/data/vcorefs_cervino -i 0.001 -o 1 -o 7 -o 8 -o 14 -o 15 -m 16 /sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_force_vcore_dvfs_opp
done