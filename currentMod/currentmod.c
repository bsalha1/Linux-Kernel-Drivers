#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>

int mod_init(void)
{
    printk(KERN_ALERT "currentMod.ko is initiating\n");
    printk(KERN_INFO "The current process is \"%s\", (pid %i)\n", current->comm, current->pid);
    return 0;
}

void mod_exit(void)
{
    printk(KERN_ALERT "currentMod.ko is unloading\n");
}

module_init(mod_init);
module_exit(mod_exit);
