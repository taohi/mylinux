#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/fs.h>
#include <linux/types.h>

#define SIMP_BLKDEV_DISKNAME "simp_blkdev"
#define SIMP_BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define SIMP_BLKDEV_BYTES (16*1024*1024)

static struct request_queue *simp_blkdev_queue;
static struct gendisk *simp_blkdev_disk;
unsigned char simp_blkdev_data[SIMP_BLKDEV_BYTES];

static struct block_device_operations simp_blkdev_fops={
    .owner = THIS_MODULE,
};

static void simp_blkdev_do_request(struct request_queue *q)
{
   struct request *req ;
   req = blk_fetch_request(q);
   while(req)
   {
       unsigned long start; 
       unsigned long len; 
       int err=0;
       start =blk_rq_pos(req)<<9; 
       len =blk_rq_cur_sectors(req)<<9;
       if(start + len >SIMP_BLKDEV_BYTES)
       {
           printk(KERN_ERR SIMP_BLKDEV_DISKNAME ":bad access:block=%lu,count=%u\n",blk_rq_pos(req), blk_rq_cur_sectors(req));
           err = -EIO;
           goto done;
       }
       if(rq_data_dir(req)==READ)
           memcpy(req->buffer,simp_blkdev_data+start,len);
       else
           memcpy(simp_blkdev_data+start,req->buffer,len);
done:
       if(!__blk_end_request_cur(req,err));
       req =blk_fetch_request(q);
   }
}

static int __init simp_blkdev_init(void)
{
    int ret;
    simp_blkdev_queue=blk_init_queue(simp_blkdev_do_request,NULL);
    if(!simp_blkdev_queue)
    {
        ret = -ENOMEM;
        goto err_alloc_queue;
    }
    simp_blkdev_disk = alloc_disk(1);
    if(!simp_blkdev_disk)
    {
        ret = -ENOMEM;
        goto err_alloc_disk;
    }
    strcpy(simp_blkdev_disk->disk_name,SIMP_BLKDEV_DISKNAME);
    simp_blkdev_disk->major = SIMP_BLKDEV_DEVICEMAJOR;
    simp_blkdev_disk->first_minor = 0;
    simp_blkdev_disk->fops = &simp_blkdev_fops;
    simp_blkdev_disk->queue =simp_blkdev_queue;
    set_capacity(simp_blkdev_disk,SIMP_BLKDEV_BYTES>>9);

    add_disk(simp_blkdev_disk);
    return 0;
err_alloc_disk:
    blk_cleanup_queue(simp_blkdev_queue);
err_alloc_queue:
    return ret;
}

static void __exit simp_blkdev_exit(void)
{
    blk_cleanup_queue(simp_blkdev_queue);
    del_gendisk(simp_blkdev_disk);
    put_disk(simp_blkdev_disk);
}

module_init(simp_blkdev_init);
module_exit(simp_blkdev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("taohi");
