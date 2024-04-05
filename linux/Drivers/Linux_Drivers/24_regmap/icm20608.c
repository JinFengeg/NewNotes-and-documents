#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include "icm20608reg.h"
#include <linux/gpio.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/regmap.h>


#define ICM20608_CNT   1
#define ICM20608_NAME    "icm20608"

struct   icm20608_dev  {//这个结构体其实就是把杂项聚集起来
    struct   spi_device  *spi;//这个结构体是我们自己定义的，不需要注册到内核中。
    dev_t  devid ;//设备号
    struct cdev   cdev ;
    struct  class  *class;
    struct device *device;//设备
    signed int gyro_x_adc;//陀螺仪x轴原始值
    signed int gyro_y_adc;
    signed int gyro_z_adc;
    signed int accel_x_adc;
    signed int accel_y_adc;
    signed int accel_z_adc;
    signed int temp_adc;
    struct regmap *regmap;
    struct regmap_config    regmap_config;
};
static unsigned char icm20608_read_onereg(struct icm20608_dev *dev, u8 reg)
{
	u8 ret;
	unsigned int data;

	ret = regmap_read(dev->regmap, reg, &data);
	return (u8)data;
}
//向icm20608指定寄存器写入指定的值，写一个寄存器
static void icm20608_write_onereg(struct icm20608_dev *dev,u8 reg,u8 value){
   
    regmap_write(dev->regmap, reg, value);
}

//读取ICM20608的数据，读取原始数据，包括三轴陀螺仪/三轴加速计和内部温度
void icm20608_readdata(struct icm20608_dev *dev){
   u8 ret;
    unsigned char data[14];
   ret = regmap_bulk_read(dev->regmap, ICM20_ACCEL_XOUT_H, data,14);
dev->accel_x_adc = (signed short)((data[0] << 8) | data[1]);
 dev->accel_y_adc = (signed short)((data[2] << 8) | data[3]);
 dev->accel_z_adc = (signed short)((data[4] << 8) | data[5]);
 dev->temp_adc = (signed short)((data[6] << 8) | data[7]);
 dev->gyro_x_adc = (signed short)((data[8] << 8) | data[9]);
 dev->gyro_y_adc = (signed short)((data[10] << 8) | data[11]);
 dev->gyro_z_adc = (signed short)((data[12] << 8) | data[13]);
}

//ICM20608内部寄存器初始化函数
void icm20608_reginit(struct icm20608_dev *dev){
    u8 value =0;
    icm20608_write_onereg(dev,ICM20_PWR_MGMT_1,0x80);
    mdelay(50);
    icm20608_write_onereg(dev,ICM20_PWR_MGMT_1,0x01);
    mdelay(50);
    value =icm20608_read_onereg(dev,ICM20_WHO_AM_I);
    printk("ICM20608  ID =%#X\r\n",value);
    icm20608_write_onereg(dev, ICM20_SMPLRT_DIV, 0x00);
 icm20608_write_onereg(dev, ICM20_GYRO_CONFIG, 0x18);
 icm20608_write_onereg(dev, ICM20_ACCEL_CONFIG, 0x18);
 icm20608_write_onereg(dev, ICM20_CONFIG, 0x04);
 icm20608_write_onereg(dev, ICM20_ACCEL_CONFIG2, 0x04);
 icm20608_write_onereg(dev, ICM20_PWR_MGMT_2, 0x00);
 icm20608_write_onereg(dev, ICM20_LP_MODE_CFG, 0x00);
 icm20608_write_onereg(dev, ICM20_FIFO_EN, 0x00);
}
//字符设备驱动
static int icm20608_open(struct inode *inode,struct file*filp){
    return 0;
}

//从设备读取数据
static  ssize_t icm20608_read(struct file *filp,char __user *buf,size_t cnt,loff_t *off){
    signed int data[7];
    long err =0;
    struct cdev *cdev =filp->f_path.dentry->d_inode->i_cdev;
    struct icm20608_dev *dev =container_of(cdev,struct icm20608_dev,cdev);

    icm20608_readdata(dev);
data[0] = dev->gyro_x_adc;
 data[1] = dev->gyro_y_adc;
 data[2] = dev->gyro_z_adc;
 data[3] = dev->accel_x_adc;
 data[4] = dev->accel_y_adc;
 data[5] = dev->accel_z_adc;
 data[6] = dev->temp_adc;
 err =copy_to_user(buf,data,sizeof(data));
 return 0;
 }

 //关闭设备
static int icm20608_release(struct inode *inode,struct file*filp){
    return 0;
}

