#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>       // Files
#include <linux/blkdev.h>   // Block device
#include <linux/blk-mq.h>
#include <linux/genhd.h>    // Harddrive
#include <linux/hdreg.h>

#include <linux/mm.h> // page_address

#include <linux/errno.h>
#include <linux/err.h>

#include "enums.h"
#include "device_operations.h"

struct gendisk * gd;

struct blk_mq_ops bops =
{
    .queue_rq = queue_rq,
};

struct block_device_operations fops =
{
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .ioctl = device_ioctl,
};

static int device_construct(struct vbdev_t * bdev)
{
    bdev = kzalloc(sizeof(struct vbdev_t), GFP_KERNEL);
    if(bdev == NULL)
    {
        print_alert("unable to allocate %ld bytes for device", sizeof(struct vbdev_t));
        return -ENOMEM;
    }
    bdev->capacity = DATA_SIZE;

    // Allocate Data
    {
        bdev->data = kzalloc(DATA_SIZE, GFP_KERNEL);
        if(bdev->data == NULL)
        {
            print_alert("unable to allocate %ld bytes for device data", sizeof(struct vbdev_t));
            return -ENOMEM;
        }
    }

    // Define Tag_Set
    {
        bdev->tag_set.ops = &bops;
        bdev->tag_set.nr_hw_queues = 1;
        bdev->tag_set.queue_depth = QUEUE_DEPTH;
        bdev->tag_set.numa_node = NUMA_NO_NODE;
        bdev->tag_set.cmd_size = sizeof(64);
        bdev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
        bdev->tag_set.driver_data = bdev;

        if(blk_mq_alloc_tag_set(&bdev->tag_set))
        {
            print_alert("failed to allocate tag set");
            return -1;
        }
    }
    
    // Define Queue
    {
        struct request_queue * queue = blk_mq_init_queue(&bdev->tag_set);
        if(IS_ERR(queue))
        {
            print_alert("failed to allocate queue");
            return -1;
        }
        bdev->queue = queue;
        bdev->queue->queuedata = bdev;
    }

    // Define Disk
    {
        gd = alloc_disk(NUM_MINORS);
        if(gd == NULL)
        {   
            print_alert("failed to allocate disk");
            return -ENOMEM;
        }

        gd->flags = GENHD_FL_NO_PART_SCAN; // only one partition
        gd->flags |= GENHD_FL_REMOVABLE;
        gd->major = major;
        gd->first_minor = 0;
        gd->fops = &fops;
        gd->private_data = bdev;
        gd->queue = bdev->queue;
        snprintf(gd->disk_name, sizeof(gd->disk_name), NODE_NAME);
        set_capacity(gd, KERNEL_SECTOR_SIZE);
        
        bdev->gd = gd;
        add_disk(gd);
    }
    return 0;
}

static void device_deconstruct(struct vbdev_t * bdev)
{
    if(bdev != NULL)
    {
        if(bdev->queue != NULL)
        {
            blk_cleanup_queue(bdev->queue);
            bdev->queue = NULL;
        }

        if(bdev->tag_set.tags != NULL)
        {
            blk_mq_free_tag_set(&bdev->tag_set);
            bdev->tag_set.tags = NULL;
        }

        if(bdev->data != NULL)
        {
            kfree(bdev->data);
            bdev->data = NULL;
        }

        if(gd != NULL)
        {
            del_gendisk(gd);
            put_disk(gd);
            gd = NULL;
        }
    }
    return;
}

static __init int mod_init(void)
{
    pid = current->pid;
    major = register_blkdev(0, MODULE_NAME);
    if(major <= 0)
    {
        print_alert("failed to register major number");
        return -1;
    }
    
    if(device_construct(&dev) != 0)
    {
        unregister_blkdev(major, MODULE_NAME);
        print_alert("failed to create virtual device");
        return -1;
    }
    
    print_bold("driver initialized, %d:%d (PID %d)", major, 0, pid);
    return 0;
}

static __exit void mod_exit(void)
{
    unregister_blkdev(major, MODULE_NAME);
    device_deconstruct(&dev);
    print_bold("driver removed");
    return;
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_AUTHOR("Bilal Salha");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A block device driver for a virtual block device");
MODULE_VERSION("1.0");

