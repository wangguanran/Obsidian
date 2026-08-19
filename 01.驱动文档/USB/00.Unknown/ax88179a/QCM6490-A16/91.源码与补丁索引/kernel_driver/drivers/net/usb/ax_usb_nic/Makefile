# SPDX-License-Identifier: GPL-2.0
#
# ASIX ax_usb_nic USB Ethernet driver (ported from LA.UM.9.14.1)
#

obj-$(CONFIG_USB_NET_AX_USB_NIC) += ax_usb_nic.o
ax_usb_nic-objs := ax_main.o ax88179_178a.o ax88179a_772d.o

# Match A14 lahaina feature flags (PTP off by default)
ccflags-y += -DENABLE_IOCTL_DEBUG
ccflags-y += -DENABLE_INT_POLLING
ccflags-y += -DENABLE_AX88279
