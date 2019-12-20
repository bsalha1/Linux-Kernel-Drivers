#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

int param = 0;
/*
    S_IRUSR
    S_IWUSR
    S_IXUSR
    S_IWGRP
    S_IRGRP
*/
module_param(param, int, S_IRUSR | S_IWUSR);

void display(void)
{
    printk(KERN_INFO "argsMod: %d\n", param);
    return;
}

static int mod_init(void)
{
    printk(KERN_INFO "argsMod: initiating\n");
    display();
    return 0;
}

static void mod_exit(void)
{
    printk(KERN_INFO "argsMod: exitting\n");
    return;
}

module_init(mod_init);
module_exit(mod_exit);
