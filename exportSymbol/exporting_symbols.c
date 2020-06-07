#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

int module_function(void)
{
    printk(KERN_INFO "inside the %s function\n", __FUNCTION__); 
    return 0;
}
EXPORT_SYMBOL(module_function);

__init int mod_init(void)
{
    printk(KERN_INFO "exportingSymbols: init\n");
    return 0;
}

void mod_exit(void)
{
    printk(KERN_INFO "exportingSymbols: exit\n");
    return;
}

module_init(mod_init);
module_exit(mod_exit);
