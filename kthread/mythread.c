#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/kthread.h>
MODULE_LICENSE("Dual BSD/GPL");

static struct task_struct *my_task=NULL;
int count=0;
int my_print(void *data);

static int mythread_init (void)
{
    int err;
    printk("function %s called.\n",__func__);
    my_task=kthread_create(my_print,"Hello world","my_thread");
    if(IS_ERR(my_task))
    {
        printk("Failed to create kthead.\n");
        err=PTR_ERR(my_task);
        return err;
    }
    wake_up_process(my_task);
    return 0;
}
/*
int my_print(void *data)
{
    while(!kthread_should_stop())
    {
        set_current_state(TASK_RUNNING);
        printk("thread says:%s\n",(char *)data);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(HZ);
    }
    return 0;
}
*/

int my_print(void *data)
{
    while(!kthread_should_stop())
    {
        printk("function %s called.\n",__func__);
        set_current_state(TASK_RUNNING);
        if(count<10)
        {
            count++;
            printk("thread says:%s,count=%d\n",(char *)data,count);
        }
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(HZ);
    }
    return 0;
}

static void mythread_exit (void)
{
    printk("function %s called.\n",__func__);
    if(my_task)
        kthread_stop(my_task);
}

module_init(mythread_init);
module_exit(mythread_exit);
