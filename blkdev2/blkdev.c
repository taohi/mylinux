#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>

#define BLKDEV_DISKNAME "blkdev"
#define BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define BLKDEV_BYTES (16*1024*1024) //16M

unsigned char blkdev_data[BLKDEV_BYTES];

static struct gendisk *blkdev_disk;
static struct request_queue *blkdev_queue;

static struct block_device_operations blkdev_fops = {
 .owner = THIS_MODULE,
};

static int blkdev_make_request(struct request_queue *q,struct bio *bio)
{
    struct bio_vec *bvec;
    int i;
    void *dsk_mem;

    if((bio->bi_sector<<9)+bio->bi_size >BLKDEV_BYTES)
    {
        printk(KERN_ERR "bad request:out of blkdev size!\n");
        bio_endio(bio,-EIO);
    }

    dsk_mem = blkdev_data + (bio->bi_sector << 9);
    bio_for_each_segment(bvec,bio,i)
    {
        void *iovec_mem;
        switch(bio_rw(bio))
        {
            case  READ:
            case READA:
                iovec_mem = kmap(bvec->bv_page) + bvec->bv_offset;
                memcpy(iovec_mem,dsk_mem,bvec->bv_len);
                kunmap(bvec->bv_page);
                break;
            case WRITE:
                iovec_mem = kmap(bvec->bv_page) + bvec->bv_offset;
                memcpy(dsk_mem,iovec_mem,bvec->bv_len);
                kunmap(bvec->bv_page);
                break;
            default:
                printk(KERN_ERR "unknown request.\n");
                bio_endio(bio,-EIO);
                return 0;
        }
        dsk_mem += bvec->bv_len;
    }
    bio_endio(bio,0);
    return 0;
}


static int __init blkdev_init(void)
{
    int ret;
    blkdev_queue = blk_alloc_queue(GFP_KERNEL);
    if(!blkdev_queue) {
        ret = -ENOMEM;
        goto err_alloc_queue;
    }

    blkdev_disk = alloc_disk(1);
    if(!blkdev_disk) {
        goto err_alloc_disk;
    }

    blk_queue_make_request(blkdev_queue,&blkdev_make_request);
    
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
err_alloc_queue:
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
