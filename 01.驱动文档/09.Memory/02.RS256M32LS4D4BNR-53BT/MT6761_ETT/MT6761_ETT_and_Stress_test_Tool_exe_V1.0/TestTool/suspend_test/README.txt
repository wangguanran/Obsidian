// run script by manually
1. suspend_loop_push.bat
2. adb shell
3. sh /data/suspend_loop.sh &
4. remove USB from PC to charger
5. use the following command to check status
	a. cat /sys/kernel/debug/cpuidle/spm/spm_last_debug_flag
		golden: 0x1910ff
	b. cat /sys/kernel/debug/cpuidle/spm/spm_sleep_count
		golden: increase