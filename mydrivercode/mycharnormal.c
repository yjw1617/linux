// #include <linux/module.h>
// #include <linux/kernel.h>
// #include <linux/fs.h>
// #include <linux/cdev.h>
// #include <asm/uaccess.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#define BUF_LEN 100
int major = 11;
int minor = 0;
int mychar_num = 1;

struct cdev mydev;
char mydev_buf[BUF_LEN];
int curlen = 0;
int mychar_open(struct inode *pnode, struct file *pfile){
	printk("my char open is called\n");
	return 0;
}
int mychar_close(struct inode *pnode, struct file *pfile){
	printk("my char close is called\n");
	return 0;
}
ssize_t mychar_read (struct file *pfile, char __user *puser, size_t count, loff_t *p_pos){
	int size = 0;
	if(count > curlen){
		size = curlen;
	}else{
		size = count;
	}
	int ret = 0;
	ret = copy_to_user(puser, mydev_buf, size);
	if(ret){
		printk("copy to user failed\n");
		return -1;
	}
	memcpy(mydev_buf, mydev_buf+size, curlen - size);
	curlen = curlen - size;
	return size;
}
ssize_t mychar_write (struct file *filp, const char __user *puser, size_t count, loff_t *p_pos){
	int size = 0;
	int ret = 0;
	if(count > BUF_LEN - curlen){
		size = BUF_LEN - curlen;
	}else{
		size = count;
	}
	ret = copy_from_user(mydev_buf + curlen, puser, size);
	if(ret){
		printk("copy from user failed\n");
		return -1;
	}
	curlen = curlen + size;
	return size;
}
struct cdev mydev = {
	.owner = THIS_MODULE,
};
struct file_operations myops = {
	.owner = THIS_MODULE,
	.open = mychar_open,
	.release = mychar_close,
	.read = mychar_read,
	.write = mychar_write,
};

int __init mychar_init(void)
{
	int ret = 0;
	dev_t devno = MKDEV(major, minor);
	/*申请设备号*/
	ret = register_chrdev_region(devno, mychar_num, "mychar");
	if(ret){
		ret = alloc_chrdev_region(&devno, minor, mychar_num, "mychar");
		if(ret){
			printk("get devno failed\n");
			return -1;
		}
		major = MAJOR(devno);// 这一步容易遗漏
		//次设备号不用管 从0开始
	}
	/*给struct_cdev对象指定操作函数集*/
	cdev_init(&mydev, &myops);
	/*将struct_cedv对象添加到内核对应的数据结构里*/
	
	cdev_add(&mydev, devno, mychar_num);

	return 0;
}

void __exit mychar_exit(void)
{
	dev_t devno = MKDEV(major, minor);
	cdev_del(&mydev);
	unregister_chrdev_region(devno, mychar_num);
	printk("mychar will exit\n");
}

//连接标记 将"GPL"放到ko文件  许可证协议 大家需要遵循的开源协议
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hiro Wang");//作者名
MODULE_DESCRIPTION("It is only a simple test");//描述
MODULE_ALIAS("HI");//为模块起别名

module_init(mychar_init);
module_exit(mychar_exit);
