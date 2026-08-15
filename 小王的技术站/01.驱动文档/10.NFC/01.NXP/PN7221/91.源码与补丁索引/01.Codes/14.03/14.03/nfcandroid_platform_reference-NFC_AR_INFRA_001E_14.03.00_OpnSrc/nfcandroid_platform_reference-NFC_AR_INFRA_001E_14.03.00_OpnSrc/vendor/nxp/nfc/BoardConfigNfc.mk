# Copyright 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_STEM := nfc/BoardConfigNfcPartial.mk
#TODO This need to commented once sepolicy updated for emvco hal.
BOARD_KERNEL_CMDLINE += androidboot.selinux=permissive
#TODO: Check and remove BUILD_BROKEN_DUP_RULES usage (artf995547).
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE := vendor/nxp/nfc/framework_compatibility_matrix.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE += vendor/nxp/emvco/emvco_framework_compatibility_matrix.xml
BUILD_BROKEN_DUP_RULES := true

-include vendor/nxp/$(LOCAL_STEM)
