#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
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
__used __section(__versions) = {
	{ 0x4b3323eb, "module_layout" },
	{ 0xbb02a4d3, "driver_unregister" },
	{ 0x33c2388, "__spi_register_driver" },
	{ 0x1bbe6db3, "spi_setup" },
	{ 0x42e555a9, "device_create" },
	{ 0x9192c2a4, "__class_create" },
	{ 0xdd504b4a, "cdev_add" },
	{ 0x294045e0, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x102bd607, "__regmap_init_spi" },
	{ 0x746ae499, "devm_kmalloc" },
	{ 0xc5850110, "printk" },
	{ 0xf13e36de, "regmap_read" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x39e49525, "regmap_write" },
	{ 0x189c5980, "arm_copy_to_user" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xb99f48eb, "regmap_bulk_read" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xba510e53, "regmap_exit" },
	{ 0x75736401, "class_destroy" },
	{ 0x96f41d99, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xff2f3447, "cdev_del" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

MODULE_INFO(depends, "");

