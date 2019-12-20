#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>

/* USB ENDPOINTS
 *
 *  Control - transferring control information, device info, resetting device etc.
 *
 *  Interrupt - Up to 8 bytes of data transfer, used for serial ports, keyboard, mouse etc.
 *
 *  Bulk - big data transfer like massive storage devices
 *
 *  Isochronous - big data transfer with badwidth guarantee. Data integrity may not be
 *  guaranteed. Used for transfers of time-sensitive data like audio, video etc.
 *
 *  All endpoints except Control are read/write.
 *
 *  An endpoint is defined using an 8bit number, MSB signifies 0 meaning out and 1 meaning in
 */

// Probe Function (Won't be called if another driver is already handling the device)
static int penProbe(struct usb_interface * interface, const struct usb_device_id * id)
{
    printk(KERN_INFO "PenDriver: PenDrive (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
    return 0; // 0 indicates that we will manage this device    
}

// Disconnect Function
static void penDisconnect(struct usb_interface * interface)
{
    printk(KERN_INFO "PenDriver: Pen drive removed\n");
}

// USB Device ID
static struct usb_device_id penTable[] = 
{
    // Vendor ID, Product ID
    { USB_DEVICE(0x0781, 0x5530) }, // Information is obtained using lsusb at command line
    {} // Terminating entry 
};
MODULE_DEVICE_TABLE(usb, penTable);


// The Driver
static struct usb_driver penDriver =
{
    .name = "Bilal-PenDriver",
    .id_table = penTable, // Used to match this driver with any device attached to USB bus
    .probe = penProbe,
    .disconnect = penDisconnect
};

static int __init mod_init(void)
{
    int ret = -1;
    printk(KERN_INFO "PenDriver: Constructor of driver\n");

    printk(KERN_INFO "PenDriver: Registering driver with kernel\n");
    ret = usb_register(&penDriver);
    printk(KERN_INFO "PenDriver: Registration is complete\n");

    return ret;
}

static void mod_exit(void)
{
    printk(KERN_INFO "PenDriver: Destructor of driver\n");
    usb_deregister(&penDriver);
    printk(KERN_INFO "PenDriver: Unregistration complete\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bilal Salha");
MODULE_DESCRIPTION("This is a USB drive registration driver");

