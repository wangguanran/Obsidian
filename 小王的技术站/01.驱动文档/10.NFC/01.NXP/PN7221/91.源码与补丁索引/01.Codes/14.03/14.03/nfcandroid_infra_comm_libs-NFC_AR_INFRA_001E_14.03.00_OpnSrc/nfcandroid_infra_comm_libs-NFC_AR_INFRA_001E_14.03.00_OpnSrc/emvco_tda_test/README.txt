###################################################################################################
#                             EMVCo TDA test tool                                      #
###################################################################################################
•  Folder Name: emvco_tda_test
•  Contents:
   o    src/*.cpp                         -> Source file
   o    Android.bp                        -> Make File
   o    README.txt                        -> How to build and use executable.

•  How to build the executable:
   o STEP1: Keep emvco_tda_test folder under hardware/nxp/emvco/
   o STEP2: Navigate to the emvco_tda_test folder and run mm -j32 command.
   o STEP3: It will build SmcuSwitchV1_0 under following folder
             out/target/product/db845c/data/nativetest64/EMVCoAidlHalTDATest/EMVCoAidlHalTDATest

•  How to do emvco_tda_test:
   o STEP1: Push this file to system/etc/ folder
             i.e. adb push EMVCoAidlHalTDATest system/etc/
   o STEP2: Give executable permission. Ex: adb shell chmod 0777 /system/etc/EMVCoAidlHalTDATest
   o STEP3: adb shell and then navigate to system/etc folder
   o STEP4: Run "./EMVCoAidlHalTDATest type AB CT" from command line.
            type AB - indicates to poll for AB RF technology
            CT - indicates to do loopback on CT slot. we can also specify SAM1 or SAM2 slots instead of CT

###################################################################################################
