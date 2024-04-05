#include <linux/types.h> 
 #include <linux/kernel.h> 
  #include <linux/delay.h> 
 #include <linux/ide.h> 
  #include <linux/init.h>
  #include <linux/module.h> 
   #include <linux/errno.h> 
    #include <linux/gpio.h> 
     #include <linux/cdev.h>
 #include <linux/device.h>
 #include <linux/of_gpio.h> 
 #include <linux/semaphore.h>
 #include <linux/timer.h>
 #include <linux/irq.h>
 #include <linux/wait.h>
 #include <linux/poll.h>
 #include <linux/fs.h>
 #include <linux/fcntl.h>
  #include <linux/platform_device.h>
 #include <asm/mach/map.h>
  #include <asm/uaccess.h>
 #include <asm/io.h>

 #define LEDDEV_CNT 1 /* 设备号长度 */
 #define LEDDEV_NAME "platled" /* 设备名字 */
 #define LEDOFF 0
 #define LEDON 1

 //因为不使用设备数，所以gpio子系统无法使用，

 /* 映射后的寄存器虚拟地址指针 */
 static void __iomem *MPU_AHB4_PERIPH_RCC_PI;
 static void __iomem *GPIOI_MODER_PI;
 static void __iomem *GPIOI_OTYPER_PI;
 static void __iomem *GPIOI_OSPEEDR_PI;
 static void __iomem *GPIOI_PUPDR_PI;
 static void __iomem *GPIOI_BSRR_PI;

 /* leddev 设备结构体 */
 struct leddev_dev{
 dev_t devid; /* 设备号 */
 struct cdev cdev; /* cdev */
 struct class *class; /* 类 */
 struct device *device; /* 设备 */ 
 };
 
 struct leddev_dev leddev; /* led 设备 */

 void led_switch(u8 sta)
 {
 u32 val = 0;
 if(sta == LEDON) {
 val = readl(GPIOI_BSRR_PI);
 val |= (1 << 16); 
 writel(val, GPIOI_BSRR_PI);
 }else if(sta == LEDOFF) {
 val = readl(GPIOI_BSRR_PI);
 val|= (1 << 0);
 writel(val, GPIOI_BSRR_PI);
 } 
 }

 void led_unmap(void)
 {
 /* 取消映射 */
 iounmap(MPU_AHB4_PERIPH_RCC_PI);
 iounmap(GPIOI_MODER_PI);
 iounmap(GPIOI_OTYPER_PI);
 iounmap(GPIOI_OSPEEDR_PI);
 iounmap(GPIOI_PUPDR_PI);
 iounmap(GPIOI_BSRR_PI);
 }

 static int led_open(struct inode *inode, struct file *filp)
 {
 return 0;
 }

 static ssize_t led_write(struct file *filp, const char __user *buf,size_t cnt, loff_t *offt)
 {
 int retvalue;
 unsigned char databuf[1];
 unsigned char ledstat;
 
 retvalue = copy_from_user(databuf, buf, cnt);
  if(retvalue < 0) {
 printk("kernel write failed!\r\n");
  return -EFAULT;
 }

 ledstat = databuf[0]; /* 获取状态值 */
 if(ledstat == LEDON) {
     led_switch(LEDON); /* 打开 LED 灯 */
}else if(ledstat == LEDOFF) {
 led_switch(LEDOFF); /* 关闭 LED 灯 */
 }

 return 0;
 }

 static struct file_operations led_fops = {
 .owner = THIS_MODULE,
 .open = led_open,
 .write = led_write,
 };
 //接下来就是platform框架的内容了
 //先实现probe函数，其实内部就是传统的字符设备驱动
 static int led_probe(struct platform_device *dev)
 { 
 int i = 0, ret;
 int ressize[6];
 u32 val = 0;
 struct resource *ledsource[6];//获取资源本来是从设备数中，现在要从设备文件的
 printk("led driver and device has matched!\r\n");
 /* 1、获取资源 ，这一步与设备数不同*/
 for (i = 0; i < 6; i++) {/*匹配成功之后
 probe函数的形参会被赋值，这就是从驱动中获取设备的方法
 也是为什么driver的形参是device，此API获得的是resource成员*/
 ledsource[i] = platform_get_resource(dev, IORESOURCE_MEM, i);//首先是获取资源结构体数组，类型是内存地址
  if (!ledsource[i]) {
 dev_err(&dev->dev, "No MEM resource for always on\n");
  return -ENXIO;
 }
 ressize[i] = resource_size(ledsource[i]); //获取存储单元的数量
 } 
 /* 2、初始化 LED */
 /* 寄存器地址映射 */
 MPU_AHB4_PERIPH_RCC_PI = ioremap(ledsource[0]->start,ressize[0]);//这些资源都是寄存器地址
 GPIOI_MODER_PI = ioremap(ledsource[1]->start, ressize[1]);
 GPIOI_OTYPER_PI = ioremap(ledsource[2]->start, ressize[2]);
 GPIOI_OSPEEDR_PI = ioremap(ledsource[3]->start, ressize[3]);
 GPIOI_PUPDR_PI = ioremap(ledsource[4]->start, ressize[4]);
 GPIOI_BSRR_PI = ioremap(ledsource[5]->start, ressize[5]);
 
 /* 3、使能 PI 时钟 */
 val = readl(MPU_AHB4_PERIPH_RCC_PI);
 val &= ~(0X1 << 8); /* 清除以前的设置 */
 val |= (0X1 << 8); /* 设置新值 */
 writel(val, MPU_AHB4_PERIPH_RCC_PI);

 /* 4、设置 PI0 通用的输出模式。*/
 val = readl(GPIOI_MODER_PI);
 val &= ~(0X3 << 0); /* bit0:1 清零 */
 val |= (0X1 << 0); /* bit0:1 设置 01 */
 writel(val, GPIOI_MODER_PI);

 /* 5、设置 PI0 为推挽模式。*/
 val = readl(GPIOI_OTYPER_PI);
 val &= ~(0X1 << 0); /* bit0 清零，设置为上拉*/
 writel(val, GPIOI_OTYPER_PI);

 /* 6、设置 PI0 为高速。*/
 val = readl(GPIOI_OSPEEDR_PI);
 val &= ~(0X3 << 0); /* bit0:1 清零 */
 val |= (0x2 << 0); /* bit0:1 设置为 10 */
 writel(val, GPIOI_OSPEEDR_PI);

 /* 7、设置 PI0 为上拉。*/
 val = readl(GPIOI_PUPDR_PI);
 val &= ~(0X3 << 0); /* bit0:1 清零 */
 val |= (0x1 << 0); /*bit0:1 设置为 01 */
 writel(val,GPIOI_PUPDR_PI);

 /* 8、默认关闭 LED */
 val = readl(GPIOI_BSRR_PI);
 val |= (0x1 << 0);
 writel(val, GPIOI_BSRR_PI);

 /* 注册字符设备驱动 */
 /* 1、申请设备号 */
 ret = alloc_chrdev_region(&leddev.devid, 0, LEDDEV_CNT,LEDDEV_NAME); 
 if(ret < 0)
 goto fail_map;
 
 /* 2、初始化 cdev */
 leddev.cdev.owner = THIS_MODULE;
 cdev_init(&leddev.cdev, &led_fops);
 
 /* 3、添加一个 cdev */
 ret = cdev_add(&leddev.cdev, leddev.devid, LEDDEV_CNT);
 if(ret < 0)
 goto del_unregister;
 
 /* 4、创建类 */
 leddev.class = class_create(THIS_MODULE, LEDDEV_NAME);
 if (IS_ERR(leddev.class)) {
 goto del_cdev;
 }

 /* 5、创建设备 */
 leddev.device = device_create(leddev.class, NULL, leddev.devid,NULL, LEDDEV_NAME);
 if (IS_ERR(leddev.device)) {
 goto destroy_class;
 }
 return 0;

 destroy_class:
 class_destroy(leddev.class);
 del_cdev:
 cdev_del(&leddev.cdev);
 del_unregister:
 unregister_chrdev_region(leddev.devid, LEDDEV_CNT);
 fail_map:
 led_unmap();
 return -EIO;
 }

 static int led_remove(struct platform_device *dev)
 {
    led_unmap();//取消映射
    cdev_del(&leddev.cdev);//删除cdev
    unregister_chrdev_region(leddev.devid, LEDDEV_CNT);
    device_destroy(leddev.class, leddev.devid);
    class_destroy(leddev.class);
    return 0;
 }

 //platform驱动结构体
 static struct platform_driver led_driver ={
    .driver ={
        .name ="stm32mp1-led",//这是为了第四种匹配方式
    },
    .probe =led_probe,
    .remove =led_remove,
 };

 //驱动入口函数
 static int __init leddriver_init(void)
 {
    return platform_driver_register(&led_driver);
 }
 static void __exit leddriver_exit(void){
    platform_driver_unregister(&led_driver);
 }

 module_init(leddriver_init);
 module_exit(leddriver_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");

