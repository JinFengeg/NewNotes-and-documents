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

 #define AD7606DEV_CNT 1 /* 设备号长度 */
 #define AD7606DEV_NAME "ad7606" /* 设备名字 */

 struct ad7606dev_dev{
 dev_t devid; /* 设备号 */
 struct cdev cdev; /* cdev*/
 struct class *class; /* 类 */
 struct device *device; /* 设备 */ 
 struct device_node *node; /* ad7606 设备节点 */
 int gpio_PRESS01; /* ad7606 灯 GPIO 标号 */
int gpio_PRESS02; 
int gpio_PRESS03; 
int gpio_PRESS04; 
 int gpio_BUSY; 
  int gpio_RESET; 
   int gpio_CONVST; 
   int gpio_RANGE; 
   int gpio_OS1; 
   int gpio_OS2; 

   int gpio_OS0; 

 };

  struct ad7606dev_dev ad7606dev;

  static int ad7606_gpio_init(struct device_node *nd)
  {
    int ret;
 
 /* 从设备树中获取 GPIO */
 ad7606dev.gpio_PRESS01 = of_get_named_gpio(nd, "press01-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_PRESS01)) {
 printk(KERN_ERR "ad7606dev: Failed to get press01-gpio\n");
 return -EINVAL;
 }
 
 ad7606dev.gpio_PRESS02 = of_get_named_gpio(nd, "press02-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_PRESS02)) {
 printk(KERN_ERR "ad7606dev: Failed to get press02-gpio\n");
 return -EINVAL;
 }

 ad7606dev.gpio_PRESS03 = of_get_named_gpio(nd, "press03-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_PRESS03)) {
 printk(KERN_ERR "ad7606dev: Failed to get press03-gpio\n");
 return -EINVAL;
 }

 ad7606dev.gpio_PRESS04 = of_get_named_gpio(nd, "press04-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_PRESS04)) {
 printk(KERN_ERR "ad7606dev: Failed to get press04-gpio\n");
 return -EINVAL;
 }

 ad7606dev.gpio_BUSY = of_get_named_gpio(nd, "busy-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_BUSY)) {
 printk(KERN_ERR "ad7606dev: Failed to get gpio_BUSY-gpio\n");
 return -EINVAL;
 }

 ad7606dev.gpio_CONVST = of_get_named_gpio(nd, "convst-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_CONVST)) {
 printk(KERN_ERR "ad7606dev: Failed to get gpio_CONVST-gpio\n");
 return -EINVAL;
 }

 ad7606dev.gpio_OS1 = of_get_named_gpio(nd, "os1-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_OS1)) {
 printk(KERN_ERR "ad7606dev: Failed to get gpio_OS1-gpio\n");
 return -EINVAL;
 }

 ad7606dev.gpio_OS2 = of_get_named_gpio(nd, "os2-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_OS2)) {
 printk(KERN_ERR "ad7606dev: Failed to get gpio_OS2-gpio\n");
 return -EINVAL;
 }

 ad7606dev.gpio_RANGE = of_get_named_gpio(nd, "range-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_RANGE)) {
 printk(KERN_ERR "ad7606dev: Failed to get gpio_RANGE-gpio\n");
 return -EINVAL;
 }

 ad7606dev.gpio_RESET = of_get_named_gpio(nd, "reset-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_RESET)) {
 printk(KERN_ERR "ad7606dev: Failed to get gpio_RESET-gpio\n");
 return -EINVAL;
 }

  ad7606dev.gpio_CS = of_get_named_gpio(nd, "cs-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_CS)) {
 printk(KERN_ERR "ad7606dev: Failed to get gpio_CS-gpio\n");
 return -EINVAL;
 }

  ad7606dev.gpio_OS0 = of_get_named_gpio(nd, "os0-gpio", 0);
 if(!gpio_is_valid(ad7606dev.gpio_OS0)) {
 printk(KERN_ERR "ad7606dev: Failed to get gpio_OS0-gpio\n");
 return -EINVAL;
 }


 /* 申请使用 GPIO */
 ret = gpio_request(ad7606dev.gpio_PRESS01, "PRESS01");
 if (ret) {
 printk(KERN_ERR "ad7606: Faiad7606 to request press01-gpio\n");
 return ret;
 }

ret = gpio_request(ad7606dev.gpio_PRESS02, "PRESS02");
 if (ret) {
 printk(KERN_ERR "ad7606: Faiad7606 to request press02-gpio\n");
 return ret;
 }

 ret = gpio_request(ad7606dev.gpio_PRESS03, "PRESS03");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request press03-gpio\n");
 return ret;
 }

