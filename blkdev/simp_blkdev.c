#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>

#define SIMP_BLKDEV_DISKNAME "simp_blkdev"
#define SIMP_BLKDEV_DEVICEMAJOR 249
#define SIMP_BLKDEV_BYTES (16*1024*1024)

//static struct request_queue *simp_blkdev_queue;
static struct gendisk *simp_blkdev_disk;
unsigned char simp_blkdev_data[SIMP_BLKDEV_BYTES];

static void simp_blkdev_do_request(struct request_queue *q);

static struct block_device_operations simp_blkdev_fops={
    .owner = THIS_MODULE,
};

static void simp_blkdev_do_request(struct request_queue *q)
{
   struct request *req ;
   while((req=blk_fetch_request(q))!=NULL)
   {
       if((blk_rq_pos(req)+ blk_rq_cur_sectors(req))<<9 >SIMP_BLKDEV_BYTES)
       {
           printk(KERN_ERR "bad request\n");
           blk_end_request_all(req ,0);
           continue;
       }
       switch(rq_data_dir(req))
       {
           case READ:
               memcpy(req->buffer,simp_blkdev_data+(blk_rq_pos(req)<<9),blk_rq_cur_sectors(req)<<9);
               blk_end_request_all(req,1);
               break;
           case WRITE:
               memcpy(simp_blkdev_data+(blk_rq_pos(req)<<9),req->buffer,blk_rq_cur_sectors(req)<<9);
               blk_end_request_all(req,1);
               break;
           default:
               break;
       }

   }

}

static int __init simp_blkdev_init(void)
{
    int ret;
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
    simp_blkdev_disk->queue =blk_init_queue(simp_blkdev_do_request,NULL);
    set_capacity(simp_blkdev_disk,SIMP_BLKDEV_BYTES>>9);
    
    add_disk(simp_blkdev_disk);
    return 0;
err_alloc_disk:
    return ret;
}

static void __exit simp_blkdev_exit(void)
{
    del_gendisk(simp_blkdev_disk);
    put_disk(simp_blkdev_disk);
    blk_cleanup_queue(simp_blkdev_disk->queue);
}

module_init(simp_blkdev_init);
module_exit(simp_blkdev_exit);
