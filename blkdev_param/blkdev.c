#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/hdreg.h>
#include <linux/radix-tree.h>
#include <linux/version.h>
#include <linux/moduleparam.h>

#define BLKDEV_DISKNAME "blkdev"
#define BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define BLKDEV_MAXPARTITIONS 4

#define BLKDEV_DATASEGORDER (2)
#define BLKDEV_DATASEGSHIFT (PAGE_SHIFT + BLKDEV_DATASEGORDER)
#define BLKDEV_DATASEGSIZE  (PAGE_SIZE << BLKDEV_DATASEGORDER)
#define BLKDEV_DATASEGMASK  (~(BLKDEV_DATASEGSIZE - 1))

#define BLKDEV_SECTORSHIFT (9)
#define BLKDEV_SECTORSIZE (1ULL << BLKDEV_SECTORSHIFT)
#define BLKDEV_SECTOR_MASK (~(BLKDEV_SECTORSIZE-1))

static struct radix_tree_root blkdev_data;
static struct gendisk *blkdev_disk;
static struct request_queue *blkdev_queue;

static char *blkdev_param_size  ="16M";
module_param_named(size,blkdev_param_size,charp,S_IRUGO);
static unsigned long long blkdev_bytes;

void free_diskmem(void);
int alloc_diskmem(void);

int getparam(void)
{
    char uint;
    char tailc;
    if(sscanf(blkdev_param_size,"%llu%c%c",&blkdev_bytes,&uint,&tailc)!=2)
        return -EINVAL;
    if(!blkdev_bytes)
        return -EINVAL;
    switch (uint){
        case 'g':
        case 'G':
            blkdev_bytes<<=30;
            break;
        case 'm':
        case 'M':
            blkdev_bytes<<=20;
            break;
        case 'k':
        case 'K':
            blkdev_bytes<<=10;
            break;
        default:
            return -EINVAL;
    }
    blkdev_bytes=(blkdev_bytes+BLKDEV_SECTORSIZE-1)&BLKDEV_SECTOR_MASK;
        return 0;
}

static int blkdev_getgeo(struct block_device *bdev,struct hd_geometry *geo)
{
    if(blkdev_bytes<16*1024*1024){
        geo->heads = 1;
        geo->sectors = 1;
    }else if(blkdev_bytes < 512*1024*1024){
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
    for(i=0;i<(blkdev_bytes + BLKDEV_DATASEGSIZE - 1)>>BLKDEV_DATASEGSHIFT;i++)
    {
        p=(void *)__get_free_pages(GFP_KERNEL,BLKDEV_DATASEGORDER);
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
    free_pages((unsigned long)p,BLKDEV_DATASEGORDER);
err_alloc:
    free_diskmem();
    return ret;
}

void free_diskmem(void)
{
    int i;
    void *p;
    for(i=0;i<(blkdev_bytes + BLKDEV_DATASEGSIZE - 1)>>BLKDEV_DATASEGSHIFT;i++)
    {
        p = radix_tree_lookup(&blkdev_data,i);
        radix_tree_delete(&blkdev_data,i);
        free_pages((unsigned long)p,BLKDEV_DATASEGORDER);
    }
}

static int blkdev_make_request(struct request_queue *q, struct bio *bio)
{
        struct bio_vec *bvec;
        int i;
        unsigned long long dsk_offset;

        if ((bio->bi_sector << BLKDEV_SECTORSHIFT) + bio->bi_size > blkdev_bytes) {
                printk(KERN_ERR BLKDEV_DISKNAME
                        ": bad request: block=%llu, count=%u\n",
                        (unsigned long long)bio->bi_sector, bio->bi_size);
                bio_endio(bio, -EIO);
                return 0;
        }

        dsk_offset = bio->bi_sector << BLKDEV_SECTORSHIFT;
        bio_for_each_segment(bvec, bio, i) {
                unsigned int count_done, count_current;
                void *iovec_mem;
                void *dsk_mem;
                iovec_mem = kmap(bvec->bv_page) + bvec->bv_offset;

                count_done = 0;
                while (count_done < bvec->bv_len) {
                        count_current = min(bvec->bv_len - count_done,
                                (unsigned int)(BLKDEV_DATASEGSIZE
                                - ((dsk_offset + count_done) & ~BLKDEV_DATASEGMASK)));

                        dsk_mem = radix_tree_lookup(&blkdev_data,
                                (dsk_offset + count_done) >> BLKDEV_DATASEGSHIFT);
                        if (!dsk_mem) {
                                printk(KERN_ERR BLKDEV_DISKNAME
                                        ": search memory failed: %llu\n",
                                        (dsk_offset + count_done)
                                        >> BLKDEV_DATASEGSHIFT);
                                kunmap(bvec->bv_page);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
                                bio_endio(bio, 0, -EIO);
#else
                                bio_endio(bio, -EIO);
#endif
                                return 0;
                        }
                        dsk_mem += (dsk_offset + count_done) & ~BLKDEV_DATASEGSHIFT;

                        switch (bio_rw(bio)) {
                        case READ:
                        case READA:
                                memcpy(iovec_mem + count_done, dsk_mem,
                                        count_current);
                                break;
                        case WRITE:
                                memcpy(dsk_mem, iovec_mem + count_done,
                                        count_current);
                                break;
                        default:
                                printk(KERN_ERR BLKDEV_DISKNAME
                                        ": unknown value of bio_rw: %lu\n",
                                        bio_rw(bio));
                                kunmap(bvec->bv_page);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
                                bio_endio(bio, 0, -EIO);
#else
                                bio_endio(bio, -EIO);
#endif
                                return 0;
                        }
                        count_done += count_current;
                }

                kunmap(bvec->bv_page);
                dsk_offset += bvec->bv_len;
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
        bio_endio(bio, bio->bi_size, 0);
#else
        bio_endio(bio, 0);
#endif

        return 0;
}



static int __init blkdev_init(void)
{
    int ret = getparam();
    if(IS_ERR_VALUE(ret))
        goto err_getparam;
    blkdev_queue = blk_alloc_queue(GFP_KERNEL);
    if(!blkdev_queue) {
        ret = -ENOMEM;
        goto err_alloc_queue;
    }
    blk_queue_make_request(blkdev_queue,&blkdev_make_request);

    blkdev_disk = alloc_disk(BLKDEV_MAXPARTITIONS);
    if(!blkdev_disk) {
        goto err_alloc_disk;
    }

    ret = alloc_diskmem();
    if(IS_ERR_VALUE(ret))
        goto err_alloc_diskmem;

    strcpy(blkdev_disk->disk_name, BLKDEV_DISKNAME);
    blkdev_disk->major = BLKDEV_DEVICEMAJOR;
    blkdev_disk->first_minor = 0;
    blkdev_disk->fops = &blkdev_fops;
    blkdev_disk->queue = blkdev_queue;
    set_capacity(blkdev_disk, blkdev_bytes>>BLKDEV_SECTORSHIFT); 
    add_disk(blkdev_disk);
    return 0;

err_alloc_diskmem:
    put_disk(blkdev_disk);
err_alloc_disk:
    blk_cleanup_queue(blkdev_queue);
err_alloc_queue:
err_getparam:
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
