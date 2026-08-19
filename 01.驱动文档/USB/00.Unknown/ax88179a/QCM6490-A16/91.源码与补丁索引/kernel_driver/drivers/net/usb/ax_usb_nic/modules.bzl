def register_modules(registry):
    registry.register(
        name = "drivers/net/usb/ax_usb_nic",
        out = "ax_usb_nic.ko",
        config = "CONFIG_USB_NET_AX_USB_NIC",
        srcs = [
            # do not sort
            "drivers/net/usb/ax_usb_nic/ax_main.c",
            "drivers/net/usb/ax_usb_nic/ax88179_178a.c",
            "drivers/net/usb/ax_usb_nic/ax88179a_772d.c",
        ],
        hdrs = [
            "drivers/net/usb/ax_usb_nic/ax_main.h",
            "drivers/net/usb/ax_usb_nic/ax88179_178a.h",
            "drivers/net/usb/ax_usb_nic/ax88179a_772d.h",
            "drivers/net/usb/ax_usb_nic/ax_ioctl.h",
            "drivers/net/usb/ax_usb_nic/ax_ptp.h",
        ],
        copts = [
            "-DENABLE_IOCTL_DEBUG",
            "-DENABLE_INT_POLLING",
            "-DENABLE_AX88279",
        ],
    )
