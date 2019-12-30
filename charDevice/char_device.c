#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h> // file_operations structure
#include <linux/cdev.h> // development for char driver
#include <linux/semaphore.h> // allows synchronization
#include <linux/slab.h> // kernel memory management

#include <asm/uaccess.h>

#include "ioctl.h"

#define DATA_SIZE 128
#define DEVICE_NAME "char_device"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bilal Salha");

struct virtual_device
{
    char data[DATA_SIZE];
    struct semaphore sem;
} fake_device;

struct cdev *mycdev;
struct class *clazz;
struct device *device;
int major_number;
int minor_number;
dev_t dev_num;



/*
 * .open file operation callback
 * return 0 for success, -1 for fail
 */
int device_open(struct inode * inode, struct file * fptr)
{
    // Try lock
    if(down_interruptible(&fake_device.sem) != 0)
    {
        printk(KERN_ALERT "char_device: could not lock device during open\n");
        return -1;
    }
    
    #ifdef DEBUG
    printk(KERN_INFO "char_device: opened device\n");
    #endif

    return 0;
}



/*
 * .read file operation callback
 * returns number of bytes read
 */
ssize_t device_read(struct file * fptr, char * buf_device, size_t buf_size, loff_t * curr_offset)
{
    #ifdef DEBUG
    printk(KERN_INFO "char_device: reading from device\n");
    #endif

    if(raw_copy_to_user(buf_device, fake_device.data, buf_size) < 0)
    {
        printk(KERN_INFO "char_device: failed to read from device\n");
        return -1;
    }

    return 0;
}



/*
 * .write file operation callback
 * returns number of bytes written
 */
ssize_t device_write(struct file * fptr, const char * buf_user, size_t buf_size, loff_t * curr_offset)
{
    #ifdef DEBUG
    printk(KERN_INFO "char_device: writing to device\n");
    #endif

    if(raw_copy_from_user(fake_device.data, buf_user, buf_size))
    {
        printk(KERN_INFO "char_device: failed to write to device\n");
        return -1;
    }

    return 0;
}



/*
 * .unlocked_ioctl file operation callback
 */
long device_ioctl(struct file * fptr, unsigned int cmd, unsigned long args)
{
    #ifdef DEBUG
    printk(KERN_INFO "char_device: handling ioctl\n");
    #endif

    switch(cmd)
    {
        case HANDSHAKE:
            return HANDSHAKE;
       
        case GET_MAJOR_NUMBER:
            return major_number;

        case GET_MINOR_NUMBER:
            return minor_number;

        case GET_REMAINING_SIZE:
             return (DATA_SIZE - strlen(fake_device.data));

        default:
             return -1;
    }
}



/*
 * .release file operation callback
 */
int device_close(struct inode * inode, struct file * fptr)
{
    up(&fake_device.sem);

    #ifdef DEBUG
    printk(KERN_INFO "char_device: closed device\n");
    #endif

    return 0;
}



/*
 * Define file operation callbacks
 */
struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_close,
    .write = device_write,
    .read = device_read,
    .unlocked_ioctl = device_ioctl
};



//__initdata int greeting = 0; 

/*
 * Module entry point
 */
__init int mod_init(void)
{
    //printk(KERN_INFO "char_device: %d\n", greeting);
    
    // Define dev_num
    if(alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
    {
        printk(KERN_ALERT "char_device: failed to allocate major number\n");
        return -1;
    }

    major_number = MAJOR(dev_num);
    minor_number = MINOR(dev_num);

    #ifdef DEBUG
    printk(KERN_INFO "char_device: device numbers are %d/%d\n", major_number, minor_number);
    #endif


    // Create class (add device to sys/class folder)
    clazz = class_create(THIS_MODULE, "char_device_class");
    if(clazz == NULL)
    {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_INFO "char_device: failed to create class\n");
        return -1;
    }

  
    // Define character device structure fields
    mycdev = cdev_alloc();
    mycdev->ops = &fops;
    mycdev->owner = THIS_MODULE;
    cdev_init(mycdev, &fops);
    if(cdev_add(mycdev, dev_num, 1) < 0) 
    {
        printk(KERN_ALERT "char_device: unable to add cdev to kernel\n");
        return -1;
    }


    // Add device to /dev folder
    if(device_create(clazz, NULL, dev_num, NULL, "char_device") == NULL)
    {
        printk(KERN_ALERT "char_device: unable to make node for device\n");
        return -1;
    }
    

    // Initialize the semaphore with value of 1
    sema_init(&fake_device.sem, 1);

    printk(KERN_INFO "char_device: virtual character device driver initialized\n");
    return 0;
}



/*
 * Module exit point
 */
static void mod_exit(void)
{
    device_destroy(clazz, dev_num);
    class_destroy(clazz);
    cdev_del(mycdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_ALERT "char_device: virtual character device driver removed\n");
    return;
}

module_init(mod_init);
module_exit(mod_exit);
