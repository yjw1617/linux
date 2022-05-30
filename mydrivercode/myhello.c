#include <linux/module.h>
#include <linux/kernel.h>

int __init myhello_init(void)
{
	printk("##############################################\n");
	printk("##############################################\n");
	printk("##############################################\n");
	printk("##############################################\n");
	printk("hello world!\n");
	printk("##############################################\n");
	printk("##############################################\n");
	printk("##############################################\n");
	printk("##############################################\n");

	return 0;
}

void __exit myhello_exit(void)
{
	printk("myhello will exit\n");
}

//连接标记 将"GPL"放到ko文件  许可证协议 大家需要遵循的开源协议
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hiro Wang");//作者名
MODULE_DESCRIPTION("It is only a simple test");//描述
MODULE_ALIAS("HI");//为模块起别名

module_init(myhello_init);
module_exit(myhello_exit);
