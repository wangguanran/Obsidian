# SPDX-License-Identifier: GPL-2.0

ccflags-y += -I$(src)

# Subdirectories
obj-y += lkdtm/

obj-$(CONFIG_QSEECOM_PROXY) += qseecom_proxy.o
obj-$(CONFIG_ODMCONFIG) += iopartition.o
obj-$(CONFIG_GPIO_USERSPACE) += gpio-userspace.o
obj-$(CONFIG_USB_UART_SW) += usb_uart_sw.o
obj-$(CONFIG_USB_SPEED_SHOW) += usb-speed-show.o
obj-$(CONFIG_MEIG_HWPROJ) += hwproject.o


obj-$(CONFIG_HUB_CONTROL) += usb-hub-control.o
obj-$(CONFIG_EXT_ADC_GPIO) += ext-adc-gpio.o
obj-$(CONFIG_CASH_DRAWER_DRIVER) += cash_drawer.o
