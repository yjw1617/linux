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
#include "mychar.h"
#define BUF_LEN 100
#define MYCHAR_DEV_CNT 3
int major = 11;
int minor = 0;
int mychar_num = 3;

struct mychar_dev{
	struct cdev mydev;
	char mydev_buf[BUF_LEN];
	int curlen;
};

struct mychar_dev gmydev_arr[MYCHAR_DEV_CNT];

struct cdev mydev = {
	.owner = THIS_MODULE,
};


int mychar_open(struct inode *pnode, struct file *pfile){
	pfile->private_data = (void*)(container_of(pnode->i_cdev, struct mychar_dev, mydev));
	printk("my char open is called\n");
	return 0;
}
int mychar_close(struct inode *pnode, struct file *pfile){
	printk("my char close is called\n");
	return 0;
}
ssize_t mychar_read (struct file *pfile, char __user *puser, size_t count, loff_t *p_pos){
	int size = 0;
	int ret = 0;
	struct mychar_dev* pmydev = (struct mychar_dev*)(pfile->private_data);
	if(count > pmydev->curlen){
		size = pmydev->curlen;
	}else{
		size = count;
	}
	ret = copy_to_user(puser, pmydev->mydev_buf, size);
	if(ret){
		printk("copy to user failed\n");
		return -1;
	}
	memcpy(pmydev->mydev_buf, pmydev->mydev_buf+size, pmydev->curlen - size);
	pmydev->curlen = pmydev->curlen - size;
	return size;
}
ssize_t mychar_write (struct file *filp, const char __user *puser, size_t count, loff_t *p_pos){
	struct mychar_dev* pmydev = (struct mychar_dev*)(filp->private_data);
	int size = 0;
	int ret = 0;
	if(count > BUF_LEN - pmydev->curlen){
		size = BUF_LEN - pmydev->curlen;
	}else{
		size = count;
	}
	ret = copy_from_user(pmydev->mydev_buf + pmydev->curlen, puser, size);
	if(ret){
		printk("copy from user failed\n");
		return -1;
	}
	pmydev->curlen = pmydev->curlen + size;
	return size;
}


long mychar_ioctl(struct file* pfile, unsigned int cmd, unsigned long arg){
	struct mychar_dev* pmydev = (struct mychar_dev*)(pfile->private_data);
	int __user *pret = (int*)arg;
	int maxlen = BUF_LEN;
	int ret = 0;
	switch(cmd){
		case MYCHAR_IOCTL_GET_MAXLEN:
			ret = copy_to_user(pret, &maxlen, sizeof(int));
			if(ret){
				printk("copy to user MAXLEN error\n");
				return -1;
			}
			break;
		case MYCHAR_IOCTL_GET_CURLEN:
			ret = copy_to_user(pret, &pmydev->curlen, sizeof(int));
			if(ret){
				printk("copy to user CURLEN error\n");
				return -1;
			}
			break;
		default:
			printk("cmd is unknow\n");
			return -1;
	}
	return 0;
}

struct file_operations myops = {
	.owner = THIS_MODULE,
	.open = mychar_open,
	.release = mychar_close,
	.read = mychar_read,
	.write = mychar_write,
	.unlocked_ioctl = mychar_ioctl,
};

int __init mychar_init(void)
{
	int ret = 0;
	dev_t devno = MKDEV(major, minor);
	int i = 0;
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
	for(i = 0; i < MYCHAR_DEV_CNT; i++){
		devno = MKDEV(major, minor + i);
		/*给struct_cdev对象指定操作函数集*/
		cdev_init(&(gmydev_arr[i].mydev), &myops);
		/*将struct_cedv对象添加到内核对应的数据结构里*/
		gmydev_arr[i].mydev.owner = THIS_MODULE;
		cdev_add(&gmydev_arr[i].mydev, devno, 1);
	}
	return 0;  
}

void __exit mychar_exit(void)
{
	dev_t devno = MKDEV(major, minor);
	int i = 0;
	for(i = 0; i < MYCHAR_DEV_CNT; i++){
		cdev_del(&gmydev_arr[i].mydev);
	}
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
