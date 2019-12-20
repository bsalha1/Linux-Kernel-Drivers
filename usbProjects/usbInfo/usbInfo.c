#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>

static struct usb_device * device;

static int penProbe(struct usb_interface * interface, const struct usb_device_id * id)
{
    struct usb_host_interface * iface_desc;
    struct usb_endpoint_descriptor * endpoint;
    int i;

    iface_desc = interface->cur_altsetting;
    printk(KERN_INFO "PenInfo: Pen %d (%04X:%04X) now probed\n", iface_desc->desc.bInterfaceNumber, id->idVendor, id->idProduct);
    printk(KERN_INFO "PenInfo: Number of endpoints: %02X\n", iface_desc->desc.bNumEndpoints);
    printk(KERN_INFO "PenInfo: Interface Class: %02X\n", iface_desc->desc.bInterfaceClass);

    for(i = 0; i < iface_desc->desc.bNumEndpoints; i++)
    {
        endpoint = &iface_desc->endpoint[i].desc;
        printk(KERN_INFO "PenInfo: ED[%d]: Endpoint Address: 0x%02X\n", i, endpoint->bEndpointAddress);
        printk(KERN_INFO "PenInfo: ED[%d]: Attributes: 0x%02X\n", i, endpoint->bmAttributes);
        printk(KERN_INFO "PenInfo: ED[%d]: Max Packet Size: 0x%04X (%d)\n", i, endpoint->wMaxPacketSize, endpoint->wMaxPacketSize);
    }

    device = interface_to_usbdev(interface);
    return 0;
}

static void penDisconnect(struct usb_interface * interface)
{
    printk(KERN_INFO "PenInfo: Pen drive removed\n");
}

static struct usb_device_id penTable[] = 
{
    { USB_DEVICE(0x0781, 0x5530) },
    {}
};
MODULE_DEVICE_TABLE(usb, penTable);

static struct usb_driver penDriver = 
{
    .name = "PenInfo",
    .id_table = penTable,
    .probe = penProbe,
    .disconnect = penDisconnect
};

static int __init mod_init(void)
{
    int ret = -1;
    ret = usb_register(&penDriver);
    printk(KERN_INFO "PenInfo: USB driver started\n");
    return ret;
}

static void __exit mod_exit(void)
{
    usb_deregister(&penDriver);
    printk(KERN_INFO "PenInfo: USB driver stopped\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bilal Salha");
MODULE_DESCRIPTION("Gets various information regarding a USB device");
