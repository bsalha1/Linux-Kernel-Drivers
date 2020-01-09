#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/pci_ids.h>
#include <linux/pci_regs.h>
#include <linux/skbuff.h>

#include "print.h"

struct net_device * vnetdev;

int	vnetdev_init(struct net_device * dev)
{
    print_info("device initializing");
    return 0;
}

int vnetdev_open(struct net_device *dev)
{
    print_info("device open");
    netif_start_queue(dev);
    return 0;
}

void make_printable(unsigned char * data, int size, char * printable_data)
{
    int i;
    for(i = 0; i < size; i++)
    {
        if(data[i] < 128)
        {
            printable_data[i] = data[i];
        }
        else
        {
            printable_data[i] = '.';
        }
    }
}

netdev_tx_t vnetdev_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    print_bold("New Packet");
    print_info("Timestamp : %d", skb->tstamp);
    print_info("Protocol: %d", skb->protocol);
    print_info("Length: %d", skb->len);

    char buf[strlen((char*) skb->head)];
    make_printable(skb->head, strlen((char*)skb->head), buf);
    print_info("Head: %s", buf);
    print_info("Data: %s", skb->data);
    dev_kfree_skb_any(skb);
    return NETDEV_TX_OK;
}

int vnetdev_stop(struct net_device *dev)
{
    print_info("device stop");
    netif_stop_queue(dev);
    return 0;
}




struct net_device_ops nops = 
{
    .ndo_init = vnetdev_init,
    .ndo_open = vnetdev_open,
    .ndo_stop = vnetdev_stop,
    .ndo_start_xmit = vnetdev_start_xmit,
};

static void virtual_setup(struct net_device *dev)
{
    dev->netdev_ops = &nops;
}

static __init int mod_init(void)
{
    print_info("initializing driver");
    vnetdev = alloc_netdev(0, "vnetdev", NET_NAME_UNKNOWN, virtual_setup);

    print_info("registering device");
    if(register_netdev(vnetdev))
    {
        print_alert("failed to register net device");
        return -1;
	}
    print_info("registered device");
    print_bold("driver initialized");
    return 0;
}

static __exit void mod_exit(void)
{
    print_info("removing driver");
    unregister_netdev(vnetdev);
    print_bold("driver removed");
    return;
}

MODULE_AUTHOR("Bilal Salha");
MODULE_LICENSE("GPL");
module_init(mod_init);
module_exit(mod_exit);