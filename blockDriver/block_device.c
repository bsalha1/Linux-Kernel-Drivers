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

#define DATA_SIZE 8192
#define HARDSECT_SIZE 512
#define KERNEL_SECTOR_SIZE 512

#define NODE_NAME "vbdev"
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

struct gendisk * gd;
struct vbdev_t dev;
unsigned int major;

void print(const char * format, const char * flag, va_list args)
{
    char message[strlen(format) + 6 + strlen(MODULE_NAME)];
    snprintf(message, sizeof(message), "%s%s: %s\n", flag, MODULE_NAME, format);
    vprintk(message, args);
}

void print_info(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    print(format, KERN_INFO, args);
    va_end(args);
}

void print_alert(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    print(format, KERN_ALERT, args);
    va_end(args);
}

void print_bold(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    print(format, KERN_WARNING, args);
    va_end(args);
}

int device_open(struct block_device * devptr, fmode_t mode)
{
    print_info("device opened");
    return 0;
}

int device_ioctl(struct block_device * devptr, fmode_t mode, unsigned int cmd, unsigned long arg)
{
    print_info("device ioctl");
    return 0;
}

void device_release(struct gendisk * gdptr, fmode_t mode)
{
    print_info("device released");
    return;
}

static int do_request(struct request * rq, unsigned int * num_bytes)
{
    struct bio_vec bvec;
    struct req_iterator iter;
    struct vbdev_t * bdev = rq->q->queuedata;
    loff_t pos = blk_rq_pos(rq) << SECTOR_SHIFT;
    loff_t dev_size = (loff_t) (bdev->capacity << SECTOR_SHIFT);

    print_info("request start from sector %lld", blk_rq_pos(rq));

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
            print_bold("processing write request");
            memcpy(bdev->data + pos, buf, b_len);
        }
        else // If read
        {
            print_bold("processing read request");
            memcpy(buf, bdev->data + pos, b_len);
        }

        pos += b_len;
        (*num_bytes) += b_len;
    }

    return 0;
}

static blk_status_t queue_rq(struct blk_mq_hw_ctx * hctx, const struct blk_mq_queue_data *bd)
{
    blk_status_t status = BLK_STS_OK;
    struct request * rq = bd->rq;

    blk_mq_start_request(rq);

    unsigned int num_bytes = 0;
    if(do_request(rq, &num_bytes) != 0)
    {
        status = BLK_STS_IOERR;
    }

    print_bold("request processed (%d bytes)", num_bytes);
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
    // .rw_page = device_rw_page
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
    print_bold("driver initialized, %d:%d", major, 0);
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
// MODULE_AUTHOR("Bilal Salha");
MODULE_LICENSE("GPL");
// MODULE_DESCRIPTION("A block device driver for a virtual block device");
// MODULE_VERSION("1.0");

