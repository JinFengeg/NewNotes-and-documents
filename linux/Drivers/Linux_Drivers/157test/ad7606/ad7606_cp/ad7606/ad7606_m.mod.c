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
	{ 0xdf76bbeb, "iio_pollfunc_store_time" },
	{ 0xfd93f8e3, "iio_trigger_notify_done" },
	{ 0xc0dffaa7, "devm_gpiod_get" },
	{ 0x33c2388, "__spi_register_driver" },
	{ 0x67ea780, "mutex_unlock" },
	{ 0x9a3253f1, "iio_get_time_ns" },
	{ 0x5bbe49f4, "__init_waitqueue_head" },
	{ 0xcc03df40, "iio_trigger_poll_chained" },
	{ 0xf58d879, "devm_iio_trigger_alloc" },
	{ 0xc609bfad, "_dev_warn" },
	{ 0x5f754e5a, "memset" },
	{ 0x7b781cfa, "devm_gpiod_get_array_optional" },
	{ 0xef4fb956, "devm_iio_device_alloc" },
	{ 0xa99ff993, "iio_triggered_buffer_postenable" },
	{ 0xe346f67a, "__mutex_init" },
	{ 0xc5850110, "printk" },
	{ 0x258cc36b, "__devm_iio_device_register" },
	{ 0x1337fb65, "__devm_iio_trigger_register" },
	{ 0xbb02a4d3, "driver_unregister" },
	{ 0xc271c3be, "mutex_lock" },
	{ 0x3f2f80ae, "iio_trigger_validate_own_device" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x53b017db, "_dev_err" },
	{ 0xd3b7c6eb, "spi_sync" },
	{ 0xb910a3e7, "iio_device_claim_direct_mode" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xb41bb7c6, "iio_triggered_buffer_predisable" },
	{ 0xc7011cb4, "get_device" },
	{ 0x854fc50, "__module_get" },
	{ 0xdab9e593, "devm_gpiod_get_optional" },
	{ 0x5e47e2ef, "spi_get_device_id" },
	{ 0x490f3976, "iio_device_release_direct_mode" },
	{ 0x1cbf53a, "devm_iio_triggered_buffer_setup" },
	{ 0x314b20c8, "scnprintf" },
	{ 0xa678ed4f, "iio_push_to_buffers" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xe6c12171, "complete" },
	{ 0x9c3c5123, "gpiod_get_raw_value" },
	{ 0x89ce056c, "gpiod_set_value" },
	{ 0xd83dc5e6, "wait_for_completion_timeout" },
	{ 0x1f144533, "devm_request_threaded_irq" },
	{ 0xa7d5e1f1, "gpiod_set_array_value" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cadi,ad7606-8");
MODULE_ALIAS("of:N*T*Cadi,ad7606-8C*");
MODULE_ALIAS("spi:ad7605-4");
MODULE_ALIAS("spi:ad7606-4");
MODULE_ALIAS("spi:ad7606-6");
MODULE_ALIAS("spi:ad7606-8");
MODULE_ALIAS("spi:ad7606b");
MODULE_ALIAS("spi:ad7616");
