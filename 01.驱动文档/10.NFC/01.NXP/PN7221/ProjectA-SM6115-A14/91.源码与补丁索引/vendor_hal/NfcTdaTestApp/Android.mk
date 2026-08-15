LOCAL_PATH := $(call my-dir)
################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := NfcTdaTestApp
LOCAL_JAVA_LIBRARIES := com.nxp.nfc

LOCAL_STATIC_JAVA_LIBRARIES += com.google.android.material_material

LOCAL_USES_LIBRARIES := com.nxp.nfc

#LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_SDK_VERSION := system_current

# built explicitly put it in the data partition
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/preinstall/app

include $(BUILD_PACKAGE)
