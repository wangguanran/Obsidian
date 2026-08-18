interval_timer()
{
	echo "timer_val_cust 0x10000" > /sys/power/spm/suspend_ctrl
	echo "test" > /sys/power/wake_lock
	while true; do
		echo "@@@@@@@ device wakeup @@@@@@\n"
		input keyevent 26
		sleep 1
		input keyevent 82
		echo "test" > /sys/power/wake_lock
		sleep 6
		input keyevent 26
		echo "@@@@@@@ sleep @@@@@@@\n"
		echo "test" > /sys/power/wake_unlock
		#/cache/busybox rtcwake -s 10
		sleep 6
	done
}

interval_timer
