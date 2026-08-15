adb remount

echo === start dvfs ===
::adb shell "./data/vcorefs_main.sh&"

echo === s0 only ===
adb shell "echo reg_spm_apsrc_req 0 > /sys/power/spm/suspend_ctrl"
adb shell "echo reg_spm_f26m_req 1 > /sys/power/spm/suspend_ctrl"
adb shell "echo reg_spm_infra_req 1 > /sys/power/spm/suspend_ctrl"
adb shell "echo reg_spm_ddren_req 1 > /sys/power/spm/suspend_ctrl"
adb shell "echo reg_spm_vrf18_req 1 > /sys/power/spm/suspend_ctrl"

echo === timer stress 0x10000 (32k=1sec) ===
adb shell "echo timer_val_cust 10000 > /sys/power/spm/suspend_ctrl"


adb shell sync

pause