#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

int module_function(void);

__init int mod_init(void)
{
    printk(KERN_INFO "usingSymbols: init\n");
    module_function();
    return 0;
}

void mod_exit(void)
{
    printk(KERN_INFO "usingSymbols: exit\n");
    return;
}

module_init(mod_init);
module_exit(mod_exit);
