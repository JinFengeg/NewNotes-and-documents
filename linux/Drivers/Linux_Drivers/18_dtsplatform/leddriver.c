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
 #define LEDDEV_NAME "dtsplatled" /* 设备名字 */
 #define LEDOFF 0
 #define LEDON 1

 struct leddev_dev{
 dev_t devid; /* 设备号 */
 struct cdev cdev; /* cdev*/
 struct class *class; /* 类 */
 struct device *device; /* 设备 */ 
 struct device_node *node; /* LED 设备节点 */
 int gpio_led; /* LED 灯 GPIO 标号 */
 };

 struct leddev_dev leddev;
 void led_switch(u8 sta)
 {
 if (sta == LEDON )
 gpio_set_value(leddev.gpio_led, 0);
 else if (sta == LEDOFF)
 gpio_set_value(leddev.gpio_led, 1);
 }

 static int led_gpio_init(struct device_node *nd)
 {
 int ret;
 
 /* 从设备树中获取 GPIO */
 leddev.gpio_led = of_get_named_gpio(nd, "led-gpio", 0);
 if(!gpio_is_valid(leddev.gpio_led)) {
 printk(KERN_ERR "leddev: Failed to get led-gpio\n");
 return -EINVAL;
 }
 
 /* 申请使用 GPIO */
 ret = gpio_request(leddev.gpio_led, "LED0");
 if (ret) {
 printk(KERN_ERR "led: Failed to request led-gpio\n");
 return ret;
 }
 
 /* 将 GPIO 设置为输出模式并设置 GPIO 初始电平状态 */
 gpio_direction_output(leddev.gpio_led,1);
 
 return 0;
 }

 static int led_open(struct inode *inode, struct file *filp)
 {
 return 0;
 }

 static ssize_t led_write(struct file *filp, const char __user *buf,
size_t cnt, loff_t *offt)
 {
 int retvalue;
 unsigned char databuf[1];
 unsigned char ledstat;

 retvalue = copy_from_user(databuf, buf, cnt);
 if(retvalue < 0) {
 printk("kernel write failed!\r\n");
 return -EFAULT;
 }
 
 ledstat = databuf[0];
 if (ledstat == LEDON) {
 led_switch(LEDON);
 } else if (ledstat == LEDOFF) {
 led_switch(LEDOFF);
 }
 return 0;
 }

 static struct file_operations led_fops = {
 .owner = THIS_MODULE,
 .open = led_open,
 .write = led_write,
 };

 static int led_probe(struct platform_device *pdev)
 { 
 int ret;
 
 printk("led driver and device was matched!\r\n");
 
 /* 初始化 LED */
 /*由linux将设备节点注册到platform_device中，因此不需要自己
 使用of_前缀API访问设备树了*/
 ret = led_gpio_init(pdev->dev.of_node);//这里能看出来实际上是设备树中的节点信息被内核生成platform_device文件之后传入驱动文件中的。
 if(ret < 0)//匹配成功之后，pdev结构体中的dev.of_node自动指向设备树节点，也就不需要我们自己找节点了。
 return ret;
 
 /* 1、设置设备号 */
 ret = alloc_chrdev_region(&leddev.devid, 0, LEDDEV_CNT,LEDDEV_NAME);
 if(ret < 0) {
 pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n",LEDDEV_NAME, ret);
 goto free_gpio;
 }
 
 /* 2、初始化 cdev */
 leddev.cdev.owner = THIS_MODULE;
 cdev_init(&leddev.cdev, &led_fops);
 
 /* 3、添加一个 cdev */
 ret = cdev_add(&leddev.cdev, leddev.devid, LEDDEV_CNT);
 if(ret < 0)
 goto del_unregister;
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
 free_gpio:
 gpio_free(leddev.gpio_led);
 return -EIO;
 }

 static int led_remove(struct platform_device *dev)
 {
 gpio_set_value(leddev.gpio_led, 1); /* 卸载驱动的时候关闭 LED */
 gpio_free(leddev.gpio_led); /* 注销 GPIO */
 cdev_del(&leddev.cdev); /* 删除 cdev */
 unregister_chrdev_region(leddev.devid, LEDDEV_CNT);
 device_destroy(leddev.class, leddev.devid); /* 注销设备 */
 class_destroy(leddev.class); /* 注销类 */
 return 0;
 }

 //匹配列表
 static const struct of_device_id led_of_match[] ={
    {.compatible ="alientek,led"},
    {}
 };

 MODULE_DEVICE_TABLE(of,led_of_match);

 //platform驱动结构体
 static struct platform_driver led_driver ={
    .driver ={
        .name ="stm32mp1-led",
        .of_match_table =led_of_match,//与设备树匹配，
    },
    .probe =led_probe,
    .remove =led_remove,
 };

 static int __init leddriver_init(void)
 {
    return platform_driver_register(&led_driver);
 }

 static void __exit leddriver_exit(void)
 {
    platform_driver_unregister(&led_driver);
 }

 module_init(leddriver_init);
 module_exit(leddriver_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");