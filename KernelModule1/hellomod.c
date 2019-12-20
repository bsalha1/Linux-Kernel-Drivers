#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

int param = 0;

/*
    S_IRUSR
    S_IWUSR
    S_IXUSR
    S_IWGRP
    S_IRGRP
*/

// This variable can be read or written to
module_param(param, int, S_IRUSR | S_IWUSR);

void display(void)
{
    printk(KERN_ALERT "TEST: param = %d\n", param);
}

static int hellomod_init(void)
{
    printk(KERN_ALERT "TEST: Hello world\n");
    display();
    return 0;
}

static void hellomod_exit(void)
{
    printk(KERN_ALERT "TEST: goodbye world\n");
}

module_init(hellomod_init);
module_exit(hellomod_exit);
