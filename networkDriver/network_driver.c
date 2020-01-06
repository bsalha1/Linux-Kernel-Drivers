#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

__init int mod_init(void)
{

    return 0;
}

void mod_exit(void)
{

    return;
}

MODULE_AUTHOR("Bilal Salha");
MODULE_LICENSE("GPL");
module_init(mod_init);
module_exit(mod_exit);