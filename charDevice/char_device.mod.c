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
	{ 0x5fe7d95d, "module_layout" },
	{ 0x52e04d4f, "cdev_del" },
	{ 0x4920100f, "class_destroy" },
	{ 0x758708d1, "device_destroy" },
	{ 0x77facd38, "device_create" },
	{ 0x1a52faba, "cdev_add" },
	{ 0x450351e, "cdev_init" },
	{ 0xb978e7e4, "cdev_alloc" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x4b9e70ce, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xa916b694, "strnlen" },
	{ 0xcf2a6966, "up" },
	{ 0x21271fd0, "copy_user_enhanced_fast_string" },
	{ 0x1f199d24, "copy_user_generic_string" },
	{ 0xecdcabd2, "copy_user_generic_unrolled" },
	{ 0xc5850110, "printk" },
	{ 0x6bd0e573, "down_interruptible" },
	{ 0xbdfb6dbb, "__fentry__" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

