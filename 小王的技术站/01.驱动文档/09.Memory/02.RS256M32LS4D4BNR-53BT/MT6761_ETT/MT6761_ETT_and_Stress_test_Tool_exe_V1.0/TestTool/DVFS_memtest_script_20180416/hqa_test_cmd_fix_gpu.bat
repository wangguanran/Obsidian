adb shell "echo 390000 >  /proc/gpufreq/gpufreq_opp_freq"
adb shell "cat /proc/gpufreq/gpufreq_var_dump"
pause