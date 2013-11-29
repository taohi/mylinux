#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/hdreg.h>
#include <linux/radix-tree.h>

#define BLKDEV_DISKNAME "blkdev"
#define BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define BLKDEV_BYTES (16*1024*1024) //16M
#define BLKDEV_MAXPARTITIONS 4

static struct radix_tree_root blkdev_data;
static struct gendisk *blkdev_disk;
static struct request_queue *blkdev_queue;


void free_diskmem(void);
int alloc_diskmem(void);

static int blkdev_getgeo(struct block_device *bdev,struct hd_geometry *geo)
{
    if(BLKDEV_BYTES<16*1024*1024){
        geo->heads = 1;
        geo->sectors = 1;
    }else if(BLKDEV_BYTES < 512*1024*1024){
        geo->heads=1;
        geo->sectors=32;
    }
    return 0;
}

static struct block_device_operations blkdev_fops = {
 .owner = THIS_MODULE,
 .getgeo=blkdev_getgeo,
};

int alloc_diskmem(void)
{
    int ret,i;
    void *p;
    INIT_RADIX_TREE(&blkdev_data,GFP_KERNEL);
    for(i=0;i<(BLKDEV_BYTES + PAGE_SIZE - 1)>>PAGE_SHIFT;i++)
    {
        p=(void *)__get_free_page(GFP_KERNEL);
        if(!p){
            ret = -ENOMEM;
            goto err_alloc;
        }
        ret = radix_tree_insert(&blkdev_data,i,p);
        if(IS_ERR_VALUE(ret))
            goto err_radix_tree_insert;
    }
    return 0;

err_radix_tree_insert:
    free_page((unsigned long)p);
err_alloc:
    free_diskmem();
    return ret;

}

void free_diskmem(void)
{
    int i;
    void *p;
    for(i=0;i<(BLKDEV_BYTES + PAGE_SIZE - 1)>>PAGE_SHIFT;i++)
    {
        p = radix_tree_lookup(&blkdev_data,i);
        radix_tree_delete(&blkdev_data,i);
        free_page((unsigned long)p);
    }
}

static int blkdev_make_request(struct request_queue *q,struct bio *bio)
{
    struct bio_vec *bvec;
    int i;
    unsigned long long dsk_offset;
    dsk_offset=bio->bi_sector << PAGE_SHIFT;

    if((bio->bi_sector<<9)+bio->bi_size >BLKDEV_BYTES)
    {
        printk(KERN_ERR "bad request:out of blkdev size!\n");
        bio_endio(bio,-EIO);
    }

    bio_for_each_segment(bvec,bio,i)
    {
        unsigned int count_done,count_current;
        void *iovec_mem;
        void *dsk_mem;
        
        iovec_mem=kmap(bvec->bv_page) + bvec->bv_offset;
        count_done = 0;
        while(count_done < bvec->bv_len)
        {
            count_current = min(bvec->bv_len - count_done,(unsigned int))
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

    blkdev_disk = alloc_disk(BLKDEV_MAXPARTITIONS);
    if(!blkdev_disk) {
        goto err_alloc_disk;
    }

    blk_queue_make_request(blkdev_queue,&blkdev_make_request);

    ret = alloc_diskmem();
    if(IS_ERR_VALUE(ret))
        goto err_alloc_diskmem;
    
    strcpy(blkdev_disk->disk_name, BLKDEV_DISKNAME);
    blkdev_disk->major = BLKDEV_DEVICEMAJOR;
    blkdev_disk->first_minor = 0;
    blkdev_disk->fops = &blkdev_fops;
    blkdev_disk->queue = blkdev_queue;
    set_capacity(blkdev_disk, BLKDEV_BYTES>>9); 
    add_disk(blkdev_disk);
    return 0;

err_alloc_diskmem:
    put_disk(blkdev_disk);
err_alloc_disk:
    blk_cleanup_queue(blkdev_queue);
err_alloc_queue:
    return ret;
}

static void __exit blkdev_exit(void)
{
    blk_cleanup_queue(blkdev_queue);
    del_gendisk(blkdev_disk);
    free_diskmem();
    put_disk(blkdev_disk);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("t4ohi");
module_init(blkdev_init);
module_exit(blkdev_exit);
