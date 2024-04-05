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
 #include <linux/of.h>
 #include <linux/of_address.h>
 #include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
 #include <asm/io.h>

 #define BEEP_CNT 1 /* 设备号个数 */
 #define BEEP_NAME "beep" /* 名字 */
 #define BEEPOFF 0 /* 关蜂鸣器 */
 #define BEEPON 1 /* 开蜂鸣器 */
 /*新字符设备驱动才有的*/
 struct beep_dev{
    dev_t devid; /*设备号*/
    struct cdev cdev;/*cdev*/
    struct class *class; /*类*/
    struct device *device; /*设备*/
    int major; /*主设备号*/
    int minor; /*此设备号*/
    struct  device_node *nd;/*使用设备数时才有的设备节点*/
    int beep_gpio;
 };

 struct beep_dev beep;/*实例化一个beep设备*/

 /*打开设备*/
 static int led_open(struct inode *inode,struct file *filp)
 {
    filp->private_data = &beep;/*新字符设备驱动才有的*/
    return 0;
 }

 /*从设备读取数据*/
 static ssize_t led_read(struct file *filp,char __user * buf,size_t cnt,loff_t *offt)
 {
    return 0;
 }

 /*向设备写数据*/
 static ssize_t led_write(struct file *filp,const char __user *buf,size_t cnt,loff_t *offt)
 {
    int retvalue;
    unsigned char databuf[1];
    unsigned char ledstat;
    struct beep_dev * dev = filp->private_data;

    retvalue = copy_from_user(databuf,buf,cnt);
    if(retvalue <0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }
    ledstat = databuf[0];
    if(ledstat ==BEEPON)
    {
        gpio_set_value(dev->beep_gpio,0);/*这里就是dev其实就是指向设备结构体的指针*/
    }else if(ledstat == BEEPOFF){
        gpio_set_value(dev->beep_gpio,1);
    }
    return 0;
 }

 //关闭设备
 static int led_release(struct inode *inode,struct file *filp)
 {
    return 0;
 }

 //设备操作结构体
 static struct file_operations beep_fops ={
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
 };

 //驱动入口函数
 static int __init led_init(void)
 {
    int ret = 0;
    const char *str;
    //设置LED所使用的GPIO
    //1.获取设备节点
    beep.nd = of_find_node_by_path("/beep");
    if(beep.nd == NULL)
    {
        printk("beep node not find!\r\n");
        return -EINVAL;
    }

    //2.读取status属性
    ret = of_property_read_string(beep.nd, "status",&str);
    if(ret<0)
    return -EINVAL;
    if(strcmp(str, "okay"))
    return -EINVAL;

    //3.获取compatible属性值并进行匹配
    ret = of_property_read_string(beep.nd, "compatible",&str);
    if(ret<0){
        printk("beep:Failed to get compatible property\n");
        return -EINVAL;
    }

    if(strcmp(str, "alientek,beep")){
        printk("beep:Compatible match failed\n");
        return -EINVAL;
    }

    //4.获取设备树中的gpio属性，得到BEEP所使用的beep编号
    beep.beep_gpio = of_get_named_gpio(beep.nd,"beep-gpio",0);
    if(beep.beep_gpio<0)
    {
        printk("cant get beep-gpio");
        return -EINVAL;
    }
    printk("beep-gpio num =%d\r\n",beep.beep_gpio);
    //5.gpio子系统申请使用gpio
    ret = gpio_request(beep.beep_gpio,"BEEP-GPIO");
    if(ret){
        printk(KERN_ERR "beep:Failed to request beep-gpio\n");
        return ret;
    }
    //6.设置pc7为输出，并且输出高电平，默认关闭BEEP
    ret = gpio_direction_output(beep.beep_gpio,1);
    if(ret<0)
    {
        printk("cant set gpio!\r\n");
    }

    /*初始化gpio完成，接下来注册字符设备驱动*/
    //1.创建设备号
    if(beep.major){
        //如果此时已经定义了设备号
        beep.devid = MKDEV(beep.major,0);
        ret = register_chrdev_region(beep.devid, BEEP_CNT,BEEP_NAME);
        if(ret < 0) { 
            pr_err("cannot register %s char driver [ret=%d]\n",BEEP_NAME, BEEP_CNT);
             goto free_gpio;
 }
    }else{
        //没有定义设备号
        ret = alloc_chrdev_region(&beep.devid, 0, BEEP_CNT,BEEP_NAME); /* 申请设备号 */
        if(ret < 0) {
 pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n",BEEP_NAME, ret);
 goto free_gpio;
    }
    beep.major = MAJOR(beep.devid); /* 获取分配号的主设备号 */
 beep.minor = MINOR(beep.devid); /* 获取分配号的次设备号 */
 }
 printk("beep major=%d,minor=%d\r\n",beep.major, beep.minor);
 /* 2、初始化 cdev */
 beep.cdev.owner = THIS_MODULE;
 cdev_init(&beep.cdev, &beep_fops);
 
 /* 3、添加一个 cdev */
 cdev_add(&beep.cdev, beep.devid, BEEP_CNT);
 if(ret < 0)
 goto del_unregister;
 
 /* 4、创建类 */
 beep.class = class_create(THIS_MODULE, BEEP_NAME);
 if (IS_ERR(beep.class)) {
 goto del_cdev;
 }
 /* 5、创建设备 */
 beep.device = device_create(beep.class, NULL, beep.devid, NULL,BEEP_NAME);
 if (IS_ERR(beep.device)) {
 goto destroy_class;
 }
 return 0;
 
 destroy_class:
 class_destroy(beep.class); 
 del_cdev:
 cdev_del(&beep.cdev);
 del_unregister:
 unregister_chrdev_region(beep.devid, BEEP_CNT);
 free_gpio:
 gpio_free(beep.beep_gpio);
 return -EIO;
 }

 //驱动出口函数
 static void __exit led_exit(void)
 {
 /* 注销字符设备驱动 */
 cdev_del(&beep.cdev); /* 删除 cdev */
 unregister_chrdev_region(beep.devid, BEEP_CNT); /* 注销设备号 */
 device_destroy(beep.class, beep.devid); /* 注销设备 */
 class_destroy(beep.class); /* 注销类 */
 gpio_free(beep.beep_gpio); /* 释放 GPIO */
 }

 module_init(led_init);
 module_exit(led_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");