ret = gpio_request(ad7606dev.gpio_PRESS04, "PRESS04");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request press04-gpio\n");
 return ret;
 }

 ret = gpio_request(ad7606dev.gpio_BUSY, "BUSY");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request busy-gpio\n");
 return ret;
 }

 ret = gpio_request(ad7606dev.gpio_CONVST, "convst");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request convst-gpio\n");
 return ret;
 }

 ret = gpio_request(ad7606dev.gpio_RANGE, "range");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request range-gpio\n");
 return ret;
 }

 ret = gpio_request(ad7606dev.gpio_OS1, "OS1");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request os1-gpio\n");
 return ret;
 }

 ret = gpio_request(ad7606dev.gpio_RESET, "reset");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request reset-gpio\n");
 return ret;
 }

 ret = gpio_request(ad7606dev.gpio_OS2, "os2");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request os2-gpio\n");
 return ret;
 }

 ret = gpio_request(ad7606dev.gpio_OS0, "os0");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request os0-gpio\n");
 return ret;
 }
 ret = gpio_request(ad7606dev.gpio_CS, "cs");
 if (ret) {
 printk(KERN_ERR "ad7606: Failed to request cs-gpio\n");
 return ret;
 }
 
 /* 将 GPIO 设置为输出模式并设置 GPIO 初始电平状态 */
 gpio_direction_output(ad7606dev.gpio_PRESS01,1);
 gpio_direction_output(ad7606dev.gpio_PRESS02,1);

 ret = gpio_direction_input(ad7606dev.gpio_BUSY);
  if(ret < 0) {
 printk("can't set gpio!\r\n");
 return ret;
 }
  gpio_direction_output(ad7606dev.gpio_OS0,0);
 gpio_direction_output(ad7606dev.gpio_CS,1);
 gpio_direction_output(ad7606dev.gpio_CONVST,1);
gpio_direction_output(ad7606dev.gpio_RESET,0);
gpio_direction_output(ad7606dev.gpio_RANGE,0);
gpio_direction_output(ad7606dev.gpio_OS1,0);
gpio_direction_output(ad7606dev.gpio_OS2,0);
gpio_direction_output(ad7606dev.gpio_PRESS03,1);
gpio_direction_output(ad7606dev.gpio_PRESS04,1);

 
 return 0;
  }

void ad7606dev_os_set(int mode){
  switch(mode){
    case 0: 
    gpio_set_value(ad7606dev.gpio_OS0, 0);
    gpio_set_value(ad7606dev.gpio_OS1, 0);
    gpio_set_value(ad7606dev.gpio_OS2, 0);
    break;
    case 1: 
    gpio_set_value(ad7606dev.gpio_OS0, 1);
    gpio_set_value(ad7606dev.gpio_OS1, 0);
    gpio_set_value(ad7606dev.gpio_OS2, 0);
    break;
    case 2: 
    gpio_set_value(ad7606dev.gpio_OS0, 0);
    gpio_set_value(ad7606dev.gpio_OS1, 1);
    gpio_set_value(ad7606dev.gpio_OS2, 0);
    break;
        case 3: 
    gpio_set_value(ad7606dev.gpio_OS0, 1);
    gpio_set_value(ad7606dev.gpio_OS1, 1);
    gpio_set_value(ad7606dev.gpio_OS2, 0);
    break;
        case 4: 
    gpio_set_value(ad7606dev.gpio_OS0, 0);
    gpio_set_value(ad7606dev.gpio_OS1, 0);
    gpio_set_value(ad7606dev.gpio_OS2, 1);
    break;
        case 5: 
    gpio_set_value(ad7606dev.gpio_OS0, 1);
    gpio_set_value(ad7606dev.gpio_OS1, 0);
    gpio_set_value(ad7606dev.gpio_OS2, 1);
    break;
        case 6: 
    gpio_set_value(ad7606dev.gpio_OS0, 0);
    gpio_set_value(ad7606dev.gpio_OS1, 1);
    gpio_set_value(ad7606dev.gpio_OS2, 1);
    break;
    default:
    gpio_set_value(ad7606dev.gpio_OS0, 0);
    gpio_set_value(ad7606dev.gpio_OS1, 0);
    gpio_set_value(ad7606dev.gpio_OS2, 0);
    break;
  }
}

void ad7606dev_range_set(int range) {
switch (range) {
  case 0: 
      gpio_set_value(ad7606dev.gpio_RANGE, 0);
break;
  case 1: 
      gpio_set_value(ad7606dev.gpio_RANGE, 1);
break;
default: 
      gpio_set_value(ad7606dev.gpio_RANGE, 0);
break;
}

}

void ad7606dev_reset(void){
  gpio_set_value(ad7606dev.gpio_CS,1);
  gpio_set_value(ad7606dev.gpio_CONVST,1);
  gpio_set_value(ad7606dev.gpio_RESET,0);
  ndelay(100);
  gpio_set_value(ad7606dev.gpio_RESET,1);
  gpio_set_value(ad7606dev.gpio_RESET,0);
}

void ad7606dev_CVT(void){
  gpio_set_value(ad7606dev.gpio_CONVST,0);
  ndelay(30);
  gpio_set_value(ad7606dev.gpio_CONVST,1);

}


