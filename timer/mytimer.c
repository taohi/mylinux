#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
MODULE_LICENSE("Dual BSD/GPL");

static int  __init mytimer_init(void)
{
    printk("In function:%s\n",__func__);
    return 0;
}

static void  __exit mytimer_exit(void)
{
    printk("In function:%s\n",__func__);
}

module_init(mytimer_init);
module_exit(mytimer_exit);
