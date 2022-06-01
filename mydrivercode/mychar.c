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
// sudo grep -r "file_operations" 文件夹路径	在文件夹下面查找file_operations字符串
#define BUF_LEN 100
int major = 11;
int minor = 0;
int mychar_num = 1;

struct mychar_dev{
	struct cdev mydev;
	char mydev_buf[BUF_LEN];
	int curlen;
	wait_queue_head_t wq;
	wait_queue_head_t rq;
	// wait_queue_head_t sq;

	struct fasync_struct *pasync_obj;//diveces signal
};

struct mychar_dev gmydev;

struct cdev mydev = {
	.owner = THIS_MODULE,
};


int mychar_open(struct inode *pnode, struct file *pfile){
	pfile->private_data = (void*)(container_of(pnode->i_cdev, struct mychar_dev, mydev));
	printk("my char open is called\n");
	return 0;
}
int mychar_close(struct inode *pnode, struct file *pfile){
	struct mychar_dev* pmydev = (struct mychar_dev*)(pfile->private_data);
	/*release函数中释放异步通知结构对象*/
	if (pmydev->pasync_obj) {
		fasync_helper(-1, pfile, 0, &pmydev->pasync_obj);
	}

	printk("my char close is called\n");
	return 0;
}
ssize_t mychar_read (struct file *pfile, char __user *puser, size_t count, loff_t *p_pos){
	int size = 0;
	int ret = 0;
	struct mychar_dev* pmydev = (struct mychar_dev*)(pfile->private_data);

	//阻塞读
	if(pmydev->curlen <= 0){
		if(pfile->f_flags & O_NONBLOCK){//如果应用程序选择非阻塞
			printk("read is error\n");
			return -1;
		}else{//如果应用程序选择阻塞
			wait_event_interruptible(pmydev->rq,pmydev->curlen > 0);//如果有数据可读则醒来
		}
	}

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

	//解除阻塞写
	wake_up_interruptible(&pmydev->wq);//发出中断唤醒写
	return size;
}
ssize_t mychar_write (struct file *filp, const char __user *puser, size_t count, loff_t *p_pos){
	struct mychar_dev* pmydev = (struct mychar_dev*)(filp->private_data);
	int size = 0;
	int ret = 0;

	//阻塞写
	if(pmydev->curlen >= BUF_LEN){
		if(filp->f_flags & O_NONBLOCK){
			printk("write data error ,buf is full\n");
			return -1;
		}else{
			wait_event_interruptible(pmydev->wq,pmydev->curlen < BUF_LEN);//如果有数据可读则醒来
		}
	}

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
	
	//解除阻塞读
	wake_up_interruptible(&pmydev->rq);//发出中断唤醒读

	//给应用层发信号
	if (pmydev->pasync_obj){
		kill_fasync(&pmydev->pasync_obj, SIGIO, POLL_IN);
	}
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
unsigned int mychar_poll(struct file *pfile, poll_table *ptl) //函数名初始化给struct file_operations的成员.poll
{
	struct mychar_dev* pmydev = (struct mychar_dev*)(pfile->private_data);
    unsigned int mask = 0;
    /*
    	1. 将所有等待队列头加入poll_table表中
    	2. 判断是否可读，如可读则mask |= POLLIN | POLLRDNORM;
    	3. 判断是否可写，如可写则mask |= POLLOUT | POLLWRNORM;
    */
   	poll_wait(pfile, &pmydev->rq, ptl);
    poll_wait(pfile, &pmydev->wq, ptl);
	if(pmydev->curlen > 0){
		mask |= POLLIN | POLLRDNORM;
	}
	if(pmydev->curlen < BUF_LEN){
		mask |= POLLOUT | POLLWRNORM;
	}
    return mask;
}

int mychar_fasync(int fd,struct file *pfile,int mode){
	struct mychar_dev *pmydev = (struct mychar_dev *)pfile->private_data;
	return fasync_helper(fd,pfile,mode,&pmydev->pasync_obj);
}
struct file_operations myops = {
	.owner = THIS_MODULE,
	.open = mychar_open,
	.release = mychar_close,
	.read = mychar_read,
	.write = mychar_write,
	.unlocked_ioctl = mychar_ioctl,//属性访问
	.poll = mychar_poll,//多路复用支持
	.fasync = mychar_fasync,//信号驱动
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

	/*读写等待阻塞*/
	init_waitqueue_head(&gmydev.rq); //初始化读等待头
	init_waitqueue_head(&gmydev.wq); //初始化写等待头	

	/*给struct_cdev对象指定操作函数集*/
	cdev_init(&gmydev.mydev, &myops);
	/*将struct_cedv对象添加到内核对应的数据结构里*/
	gmydev.mydev.owner = THIS_MODULE;
	cdev_add(&gmydev.mydev, devno, mychar_num);


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
