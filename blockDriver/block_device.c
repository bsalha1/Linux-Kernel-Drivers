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

#define NUM_MINORS 1
#define NUM_SECTORS 1024

#define DATA_SIZE 1024
#define HARDSECT_SIZE 512
#define KERNEL_SECTOR_SIZE 512

#define MODULE_NAME "block_device"
#define QUEUE_DEPTH 128 // Max waitlist

typedef struct vbdev_t
{
    sector_t capacity;
    u8 * data;

    atomic_t open_counter;
    struct blk_mq_tag_set tag_set;
    struct request_queue * queue;
    struct gendisk * gd;
} virtual_device;

struct vbdev_t dev;
unsigned int major;

int device_open(struct block_device * devptr, fmode_t mode)
{
    printk(KERN_INFO "block_device: device opened\n");
    return 0;
}

int device_ioctl(struct block_device * devptr, fmode_t mode, unsigned int cmd, unsigned long arg)
{
    printk(KERN_INFO "block_device: device ioctl\n");
    return 0;
}

void device_release(struct gendisk * gdptr, fmode_t mode)
{
    printk(KERN_INFO "block_device: device released\n");
    return;
}

int device_rw_page(struct block_device * bdev, sector_t sector, struct page * pg, unsigned int num)
{
    printk(KERN_INFO "block_device: device read/write page\n");
    return 0;
}

static int do_request(struct request * rq, unsigned int * num_bytes)
{
    struct bio_vec bvec;
    struct req_iterator iter;
    struct vbdev_t * bdev = rq->q->queuedata;
    loff_t pos = blk_rq_pos(rq) << SECTOR_SHIFT;
    loff_t dev_size = (loff_t) (bdev->capacity << SECTOR_SHIFT);

    printk(KERN_INFO "block_device: request start from sector %ld\n", blk_rq_pos(rq));

    rq_for_each_segment(bvec, rq, iter)
    {
        unsigned long b_len = bvec.bv_len;
        void * buf = page_address(bvec.bv_page) + bvec.bv_offset;
        if((pos + b_len) > dev_size)
        {
            b_len = (unsigned long) (dev_size - pos);
        }

        // If write
        if(rq_data_dir(rq))
        {
            memcpy(bdev->data + pos, buf, b_len);
        }
        else // If read
        {
            memcpy(buf, bdev->data + pos, b_len);
        }

        pos += b_len;
        (*num_bytes) += b_len;
    }

    return 0;
}

static blk_status_t queue_rq(struct blk_mq_hw_ctx * hctx, const struct blk_mq_queue_data *bd)
{
    printk(KERN_INFO "block_device: request begin\n");
    blk_status_t status = BLK_STS_OK;
    struct request * rq = bd->rq;

    blk_mq_start_request(rq);

    unsigned int num_bytes = 0;
    if(do_request(rq, num_bytes) != 0)
    {
        status = BLK_STS_IOERR;
    }

    printk(KERN_WARNING "block_device: request process %d bytes\n", num_bytes);
    blk_update_request(rq, status, num_bytes);
    __blk_mq_end_request(rq, status);
    return BLK_STS_OK;
}

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
    .rw_page = device_rw_page
};

static int device_construct(struct vbdev_t * bdev)
{
    bdev = kzalloc(sizeof(struct vbdev_t), GFP_KERNEL);
    if(bdev == NULL)
    {
        printk(KERN_INFO "block_device: unable to allocate %ld bytes for device\n", sizeof(struct vbdev_t));
        return -ENOMEM;
    }

    // Allocate Data
    {
        bdev->data = kzalloc(DATA_SIZE, GFP_KERNEL);
        if(bdev->data == NULL)
        {
            printk(KERN_INFO "block_device: unable to allocate %ld bytes for device data\n", sizeof(struct vbdev_t));
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
            printk(KERN_ALERT "block_device: failed to allocate tag set\n");
            return -1;
        }
    }
    
    // Define Queue
    {
        struct request_queue * queue = blk_mq_init_queue(&bdev->tag_set);
        if(IS_ERR(queue))
        {
            printk(KERN_ALERT "block_device: failed to allocate queue\n");
            return -1;
        }
        bdev->queue = queue;
        bdev->queue->queuedata = bdev;
    }

    // Define Disk
    {
        struct gendisk * gd = alloc_disk(NUM_MINORS);
        if(gd == NULL)
        {   
            printk(KERN_ALERT "block_device: failed to allocate disk\n");
            return -ENOMEM;
        }

        gd->flags = GENHD_FL_NO_PART_SCAN; // only one partition
        gd->flags |= GENHD_FL_REMOVABLE;
        gd->major = major;
        gd->first_minor = 0;
        gd->fops = &fops;
        gd->private_data = bdev;
        gd->queue = bdev->queue;
        snprintf(gd->disk_name, sizeof(gd->disk_name), "vbdev");
        set_capacity(gd, KERNEL_SECTOR_SIZE);
        
        bdev->gd = gd;
        add_disk(gd);
    }

    printk(KERN_INFO "block_device: virtual block device created\n");
    return 0;
}

static void device_deconstruct(struct vbdev_t * bdev)
{
    if(bdev->gd)
    {
        del_gendisk(bdev->gd);
    }

    if(bdev->queue)
    {
        blk_cleanup_queue(bdev->queue);
    }

    if(bdev->tag_set.tags)
    {
        blk_mq_free_tag_set(&bdev->tag_set);
    }

    if(bdev->gd)
    {
        put_disk(bdev->gd);
    }

    kfree(bdev->data);
    kfree(bdev);
    return;
}

__init int mod_init(void)
{
    major = register_blkdev(0, MODULE_NAME);
    if(major <= 0)
    {
        printk(KERN_ALERT "block_device: failed to register major number\n");
        return -1;
    }
    printk(KERN_INFO "block_device: registered major number\n");
    
    if(device_construct(&dev) != 0)
    {
        unregister_blkdev(major, MODULE_NAME);
        printk(KERN_ALERT "block_device: failed to create virtual device\n");
        return -1;
    }
    printk(KERN_INFO "block_device: virtual device created\n");
    printk(KERN_INFO "block_device: driver initialized\n");
    return 0;
}

static void mod_exit(void)
{
    device_deconstruct(&dev);
    printk(KERN_INFO "block_device: virtual device destroyed\n");

    unregister_blkdev(major, MODULE_NAME);
    printk(KERN_INFO "block_device: driver removed\n");
    return;
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");

