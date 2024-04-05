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
	{ 0x3f2f80ae, "iio_trigger_validate_own_device" },
	{ 0xbb02a4d3, "driver_unregister" },
	{ 0x33c2388, "__spi_register_driver" },
	{ 0x53b017db, "_dev_err" },
	{ 0x1bbe6db3, "spi_setup" },
	{ 0x8d1ce01f, "__iio_device_register" },
	{ 0x1f144533, "devm_request_threaded_irq" },
	{ 0x854fc50, "__module_get" },
	{ 0xc7011cb4, "get_device" },
	{ 0x1337fb65, "__devm_iio_trigger_register" },
	{ 0xc1453300, "regmap_get_device" },
	{ 0xf58d879, "devm_iio_trigger_alloc" },
	{ 0x1cbf53a, "devm_iio_triggered_buffer_setup" },
	{ 0xdf76bbeb, "iio_pollfunc_store_time" },
	{ 0xe346f67a, "__mutex_init" },
	{ 0x102bd607, "__regmap_init_spi" },
	{ 0xef4fb956, "devm_iio_device_alloc" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0xe0165efa, "regmap_bulk_write" },
	{ 0x490f3976, "iio_device_release_direct_mode" },
	{ 0xb910a3e7, "iio_device_claim_direct_mode" },
	{ 0xe4463fa6, "iio_device_unregister" },
	{ 0xba510e53, "regmap_exit" },
	{ 0xd6e1560e, "iio_trigger_poll" },
	{ 0xc5850110, "printk" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xa678ed4f, "iio_push_to_buffers" },
	{ 0xfd93f8e3, "iio_trigger_notify_done" },
	{ 0xb99f48eb, "regmap_bulk_read" },
	{ 0xf13e36de, "regmap_read" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x67ea780, "mutex_unlock" },
	{ 0x39e49525, "regmap_write" },
	{ 0xc271c3be, "mutex_lock" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

MODULE_INFO(depends, "");

