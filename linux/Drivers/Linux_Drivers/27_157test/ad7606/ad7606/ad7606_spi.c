// SPDX-License-Identifier: GPL-2.0
/*
 * AD7606 SPI ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/types.h>
#include <linux/err.h>

#include <linux/iio/iio.h>
#include "ad7606.h"

static int ad7606_spi_read_block(struct device* dev,
	int count, void* buf)
{
	struct spi_device* spi = to_spi_device(dev);
	int i, ret;
	unsigned short* data = buf;
	__be16* bdata = buf;

	ret = spi_read(spi, buf, count * 2);
	if (ret < 0) {
		dev_err(&spi->dev, "SPI read error\n");
		//printk("SPI read error\n");
		return ret;
	}else if(ret ==0){
		//printk("spi_read return:%d\n",ret);
	}

	for (i = 0; i < count; i++)
	{
		data[i] = be16_to_cpu(bdata[i]);
		//printk("SPI read data: %u\n", data[i]);
	}
	return 0;
}


static const struct ad7606_bus_ops ad7606_spi_bops = {
	.read_block = ad7606_spi_read_block,
};



static int ad7606_spi_probe(struct spi_device* spi)
{
	
	const struct spi_device_id* id = spi_get_device_id(spi);
	const struct ad7606_bus_ops* bops;
	printk("ad7606 match successful\n");
	switch (id->driver_data) {

	default:
		bops = &ad7606_spi_bops;
		break;
	}

	return ad7606_probe(&spi->dev, spi->irq, NULL,
		id->name, id->driver_data,
		bops);
}

static const struct spi_device_id ad7606_id_table[] = {
	{ "ad7605-4", ID_AD7605_4 },
	{ "ad7606-4", ID_AD7606_4 },
	{ "ad7606-6", ID_AD7606_6 },
	{ "ad7606-8", ID_AD7606_8 },
	{ "ad7606b",  ID_AD7606B },
	{ "ad7616",   ID_AD7616 },
	{}
};
MODULE_DEVICE_TABLE(spi, ad7606_id_table);

static const struct of_device_id ad7606_of_match[] = {
	{.compatible = "adi,ad7606-8" },
	{ },
};
MODULE_DEVICE_TABLE(of, ad7606_of_match);

static struct spi_driver ad7606_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "ad7606",
		.of_match_table = ad7606_of_match,
		.pm = AD7606_PM_OPS,
	},
	.probe = ad7606_spi_probe,
	.id_table = ad7606_id_table,
};
static int __init ad7606_init(void)
{
	printk("start match ad7606\n");
	return spi_register_driver(&ad7606_driver);
}

static void __exit ad7606_exit(void)
{
	spi_unregister_driver(&ad7606_driver);
}

module_init(ad7606_init);
module_exit(ad7606_exit);

MODULE_AUTHOR("Michael Hennerich <michael.hennerich@analog.com>");
MODULE_DESCRIPTION("Analog Devices AD7606 ADC");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(IIO_AD7606);
MODULE_INFO(intree,"Y");