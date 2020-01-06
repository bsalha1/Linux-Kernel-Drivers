#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x25a444ff, "module_layout" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0x2c5c32f6, "device_add_disk" },
	{ 0x6bdfe9e6, "__alloc_disk_node" },
	{ 0x23bbf3, "blk_mq_init_queue" },
	{ 0x12740ef4, "blk_mq_alloc_tag_set" },
	{ 0xf02216ad, "kmem_cache_alloc_trace" },
	{ 0x1148c8a, "kmalloc_caches" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0xa047b91, "__blk_mq_end_request" },
	{ 0x3d456bca, "blk_update_request" },
	{ 0x56470118, "__warn_printk" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x2568395b, "blk_mq_start_request" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";
