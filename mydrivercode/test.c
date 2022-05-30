#include <linux/module.h>
#include <linux/kernel.h>
int test_func(void);
int __init test_init(void)
{
	test_func();
	return 0;
}

void __exit test_exit(void)
{
	printk("test will exit\n");
}

//连接标记 将"GPL"放到ko文件  许可证协议 大家需要遵循的开源协议
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hiro Wang");
MODULE_DESCRIPTION("It is only a simple test");
MODULE_ALIAS("HI");

module_init(test_init);
module_exit(test_exit);