static int ad7606_open(struct inode *inode, struct file *filp)
 {
 return 0;
 }

 static ssize_t ad7606_read(struct file *filp,char __user *buf,size_t cnt ,loff_t *offt)
 {
   int ret = 0;

  ret =gpio_get_value(ad7606dev.gpio_BUSY);
 if(ret <0)
 {
   printk("ad7606_gpio_get_value failing");
 }else{
   printk("ad7606_gpio_get_value =%d",ret);
 }
   return 0;
 }

 static struct file_operations ad7606_fops = {
 .owner = THIS_MODULE,
 .open = ad7606_open,
 .read = ad7606_read,
 };

 //释放引脚函数
 void GPIO_FREE(void){
  gpio_free(ad7606dev.gpio_PRESS01); /* 注销 GPIO */
  gpio_free(ad7606dev.gpio_PRESS02);
 gpio_free(ad7606dev.gpio_PRESS03);
 gpio_free(ad7606dev.gpio_PRESS04);
 gpio_free(ad7606dev.gpio_BUSY);
 gpio_free(ad7606dev.gpio_RESET);
 gpio_free(ad7606dev.gpio_CONVST);
 gpio_free(ad7606dev.gpio_RANGE);
 gpio_free(ad7606dev.gpio_OS1);
 gpio_free(ad7606dev.gpio_OS2);
  gpio_free(ad7606dev.gpio_OS0);
 gpio_free(ad7606dev.gpio_CS);
 }

 static int ad7606_probe(struct platform_device *pdev)
 { 
 int ret;
 
 printk("ad7606 driver and device was matched!\r\n");
 
 /* 初始化 ad7606 */
 ret = ad7606_gpio_init(pdev->dev.of_node);//这里能看出来实际上是设备树中的节点信息被内核生成platform_device文件之后传入驱动文件中的。
 if(ret < 0)//匹配成功之后，pdev结构体中的dev.of_node自动指向设备树节点，也就不需要我们自己找节点了。
 return ret;
//ad7606_gpio_set(ad7606dev,1);
 
 /* 1、设置设备号 */
 ret = alloc_chrdev_region(&ad7606dev.devid, 0, AD7606DEV_CNT,AD7606DEV_NAME);
 if(ret < 0) {
 pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n",AD7606DEV_NAME, ret);
 goto free_gpio;
 }
 
 /* 2、初始化 cdev */
 ad7606dev.cdev.owner = THIS_MODULE;
 cdev_init(&ad7606dev.cdev, &ad7606_fops);
 
 /* 3、添加一个 cdev */
 ret = cdev_add(&ad7606dev.cdev, ad7606dev.devid, AD7606DEV_CNT);
 if(ret < 0)
 goto del_unregister;
 ad7606dev.class = class_create(THIS_MODULE, AD7606DEV_NAME);
 if (IS_ERR(ad7606dev.class)) {
 goto del_cdev;
 }

 /* 5、创建设备 */
 ad7606dev.device = device_create(ad7606dev.class, NULL, ad7606dev.devid,NULL, AD7606DEV_NAME);
 if (IS_ERR(ad7606dev.device)) {
 goto destroy_class;
 }
 
 return 0;
 destroy_class:
 class_destroy(ad7606dev.class);
 del_cdev:
 cdev_del(&ad7606dev.cdev);
 del_unregister:
 unregister_chrdev_region(ad7606dev.devid, AD7606DEV_CNT);
 free_gpio:
 GPIO_FREE();
 return -EIO;
 }

 static int ad7606_remove(struct platform_device *dev)
 {
 gpio_set_value(ad7606dev.gpio_PRESS01, 1); /* 卸载驱动的时候关闭 LED */
 gpio_set_value(ad7606dev.gpio_PRESS02, 1);
 gpio_set_value(ad7606dev.gpio_PRESS03, 1);
 gpio_set_value(ad7606dev.gpio_PRESS04, 1);
 gpio_set_value(ad7606dev.gpio_CONVST, 1);
 gpio_set_value(ad7606dev.gpio_CS, 1);
 gpio_set_value(ad7606dev.gpio_OS0, 0);
 gpio_set_value(ad7606dev.gpio_RANGE, 0);
 gpio_set_value(ad7606dev.gpio_RESET, 0);
 gpio_set_value(ad7606dev.gpio_OS1, 0);
 gpio_set_value(ad7606dev.gpio_OS2, 0);
 
  GPIO_FREE();


 cdev_del(&ad7606dev.cdev); /* 删除 cdev */
 unregister_chrdev_region(ad7606dev.devid, AD7606DEV_CNT);
 device_destroy(ad7606dev.class, ad7606dev.devid); /* 注销设备 */
 class_destroy(ad7606dev.class); /* 注销类 */
 return 0;
 }

 //匹配列表
 static const struct of_device_id ad7606_of_match[] ={
    {.compatible ="ad7606,gpio"},
    {}
 };

 MODULE_DEVICE_TABLE(of,ad7606_of_match);

 //platform驱动结构体
 static struct platform_driver ad7606_driver ={
    .driver ={
        .name ="stm32mp1-ad7606",
        .of_match_table =ad7606_of_match,
    },
    .probe =ad7606_probe,
    .remove =ad7606_remove,
 };

 static int __init ad7606driver_init(void)
 {
    return platform_driver_register(&ad7606_driver);
 }

 static void __exit ad7606driver_exit(void)
 {
    platform_driver_unregister(&ad7606_driver);
 }

 module_init(ad7606driver_init);
 module_exit(ad7606driver_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");