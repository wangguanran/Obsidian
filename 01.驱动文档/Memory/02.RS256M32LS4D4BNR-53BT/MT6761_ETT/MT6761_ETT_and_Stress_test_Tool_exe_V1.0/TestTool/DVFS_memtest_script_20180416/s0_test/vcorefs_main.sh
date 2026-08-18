#!/system/bin/sh

#####################################
# Customizable
#####################################

VCORE_DVS_EN=1
DDR_DFS_EN=1

#### need to stress opp level
STRESS_OPP="0 1 2 3"

#### 0=all on. 65535=all off.
KR_LOG_MASK=65535

#### wait latency for each DVFS finish (0.1=100ms, 0.001=1ms)
T_DVFS_INTERVAL=0.01

OPP0_VCORE_UV=900000
OPP1_VCORE_UV=900000
OPP2_VCORE_UV=800000
OPP3_VCORE_UV=800000




#####################################
# Dont change below setting
#####################################

run_chip=$(getprop ro.board.platform)
opp_num=$(cat /sys/power/vcorefs/opp_num)

#echo test > /sys/power/wake_lock

#### keep Screen no off (forever)
#setprop persist.keep.awake true 

for i in 0 1; do
	echo KIR_SYSFS 0 > /sys/power/vcorefs/vcore_debug
	echo 0 $OPP0_VCORE_UV > /proc/eem/EEM_DET_SOC/eem_vcore_volt
	echo 1 $OPP1_VCORE_UV > /proc/eem/EEM_DET_SOC/eem_vcore_volt
	echo 2 $OPP2_VCORE_UV > /proc/eem/EEM_DET_SOC/eem_vcore_volt
	echo 3 $OPP3_VCORE_UV > /proc/eem/EEM_DET_SOC/eem_vcore_volt
	echo KIR_SYSFS -1 > /sys/power/vcorefs/vcore_debug
done

echo vcore_dvs $VCORE_DVS_EN > /sys/power/vcorefs/vcore_debug
echo ddr_dfs $DDR_DFS_EN > /sys/power/vcorefs/vcore_debug

echo kr_log_mask $KR_LOG_MASK > /sys/power/vcorefs/vcore_debug




#####################################
# Change each opp leve from SPEC
#####################################

while [ 1 ]
do

		for opp in $STRESS_OPP; do
			sleep $T_DVFS_INTERVAL
			# echo "VCOREFS_ATE_STRESS: run opp $opp" > /dev/kmsg
			echo "VCOREFS_ATE_STRESS: run opp $opp"
			echo KIR_SYSFS $opp > /sys/power/vcorefs/vcore_debug
		done

done
