#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x02
#define BULK_EP_IN 0x81
#define MAX_PKT_SIZE 512

static struct mutex lock;
static struct usb_device * device;
static struct usb_class_driver class;
static unsigned char bulk_buf[MAX_PKT_SIZE];
static __le16 maxPacketSize = 0;



/*
 * Method ran when pen is opened
 */
static int pen_open(struct inode * i, struct file * f)
{
    printk(KERN_INFO "usbBD: the user has opened the pen drive\n");
    return 0;
}



/*
 * Method ran when pen is closed
 */
static int pen_close(struct inode * i, struct file * f)
{
    printk(KERN_INFO "usbBD: the user has closed the pen drive\n");
    return 0;
}



/*
 * Allows user to read from the device
 */
static ssize_t pen_read(struct file *f, char __user * buf, size_t count, loff_t * off)
{
    int ret;
    int read_count;
    
    ret = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN), bulk_buf, MAX_PKT_SIZE, &read_count, 5000);
    if(ret)
    {
        printk(KERN_ERR "usbBD: reading from USB bulk endpoint failed\n");
        return ret;
    }

    if(raw_copy_to_user(buf, bulk_buf, MIN(count, read_count)))
    {
        return -EFAULT;
    }

    return MIN(count, read_count);
}



/*
 * Writes data to the user from device
 */
static ssize_t pen_write(struct file * f, const char __user * buf, size_t count, loff_t * off)
{
    int ret;
    int wrote_count;

    if(raw_copy_from_user(bulk_buf, buf, MIN(count, MAX_PKT_SIZE)))
    {
        return -EFAULT;
    }

    ret = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT), bulk_buf, MIN(count, MAX_PKT_SIZE), &wrote_count, 5000);
    if(ret)
    {
        printk(KERN_ERR "usbBD: writing to USB bulk endpoint failed\n");
    }

    return wrote_count;
}



/*
 * The definitions for the file operations on this driver
 */
static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .open = pen_open,
    .release = pen_close,
    .read = pen_read,
    .write = pen_write
};



/*
 * Method ran when the pen is plugged in. If it fails to run, end uas and usb-storage modules
 */
static int pen_probe(struct usb_interface * interface, const struct usb_device_id * id)
{
    int ret;
    int i;
    struct usb_host_interface * iface = interface->cur_altsetting;
    struct usb_endpoint_descriptor endpoint_desc;
    
    printk(KERN_INFO "usbBD: Pen (%04X:%04X) plugged in\n", id->idVendor, id->idProduct);

    // Iterate through endpoints trying to find bulk endpoints
    for(i = 0; i < iface->desc.bNumEndpoints; i++)
    {
        endpoint_desc = iface->endpoint[i].desc;
        if(endpoint_desc.bmAttributes == USB_ENDPOINT_XFER_BULK) // If bulk endpoint
        {
            printk(KERN_INFO "usbBD: Bulk endpoint at address 0x%02X", endpoint_desc.bEndpointAddress);
            maxPacketSize = endpoint_desc.wMaxPacketSize;
        }
    }

    // If max packet size isn't assigned, there were no bulk endpoints
    if(!maxPacketSize)
    {
        printk(KERN_INFO "usbBD: No bulk endpoints found\n");
        return -1;
    }

    device = interface_to_usbdev(interface);
    
    class.name = "usb/pen%d";
    class.fops = &fops;
    
    ret = usb_register_dev(interface, &class);
    if(ret < 0)
    {
        printk(KERN_ERR "usbBD: failed to get a minor for USB\n");
    }
    else
    {
        printk(KERN_INFO "usbBD: minor obtained: %d\n", interface->minor);
    }

    return ret;
}



/*
 * Unregisters the driver from the dev directory
 */
static void pen_disconnect(struct usb_interface * interface)
{
    mutex_lock(&lock);
    usb_deregister_dev(interface, &class);
    mutex_unlock(&lock);

    printk(KERN_INFO "usbBD: pen%d disconnected\n", interface->minor);
}



/*
 * List of usb devices to be controlled by the driver
 */
static struct usb_device_id pen_table[] =
{
    { USB_DEVICE(0x0781, 0x5530)},
    {}
};
MODULE_DEVICE_TABLE(usb, pen_table);



/*
 * The usb driver structure
 */
static struct usb_driver pen_driver = 
{
    .name = "usbBD",
    .probe = pen_probe,
    .disconnect = pen_disconnect,
    .id_table = pen_table
};



/*
 * Initializes the driver; registers the usb driver with the kernel
 */
static int __init mod_init(void)
{
    int ret;
    ret = usb_register(&pen_driver);

    if(ret)
    {
        printk(KERN_ERR "usbBD: registration failed, code: %d\n", ret);
    }
    return ret;
}



/*
 * The exitting routine for the driver; deregisters the usb driver with the kernel
 */
static void __exit mod_exit(void)
{
    usb_deregister(&pen_driver);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bilal Salha");
MODULE_DESCRIPTION("Allows talking between USB driver and user");
