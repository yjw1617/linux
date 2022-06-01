#include <linux/module.h>
#include <linux/kernel.h>
int gx  =10;
char* gstr = "hello";
int garr[5] = {1,2,3,4,5};

module_param(gx, int, 0664);
module_param(gstr, charp, 0664);
module_param_array(garr, int, NULL, 0664);
int __init testparam_init(void)
{
	int i = 0;
	printk("gx=%d\n", gx);
	printk("gstr=%s\n", gstr);
	for(i = 0;i < 5;i++){
		printk("%d ",garr[i]);
	}
	printk("\n");
	return 0;
}

void __exit testparam_exit(void)
{
	printk("testparam will exit\n");
}

//连接标记 将"GPL"放到ko文件  许可证协议 大家需要遵循的开源协议
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hiro Wang");//作者名
MODULE_DESCRIPTION("It is only a simple test");//描述
MODULE_ALIAS("HI");//为模块起别名

module_init(testparam_init);
module_exit(testparam_exit);