//icm20608操作函数
static const struct file_operations icm20608_ops ={
    .owner =THIS_MODULE,
    .open =icm20608_open,
    .read =icm20608_read,
    .release =icm20608_release,
};

//probe函数
static int icm20608_probe(struct spi_device *spi){
    int ret;
    struct icm20608_dev *icm20608dev;
    printk("successful match\n");
    //首先是分配内存给上面这个结构体这样其中的spi就有空间了
    icm20608dev =devm_kzalloc(&spi->dev,sizeof(*icm20608dev),GFP_KERNEL);
    if(!icm20608dev)
    return -ENOMEM;

       icm20608dev->regmap_config.reg_bits = 8;
   icm20608dev->regmap_config.val_bits = 8;
   icm20608dev->regmap_config.read_flag_mask = 0x80;
   icm20608dev->regmap = regmap_init_spi(spi, &icm20608dev->regmap_config);
   if (IS_ERR(icm20608dev->regmap)) {
 return PTR_ERR(icm20608dev->regmap);
 }
   
    //注册字符设备驱动
    ret =alloc_chrdev_region(&icm20608dev->devid,0,ICM20608_CNT,ICM20608_NAME);
    if(ret < 0){
        pr_err("%s count alloc_chrdev_region,ret = %d\r\n",ICM20608_NAME,ret);
        goto del_regmap;
    }
    printk("devid =%d",icm20608dev->devid);
    //初始化cdev
    icm20608dev->cdev.owner =THIS_MODULE;
    cdev_init(&icm20608dev->cdev,&icm20608_ops);//操作函数结构体注册到cdev里了。

    //添加cdev
    ret =cdev_add(&icm20608dev->cdev,icm20608dev->devid,ICM20608_CNT);
    if(ret < 0){
        goto del_unregister;
    }
    //4.创建类
    icm20608dev->class =class_create(THIS_MODULE,ICM20608_NAME);
    if(IS_ERR(icm20608dev->class)){
        goto del_cdev;
    }

    //创建设备
    icm20608dev->device =device_create(icm20608dev->class,NULL,icm20608dev->devid,NULL,ICM20608_NAME);
    if(IS_ERR(icm20608dev->device)){
        goto destroy_class;
    }
    icm20608dev->spi = spi;
	
	/*初始化spi_device */
	spi->mode = SPI_MODE_0;	/*MODE0，CPOL=0，CPHA=0*/
	spi_setup(spi);
	
	/* 初始化ICM20608内部寄存器 */
	icm20608_reginit(icm20608dev);	
	/* 保存icm20608dev结构体 */
	spi_set_drvdata(spi, icm20608dev);
 return 0;

    destroy_class:
    device_destroy(icm20608dev->class,icm20608dev->devid);
    del_cdev:
    cdev_del(&icm20608dev->cdev);
    del_unregister:
    unregister_chrdev_region(icm20608dev->devid,ICM20608_CNT);
    del_regmap:
     regmap_exit(icm20608dev->regmap);
    return -EIO;

}

//spi驱动的remove函数
static int icm20608_remove(struct spi_device *spi){
    struct icm20608_dev *icm20608dev =spi_get_drvdata(spi);
    //注销字符设备驱动
    //1.删除cdev
    cdev_del(&icm20608dev->cdev);
    //2.注销设备号
    unregister_chrdev_region(icm20608dev->devid,ICM20608_CNT);
    //3.注销设备
    device_destroy(icm20608dev->class,icm20608dev->devid);
    //注销类
    class_destroy(icm20608dev->class);

    regmap_exit(icm20608dev->regmap);
    return 0;
}

//接下来需要初始化spi_driver
//传统匹配方式ID列表
static const struct spi_device_id icm20608_id[] ={
    {"alientek,icm20608",0},
    {}
};

//设备树匹配列表
static const struct of_device_id icm20608_of_match[] ={
    {.compatible ="alientek,icm20608"},
    {}
};

//SPI驱动结构体
static struct spi_driver icm20608_driver ={
    .probe = icm20608_probe,
    .remove = icm20608_remove,
    .driver ={
        .name = "icm20608",
        .owner =THIS_MODULE,
        .of_match_table =icm20608_of_match,
    },
    .id_table = icm20608_id,
};

//驱动入口函数
static int __init icm20608_init(void){
    return spi_register_driver(&icm20608_driver);
}

//驱动出口函数
static void __exit icm20608_exit(void){
    spi_unregister_driver(&icm20608_driver);
}

module_init(icm20608_init);
module_exit(icm20608_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIENTEK");
MODULE_INFO(intree,"Y");//这行实施森么意思？