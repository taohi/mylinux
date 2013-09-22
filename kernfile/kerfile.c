#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

#define MY_FILE "testfile"

char buf[128];
char buf2[128];
int ret1,ret2;
struct file *filp=NULL;

static int __init my_init(void)
{
	mm_segment_t old_fs;
	printk("%s() is called.\n",__func__);
	if(!filp)
		filp=filp_open(MY_FILE,O_RDWR|O_APPEND|O_CREAT,0644);
	if(IS_ERR(filp))
	{
		printk("error while opening file.\n");
		return 0;
	}

    sprintf(buf,"%s","what can you see in the jungle.\n");

	old_fs=get_fs();
	set_fs(KERNEL_DS);

	filp->f_op->write(filp,buf,sizeof(buf),&filp->f_pos);
	printk("Already Wrote to testfile!\n%d",sizeof(buf));
	filp->f_op->llseek(filp,0,0);
	filp->f_op->read(filp,buf2,sizeof(buf2),&filp->f_pos);
	buf2[127]='\0';
	printk("Already Read.buf2:%s\n",buf2);
	set_fs(old_fs);
	return 0;
}

static void __exit my_exit(void)
{
	printk("%s() is called.\n",__func__);
	if(!filp)
		filp_close(filp,NULL);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("taohi");
