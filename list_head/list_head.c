#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
MODULE_LICENSE("Dual BSD/GPL");

struct module *m = &__this_module;

struct blklist_head_t{
    struct list_head list;
    struct spinlock lock;
};

struct list_node{
    struct list_head list;
    int number;    
};

static int list_module_init (void)
{
    struct blklist_head_t *free_list=kmalloc(sizeof(struct blklist_head_t),GFP_KERNEL);
    struct blklist_head_t *valid_list=kmalloc(sizeof(struct blklist_head_t),GFP_KERNEL);
    int i=0,count=10;
    struct list_node *node=NULL;
    printk("function %s called.\n",__func__);

    INIT_LIST_HEAD(&(free_list->list));
    INIT_LIST_HEAD(&(valid_list->list));
    spin_lock_init(&(free_list->lock));

    spin_lock(&(free_list->lock));
    if(list_empty(&(free_list->list)))
        printk("free_list is empty\n");
    for(i=0;i<count;i++)
    {
        node=kmalloc(sizeof(struct list_node),GFP_KERNEL);
        node->number=i;
        list_add_tail(&node->list,&free_list->list);
    }
    if(!list_empty(&(free_list->list)))
        printk("free_list is not empty now\n");
    spin_unlock(&(free_list->lock));


    list_for_each_entry(node,&free_list->list,list)
        printk("NODE:%d\n",node->number);
    
    list_for_each_entry(node,&free_list->list,list)
    {
        if (node->number==5)
           break;

    }
   // list_del(&node->list);
   // list_add_tail(&node->list,&valid_list->list);
    list_move_tail(&node->list,&valid_list->list);

    list_for_each_entry(node,&(free_list->list),list)
        printk("NODE:%d\n",node->number);
/*
    int i=0,length=10;
    struct list_node *nodes=(struct list_node *)kmalloc(length * sizeof(struct list_node) ,GFP_KERNEL);
    int *flags=(int *)kmalloc(20*sizeof(int),GFP_KERNEL);
    
//    struct list_node *nodes=(struct list_node *)vmalloc(length * sizeof(struct list_node));
    while(i<20){
        *(flags+i)=i;
        i++;
    }
    i=0; 
    while(i<length )
    {
        (nodes+i)->number = i;
        //printk("nodes[%d]:%d\n",i,(nodes+i)->number);
    //    (flags+i)[0]=1;
    //    (flags+i)[1]=2;
        printk("flags[%d][0]:%d\t flags[%d][1]:%d\n",i,(flags+i)[0],i,(flags+i)[1]);
        i++;
    }
    */
    return 0;

}

static void list_module_exit (void)
{
    printk("function %s called.\n",__func__);
}

module_init(list_module_init);
module_exit(list_module_exit);
