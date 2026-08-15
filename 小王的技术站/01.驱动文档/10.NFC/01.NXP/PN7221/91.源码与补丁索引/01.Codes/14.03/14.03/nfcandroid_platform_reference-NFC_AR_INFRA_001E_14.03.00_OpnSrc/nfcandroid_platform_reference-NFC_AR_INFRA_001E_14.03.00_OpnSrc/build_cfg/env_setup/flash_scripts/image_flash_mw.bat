adb wait-for-device
adb root
adb wait-for-device
adb remount
adb wait-for-device
adb shell rm -rf /vendor/etc/init/android.hardware.nfc@1.1-service.rc
adb shell rm -rf /vendor/bin/hw/android.hardware.nfc@1.1-service
adb shell rm -rf /vendor/etc/init/android.hardware.secure_element@1.0-service.rc
adb shell rm -rf /vendor/bin/hw/android.hardware.secure_element@1.0-service
adb shell rm -rf /vendor/etc/init/android.hardware.wired_se@1.0-service.rc
adb shell rm -rf /vendor/bin/hw/android.hardware.wired_se@1.0-service
adb shell rm -rf /system/app/SecureElement
adb shell rm -rf /system/app/SmartcardService
adb shell rm -rf /system/app/LotsOfApps
adb shell rm -rf /system/app/FakeOemFeatures
adb shell rm -rf /system/app/TransitionTests
adb shell rm -rf /system/app/ExamplePlugin
adb shell rm -rf /system/priv-app/TeleService/      
adb shell rm -rf /system/priv-app/Telecom/          
adb shell rm -rf /system/priv-app/TelephonyProvider/
adb shell rm -rf /system_ext/priv-app/CarSystemUI/

adb shell rm -rf /system/vendor/etc/libese-nxp.conf 

adb push libnfc_nci_jni.so /system/lib64/libnfc_nci_jni.so
adb push libnfc-nci.so /system/lib64/libnfc-nci.so
adb push vendor.nxp.nxpnfc@1.0.so /system/lib64/vendor.nxp.nxpnfc@1.0.so
adb push libnfc-nci.conf /etc/
adb push libnfc-nxp.conf /vendor/etc/
adb push libnfc-nxp-eeprom.conf /vendor/etc/
adb shell setprop persist.logd.size 16M
adb reboot

