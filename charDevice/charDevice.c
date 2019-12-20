#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h> // file_operations structure
#include <linux/cdev.h> // development for char driver
#include <linux/semaphore.h> // allows synchronization
#include <asm/uaccess.h>
MODULE_LICENSE("GPL");
    
struct fakeDevice
{
    char data[100];
    struct semaphore sem;
} virtualDevice;

struct cdev *mycdev;
int majorNumber;
int ret;
dev_t devNum;

#define DEVICE_NAME "myDevice"

int deviceOpen(struct inode * inode, struct file * fptr)
{
    // Allow only one process to open this device by using a semaphore as a mutex lock
    if(down_interruptible(&virtualDevice.sem) != 0) {
        printk(KERN_ALERT "charDevice: could not lock device during open\n");
        return -1;
    }

    printk(KERN_INFO "charDevice: opened device\n");
    return 0;
}

// Returns number of bytes read
ssize_t deviceRead(struct file * fptr, char * bufStoreData, size_t bufCount, loff_t * currOffset)
{
    // Send data from kernel space to user
    printk(KERN_INFO "charDevice: reading from device\n");
    // copy_to_user(destination, source, size to transfer)
    ret = raw_copy_to_user(bufStoreData, virtualDevice.data, bufCount);
    return ret;
}

// Returns number of bytes written
ssize_t deviceWrite(struct file * fptr, const char * bufSourceData, size_t bufCount, loff_t * currOffset)
{
    // Send data from user to kernel space
    printk(KERN_INFO "charDevice: writing to device\n");
    // copy_from_user(destination, source, size to transfer)
    ret = raw_copy_from_user(virtualDevice.data, bufSourceData, bufCount);
    return ret;
}

int deviceClose(struct inode * inode, struct file * fptr)
{
    // Calling up is the opposite of down as in deviceOpen(), this releases the mutex, which allows other processes to use the device now
    up(&virtualDevice.sem);
    printk(KERN_INFO "charDevice: closed device\n");
    return 0;
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = deviceOpen,
    .release = deviceClose,
    .write = deviceWrite,
    .read = deviceRead
};

__initdata int greeting = 0; 

__init int mod_init(void)
{
    printk(KERN_INFO "charDevice: %d\n", greeting);
    ret = alloc_chrdev_region(&devNum, 0, 1, DEVICE_NAME); // Allocate major number
    if(ret < 0)
    {
        printk(KERN_ALERT "charDevice: failed to allocate major number\n");
        return ret;
    }
    majorNumber = MAJOR(devNum); // Gets major number
    printk(KERN_INFO "charDevice: major number is %d\n", majorNumber);
    printk(KERN_INFO "charDevice: use \"mknod /dev/%s c %d 0\" for device file\n", DEVICE_NAME, majorNumber);

    // Creates device structure
    mycdev = cdev_alloc();
    mycdev->ops = &fops;
    mycdev->owner = THIS_MODULE;

    ret = cdev_add(mycdev, devNum, 1);
    if(ret < 0) 
    {
        printk(KERN_ALERT "charDevice: unable to add cdev to kernel\n");
        return ret;
    }
    
    // Initialize the semaphore
    sema_init(&virtualDevice.sem, 1); // Initial value of 1
    return 0;
}

static void mod_exit(void)
{
    cdev_del(mycdev);
    
    unregister_chrdev_region(devNum, 1);
    printk(KERN_ALERT "charDevice: unloaded module\n");
    return;
}

module_init(mod_init);
module_exit(mod_exit);
