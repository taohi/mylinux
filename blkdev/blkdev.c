#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>

#define BLKDEV_DISKNAME "blkdev"
#define BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define BLKDEV_BYTES (16*1024*1024) //16M

unsigned char blkdev_data[BLKDEV_BYTES];

static struct gendisk *blkdev_disk;
static struct request_queue *blkdev_queue;

static struct block_device_operations blkdev_fops = {
 .owner = THIS_MODULE,
};

static void blkdev_do_request(struct request_queue *q)
{
    struct request *req;
    req = blk_fetch_request(q); 

    while(req) {
        unsigned long start = blk_rq_pos(req) << 9;
        unsigned long len = blk_rq_cur_sectors(req) << 9;
        int err = 0;

        if(start + len > BLKDEV_BYTES) {
            printk(KERN_ERR BLKDEV_DISKNAME ":bad access:block=%lu,count=%u\n",blk_rq_pos(req), blk_rq_cur_sectors(req));
            err = -EIO;
            goto done;
        }
        if(rq_data_dir(req) == READ)
            memcpy(req->buffer, blkdev_data+start,len);
        else
            memcpy(blkdev_data+start, req->buffer, len);
done:
        if(!__blk_end_request_cur(req, err))
            req = blk_fetch_request(q);
    }
}

static int __init blkdev_init(void)
{
    int ret;

    blkdev_queue = blk_init_queue(blkdev_do_request, NULL);
    if(!blkdev_queue) {
        ret = -ENOMEM;
        goto err_init_queue;
    }

    blkdev_disk = alloc_disk(1);
    if(!blkdev_disk) {
        goto err_alloc_disk;
    }
    strcpy(blkdev_disk->disk_name, BLKDEV_DISKNAME);
    blkdev_disk->major = BLKDEV_DEVICEMAJOR;
    blkdev_disk->first_minor = 0;
    blkdev_disk->fops = &blkdev_fops;
    blkdev_disk->queue = blkdev_queue;
    set_capacity(blkdev_disk, BLKDEV_BYTES>>9); 
    add_disk(blkdev_disk);
    return 0;

err_alloc_disk:
    blk_cleanup_queue(blkdev_queue);
err_init_queue:
    return ret;
}

static void __exit blkdev_exit(void)
{
    blk_cleanup_queue(blkdev_queue);
    del_gendisk(blkdev_disk);
    put_disk(blkdev_disk);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("t4ohi");
module_init(blkdev_init);
module_exit(blkdev_exit);
