#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

int disk_size=1024;
module_param_named(size,disk_size,int,S_IRUGO);

static int  __init param_init(void) 
{
    printk("module init:disk_size now:%d\n",disk_size);
    return 0;
}

static void __exit param_exit(void)
{
    printk("exit module.\n");
}

module_init(param_init);
module_exit(param_exit);
MODULE_LICENSE("GPL");
