#include <linux/kernel.h>
#include <linux/blk-mq.h>

#include "print.h"
#include "ioctl.h"

typedef struct vbdev_t
{
    sector_t capacity;
    u8 * data;

    atomic_t open_counter;
    struct blk_mq_tag_set tag_set;
    struct request_queue * queue;

    struct gendisk * gd;
};

struct vbdev_t dev;
unsigned int major;
int pid;

int device_open(struct block_device * devptr, fmode_t mode)
{
    print_info("device opened");
    return 0;
}

int device_ioctl(struct block_device * devptr, fmode_t mode, unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case HANDSHAKE:
            return 0;
        case GET_MAJOR_NUMBER:
            return major;
        case GET_MINOR_NUMBER:
            return 0;
        case GET_REMAINING_SPACE:
            return 0;
        case GET_PID:
            return pid;
        default:
            return -EINVAL;
    }
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
    loff_t pos = rq->__sector << SECTOR_SHIFT;
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