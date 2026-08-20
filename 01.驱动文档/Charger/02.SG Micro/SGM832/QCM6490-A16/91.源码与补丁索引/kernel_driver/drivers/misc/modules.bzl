load(":drivers/misc/isl97900_led/modules.bzl", register_isl97900_led = "register_modules")
load(":drivers/misc/lkdtm/modules.bzl", register_lkdtm = "register_modules")

def register_modules(registry):
    register_lkdtm(registry)
    register_isl97900_led(registry)

    registry.register(
        name = "drivers/misc/qseecom_proxy",
        out = "qseecom_proxy.ko",
        config = "CONFIG_QSEECOM_PROXY",
        srcs = [
            # do not sort
            "drivers/misc/qseecom_proxy.c",
        ],
    )

    registry.register(
        name = "drivers/misc/bootmarker_proxy",
        out = "bootmarker_proxy.ko",
        config = "CONFIG_BOOTMARKER_PROXY",
        srcs = [
            # do not sort
            "drivers/misc/bootmarker_proxy.c",
        ],
    )

    registry.register(
        name = "drivers/misc/fastrpc",
        out = "frpc-adsprpc.ko",
        config = "CONFIG_QTI_FASTRPC",
        srcs = [
            # do not sort
            "drivers/misc/fastrpc.c",
        ],
        deps = [
            # do not sort
            "drivers/firmware/qcom/qcom-scm",
        ],
    )

    registry.register(
        name = "drivers/misc/iopartition",
        out = "iopartition.ko",
        config = "CONFIG_ODMCONFIG",
        srcs = [
            # do not sort
            "drivers/misc/iopartition.c",
            "drivers/misc/project_feature.h",
        ],
    )

    registry.register(
        name = "drivers/misc/gpio-userspace",
        out = "gpio-userspace.ko",
        config = "CONFIG_GPIO_USERSPACE",
        srcs = [
            # do not sort
            "drivers/misc/gpio-userspace.c",
        ],
    )

    registry.register(
        name = "drivers/misc/usb_uart_sw",
        out = "usb_uart_sw.ko",
        config = "CONFIG_USB_UART_SW",
        srcs = [
            # do not sort
            "drivers/misc/usb_uart_sw.c",
        ],
    )

    registry.register(
        name = "drivers/misc/usb-speed-show",
        out = "usb-speed-show.ko",
        config = "CONFIG_USB_SPEED_SHOW",
        srcs = [
            # do not sort
            "drivers/misc/usb-speed-show.c",
        ],
    )

    registry.register(
        name = "drivers/misc/hwproject",
        out = "hwproject.ko",
        config = "CONFIG_MEIG_HWPROJ",
        srcs = [
            # do not sort
            "drivers/misc/hwproject.c",
        ],
    )

    registry.register(
        name = "drivers/misc/usb-hub-control",
        out = "usb-hub-control.ko",
        config = "CONFIG_HUB_CONTROL",
        srcs = [
            # do not sort
            "drivers/misc/usb-hub-control.c",
        ],
    )

    registry.register(
        name = "drivers/misc/ext-adc-gpio",
        out = "ext-adc-gpio.ko",
        config = "CONFIG_EXT_ADC_GPIO",
        srcs = [
            # do not sort
            "drivers/misc/ext-adc-gpio.c",
        ],
    )

    registry.register(
        name = "drivers/misc/cash_drawer",
        out = "cash_drawer.ko",
        config = "CONFIG_CASH_DRAWER_DRIVER",
        srcs = [
            # do not sort
            "drivers/misc/cash_drawer.c",
        ],
    )

