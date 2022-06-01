#include <linux/module.h>
#include <linux/kernel.h>
int gx = 19;
EXPORT_SYMBOL(gx);
int __init module_a_init(void)
{
	printk("module a init gx = %d\n", gx);
	return 0;
}

void __exit module_a_exit(void)
{
	printk("myhello will exit\n");
}

//连接标记 将"GPL"放到ko文件  许可证协议 大家需要遵循的开源协议
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hiro Wang");//作者名
MODULE_DESCRIPTION("It is only a simple test");//描述
MODULE_ALIAS("HI");//为模块起别名

module_init(module_a_init);
module_exit(module_a_exit);
