#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/slab.h>
MODULE_LICENSE("Dual BSD/GPL");
#define SHARE_SIZE 1   

char *shmem;   
struct page *share_page;      
static int my_major=244;

struct mmap_dev {
        int mem_var;
        struct cdev cdev;
} *mmap_dev;            

static int mmap_dev_open(struct inode *inode, struct file *file)
{
        printk("%s() is called.\n", __func__);
        return 0;
}

static int mmap_dev_release(struct inode *inode, struct file *file)
{
        printk("%s() is called.\n", __func__);
        return 0;
}

static ssize_t mmap_dev_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
        printk("%s() is called.\n", __func__);
        return 0;
}

static ssize_t mmap_dev_write(struct file *file, const char *buffer, size_t length, loff_t *offset)      
{
        printk("%s() is called.\n", __func__);
        return 0;
}
 
void mmap_dev_vma_open(struct vm_area_struct *vma)
 {
        printk("%s() is called.\n", __func__);
 }
 
void mmap_dev_vma_close(struct vm_area_struct *vma)
{
        printk("%s() is called.\n", __func__);
 }

static struct vm_operations_struct mmap_dev_remap_vm_ops = {
        .open = mmap_dev_vma_open,
        .close = mmap_dev_vma_close,
};

static int mmap_dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	printk("%s() is called.\n", __func__);
	if( remap_pfn_range(vma, vma->vm_start, page_to_pfn(share_page), 
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot) )
		return -EAGAIN;
	vma->vm_ops = &mmap_dev_remap_vm_ops;
	mmap_dev_vma_open(vma);
	return 0;
}

struct file_operations mmap_dev_fops = {
	.owner = THIS_MODULE,
	.open = mmap_dev_open,
	.release = mmap_dev_release,
	.read = mmap_dev_read,
	.write = mmap_dev_write,
	.mmap = mmap_dev_mmap,
};

static int __init mmap_dev_init(void)
{
	int ret;
	int err;
	dev_t my_dev = MKDEV(my_major, 0);
	ret=register_chrdev_region(my_dev, 1, "mmap_dev");
	if (ret < 0)          
	{
		ret = alloc_chrdev_region(&my_dev, 0, 1, "mmap_dev");
		my_major = MAJOR(my_dev);
	}  
	printk("alloc successfully mmap_dev:%d(MAJOR)\n", my_major);
	if (ret < 0)
		return ret;
	mmap_dev = kmalloc( sizeof(struct mmap_dev), GFP_KERNEL );
	if( !mmap_dev )
	{
		ret = -ENOMEM;
		printk("create device failed.\n");
	}
	else
	{
		mmap_dev->mem_var = 0;
		cdev_init( &mmap_dev->cdev, &mmap_dev_fops);
		mmap_dev->cdev.owner = THIS_MODULE;
		err = cdev_add( &mmap_dev->cdev, my_dev,1 );
		if(err)
			printk("Adding device failed.\n");  
	}
	share_page = alloc_pages(__GFP_WAIT, SHARE_SIZE);
	shmem = page_address(share_page);
	strcpy(shmem,"hello,haitao's mmap\n");
	return ret;
}

static void __exit mmap_dev_exit(void)
{
	cdev_del(&mmap_dev->cdev);                                 
	unregister_chrdev_region(MKDEV(my_major,0),1);         
	printk("unregister successfully mmap_dev.\n");
	kfree(mmap_dev);
}
module_init(mmap_dev_init);
module_exit(mmap_dev_exit);
