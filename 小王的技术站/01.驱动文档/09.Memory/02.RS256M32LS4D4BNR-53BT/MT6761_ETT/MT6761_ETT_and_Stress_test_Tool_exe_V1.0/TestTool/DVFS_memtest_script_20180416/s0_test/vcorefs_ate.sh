#!/system/bin/sh

#####################################
# Customizable
#####################################

#### Enter Low Power Feature
FEATURE_SODI=0
FEATURE_DPIDLE=0
FEATURE_SUSPEND=0

#### 0=all on. 65535=all off.
KR_LOG_MASK=0

#### wait latency for each DVFS finish (0.1=100ms)
T_DVFS_INTERVAL=5

T_ENTER_SODI=5
T_ENTER_DPIDLE=6
T_ENTER_SUSPEND=6
T_END_AROUND=2

LOOP_DVFS_COUNTER="0 1 2"




#####################################
# Dont change below setting
#####################################

run_chip=$(getprop ro.board.platform)
opp_num=$(cat /sys/power/vcorefs/opp_num)

#### keep Screen no off (forever)
setprop persist.keep.awake true

#### 2s wakeup in suspend
echo timer_val_cust 10000 > /sys/power/spm/suspend_ctrl

echo kr_log_mask $KR_LOG_MASK > /sys/power/vcorefs/vcore_debug


#####################################
# Change each opp leve for different low power scenario
#####################################

while [ 1 ]
do

	if [ "$FEATURE_SODI" == "1" ]; then
		echo "<<<<<<< VCOREFS_ATE_STRESS: SODI >>>>>>>" > /dev/kmsg
		echo "<<<<<<< VCOREFS_ATE_STRESS: SODI >>>>>>>"
		echo VCORE_ATE > /sys/power/wake_lock
	
		sleep $T_ENTER_SODI
	
		for dvfs_counter in $LOOP_DVFS_COUNTER; do
			opp=0
			while [ $opp -lt $opp_num ]
			do
				sleep $T_DVFS_INTERVAL
				echo "VCOREFS_ATE_STRESS: run opp $opp" > /dev/kmsg
				echo "VCOREFS_ATE_STRESS: run opp $opp"
				echo KIR_SYSFS $opp > /sys/power/vcorefs/vcore_debug
				true $(( opp++ ))
			done
		done
	
		echo VCORE_ATE > /sys/power/wake_unlock
	fi

sleep 5

	if [ "$FEATURE_DPIDLE" == "1" ]; then
		echo "<<<<<<< VCOREFS_ATE_STRESS: DP_IDLE >>>>>>>" > /dev/kmsg
		echo "<<<<<<< VCOREFS_ATE_STRESS: DP_IDLE >>>>>>>"
		
		echo VCORE_ATE > /sys/power/wake_lock
		#### Power key press
		input keyevent 26
	
		sleep $T_ENTER_DPIDLE
	
		for dvfs_counter in $LOOP_DVFS_COUNTER; do
			opp=0
			while [ $opp -lt $opp_num ]
			do
				sleep $T_DVFS_INTERVAL
				echo "VCOREFS_ATE_STRESS: run opp $opp" > /dev/kmsg
				echo "VCOREFS_ATE_STRESS: run opp $opp"
				echo KIR_SYSFS $opp > /sys/power/vcorefs/vcore_debug
				true $(( opp++ ))
			done
		done
	
		#### Power key press
		input keyevent 26
		echo VCORE_ATE > /sys/power/wake_unlock
	fi

sleep 5

	if [ "$FEATURE_SUSPEND" == "1" ]; then
		echo "<<<<<<< VCOREFS_ATE_STRESS: SUSPEND >>>>>>>" > /dev/kmsg
		echo "<<<<<<< VCOREFS_ATE_STRESS: SUSPEND >>>>>>>"

		echo VCORE_ATE > /sys/power/wake_unlock
		#### Power key press
		input keyevent 26
	
		sleep $T_ENTER_SUSPEND
	
		for dvfs_counter in $LOOP_DVFS_COUNTER; do
			opp=0
			while [ $opp -lt $opp_num ]
			do
				sleep $T_DVFS_INTERVAL
				echo "VCOREFS_ATE_STRESS: run opp $opp" > /dev/kmsg
				echo "VCOREFS_ATE_STRESS: run opp $opp"
				echo KIR_SYSFS $opp > /sys/power/vcorefs/vcore_debug
				true $(( opp++ ))
			done
		done
	
		sleep $T_END_AROUND
	
		#### Power key press
		input keyevent 26
	fi

	cat /sys/kernel/debug/cpuidle/idle_state
done
