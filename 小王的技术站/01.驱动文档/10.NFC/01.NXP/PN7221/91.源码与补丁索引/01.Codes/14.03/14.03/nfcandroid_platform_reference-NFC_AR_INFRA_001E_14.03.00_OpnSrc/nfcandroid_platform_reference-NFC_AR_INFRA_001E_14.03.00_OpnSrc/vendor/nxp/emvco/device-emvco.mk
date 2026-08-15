NXP_VENDOR_DIR := nxp
NXP_NFC_HW := $(TARGET_NXP_NFC_HW)
ifeq ($(strip $(TARGET_NXP_NFC_HW)),)
    NXP_NFC_HW := pn7220_i2cs
endif

# Nfc service has dependency with EMVCo JAR
PRODUCT_PACKAGES += \
    com.nxp.emvco

# Enable EMVCo binaries only for PN7220 and PN7221 product varaint
ifeq ($(NXP_IS_EMVCO_SUPPORTED),true)
PRODUCT_PACKAGES += \
    vendor.nxp.emvco-V1-ndk \
    vendor.nxp.emvco-service \
    EMVCoAidlHalComplianceTest \
    EMVCoAidlHalDesfireTest \
    EMVCoAidlHalTransacTest \
    emvco_poller \
    emvco_tda \
    EMVCoAidlHalTDATest
endif

ifeq ($(NXP_NFC_HW),pn7220_i2cs)
PRODUCT_PACKAGES += \
    EMVCoModeSwitchApp \
    vendor.nxp.emvco-V1-ndk \
    vendor.nxp.emvco-service \
    EMVCoAidlHalComplianceTest \
    EMVCoAidlHalDesfireTest \
    EMVCoAidlHalTransacTest \
    emvco_poller \
    emvco_tda \
    EMVCoAidlHalTDATest
else ifeq ($(NXP_NFC_HW),pn7221_i2cs)
PRODUCT_PACKAGES += \
    EMVCoModeSwitchVasApp \
    vendor.nxp.emvco-V1-ndk \
    vendor.nxp.emvco-service \
    EMVCoAidlHalComplianceTest \
    EMVCoAidlHalDesfireTest \
    EMVCoAidlHalTransacTest \
    emvco_poller \
    emvco_tda \
    EMVCoAidlHalTDATest
endif

# Enable EMVCo config and sepolicy only for PN7220 and PN7221 product varaint
ifeq ($(NXP_NFC_HW),pn7220_i2cs)
BOARD_SEPOLICY_DIRS += vendor/$(NXP_VENDOR_DIR)/emvco/sepolicy
PRODUCT_COPY_FILES += \
    vendor/$(NXP_VENDOR_DIR)/emvco/hw/$(NXP_NFC_HW)/libemvco-nxp.conf:vendor/etc/libemvco-nxp.conf
else ifeq ($(NXP_NFC_HW),pn7221_i2cs)
BOARD_SEPOLICY_DIRS += vendor/$(NXP_VENDOR_DIR)/emvco/sepolicy
PRODUCT_COPY_FILES += \
    vendor/$(NXP_VENDOR_DIR)/emvco/hw/$(NXP_NFC_HW)/libemvco-nxp.conf:vendor/etc/libemvco-nxp.conf
endif