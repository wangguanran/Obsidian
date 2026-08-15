#####
##### NXP NFC Device Configuration makefile
######

NXP_NFC_HOST := $(TARGET_PRODUCT)
ifndef TARGET_NXP_NFC_HW
NXP_NFC_HW := pn7220_i2cs
else
NXP_NFC_HW := $(TARGET_NXP_NFC_HW)
endif
NXP_NFC_PLATFORM := pn54x
NXP_VENDOR_DIR := nxp
NXP_I2CM_S := $(TARGET_NXP_I2C_M_S)


ifeq ($(strip $(TARGET_NXP_I2C_M_S)),)
    NXP_I2CM_S := FALSE
endif

# These are the hardware-specific features
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:system/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:system/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
    vendor/$(NXP_VENDOR_DIR)/frameworks/nfc/com.nxp.nfc.xml:system/etc/permissions/com.nxp.nfc.xml

# NFC config files
PRODUCT_COPY_FILES += \
    vendor/$(NXP_VENDOR_DIR)/nfc/hw/$(NXP_NFC_HW)/libnfc-nci.conf:system/etc/libnfc-nci.conf \
    vendor/$(NXP_VENDOR_DIR)/nfc/hw/$(NXP_NFC_HW)/libnfc-nxp.conf:vendor/etc/libnfc-nxp.conf \
    vendor/$(NXP_VENDOR_DIR)/nfc/hw/init.module.nfc.rc:vendor/etc/init/init.module.nfc.rc

ifneq ($(NXP_NFC_HW),pn7160)
PRODUCT_COPY_FILES += \
    vendor/$(NXP_VENDOR_DIR)/nfc/hw/$(NXP_NFC_HW)/libnfc-nxp-eeprom.conf:vendor/etc/libnfc-nxp-eeprom.conf
endif

ifeq ($(NXP_NFC_HW),pn7220_i2cs)
PRODUCT_COPY_FILES += \
    vendor/$(NXP_VENDOR_DIR)/nfc/hw/$(NXP_NFC_HW)/libnfc-nxp-rfExt.conf:vendor/etc/libnfc-nxp-rfExt.conf
endif

ifeq ($(NXP_NFC_HW),pn7221_i2cs)
PRODUCT_COPY_FILES += \
    vendor/$(NXP_VENDOR_DIR)/nfc/hw/$(NXP_NFC_HW)/libnfc-nxp-rfExt.conf:vendor/etc/libnfc-nxp-rfExt.conf
endif

# NFC Init Files
PRODUCT_COPY_FILES += \
     vendor/$(NXP_VENDOR_DIR)/nfc/hw/init.$(NXP_NFC_PLATFORM).nfc.rc:vendor/etc/init/init.$(NXP_NFC_HOST).nfc.rc

# Service framework, Jar and VTS packages required for all products
PRODUCT_PACKAGES += \
    NfcNci \
    Tag \
    VtsAidlHalNfcTargetTest \
    com.nxp.nfc \
    NxpDTA \
    nfc_tda \
    NfcTdaTestApp \
    JrcpProxyPallas


# Add NFC TDA and SMCU for only PN7220 I2CMS product
ifeq ($(NXP_NFC_HW),pn7220_i2cms)
PRODUCT_PACKAGES += \
   SmcuSwitchV2_0
endif


# Add T4T for only 7160
ifeq ($(NXP_NFC_HW),pn7160)
PRODUCT_PACKAGES += \
   T4TDemo \
   SelfTestHalAidlNfc
endif

PRODUCT_PACKAGES += \
    android.hardware.nfc-service.nxp

ifeq ($(ENABLE_TREBLE), true)
PRODUCT_PACKAGES += \
    vendor.nxp.nxpnfc@1.0-impl \
    vendor.nxp.nxpnfc@1.0-service
endif
ifeq ($(strip $(NXP_I2CM_S)),TRUE)
    I2CM_S_ENABLED = 1
else
    I2CM_S_ENABLED = 0
endif
PRODUCT_PROPERTY_OVERRIDES += \
     ro.hardware.nfc_nci=$(NXP_NFC_PLATFORM) \
     persist.vendor.nxp.i2cms.enabled=$(I2CM_S_ENABLED)

BOARD_SEPOLICY_DIRS += \
    vendor/$(NXP_VENDOR_DIR)/nfc/sepolicy \
    vendor/$(NXP_VENDOR_DIR)/nfc/sepolicy/nfc
