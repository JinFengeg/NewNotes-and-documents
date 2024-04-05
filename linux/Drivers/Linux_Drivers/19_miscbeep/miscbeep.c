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
 #include <linux/platform_device.h>
 #include <linux/miscdevice.h>
 #include <asm/mach/map.h>
 #include <asm/uaccess.h>
 #include <asm/io.h>

 #define MISCBEEP_NAME "miscbeep"
 #define MISCBEEP_MINOR 144//这里是我们自己提高的子设备号，朱设备号都是10
 #define BEEPOFF  0
 #define BEEPON 1

 //miscbeep设备结构体，这是我们自己设置的结构体，就是把用到的数据整理起来。
 struct miscbeep_dev {
    dev_t devid;//设备号
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int beep_gpio;//要申请的GPIO编号
 };

 struct miscbeep_dev     miscbeep;//beep设备

 static int beep_gpio_init(struct device_node *nd){
    //这里是引脚初始化函数，传入的参数是指向设备树节点的指针。
    int ret;
    //从设备树中获取GPIO
    miscbeep.beep_gpio = of_get_named_gpio(nd,"beep-gpio",0);//此函数返回值就是GPIO编号了
    if(!gpio_is_valid(miscbeep.beep_gpio)){
        printk("miscbeep:Failed to get beep-gpio\n");
        return -EINVAL;
    }
    //申请使用GPIO，之前只是获取编号而已
    ret =gpio_request(miscbeep.beep_gpio,"beep");
    if(ret){
        printk("beep:failed to request beep-gpio\n");
        return ret;
    }
    //将引脚设置为输出模式，并设置默认电平
    //其实这里有点问题，设备树中都已经写明白了，这里是重新定义了
    gpio_direction_output(miscbeep.beep_gpio,1);
    return 0;
 }

 //打开文集
 static int miscbeep_open(struct inode *inode,struct file *filp)
 {
    return 0;
 }

 //写函数
 static ssize_t miscbeep_write(struct file *filp,const char __user *buf,size_t cnt,loff_t *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned char beepstat;
    retvalue =copy_from_user(databuf,buf,cnt);
    if(retvalue<0){
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }
    beepstat =databuf[0];
    //这里就是根据传入的数据来决定高低电平
    if(beepstat ==BEEPON){
        gpio_set_value(miscbeep.beep_gpio,0);
    }else if(beepstat ==BEEPOFF){
        gpio_set_value(miscbeep.beep_gpio,1);
    }
    return 0;
}

//设备操作结构体
static struct file_operations miscbeep_fops ={
    .owner =THIS_MODULE,
    .open =miscbeep_open,
    .write =miscbeep_write,
};

//MISC设备结构体，之前是把MISC设备结构体和MISCBEEP结构体放在一起
static struct miscdevice beep_miscdev ={
    .minor =MISCBEEP_MINOR,//不需要我们自己申请设备号了
    .name =MISCBEEP_NAME,
    .fops =&miscbeep_fops,//不需要我们额外注册设备操作结构体了
};

//probe函数
static int miscbeep_probe(struct platform_device *pdev) {
    int ret =0;
    printk("beep driver and device was matched!\r\n");
    //初始化BEEP
    ret =beep_gpio_init(pdev->dev.of_node);//当匹配成果后参数就指向设备树节点了
    if(ret<0){
        return ret;
    }
    ret =misc_register(&beep_miscdev);//这里我们不需要自己注册字符设备驱动，只需要注册misc设备
    if(ret<0){
        printk("misc device register failed!\r\n");
        goto free_gpio;
    }
    return 0;
    free_gpio:
    gpio_free(miscbeep.beep_gpio);
    return -EINVAL;
}

//remove函数
static int miscbeep_remove(struct platform_device *dev)
{
    //注销设备的时候先关闭BEEP
    gpio_set_value(miscbeep.beep_gpio,1);
    //之前申请了GPIO编号，现在要释放
    gpio_free(miscbeep.beep_gpio);
    //注销misc设备
    misc_deregister(&beep_miscdev);
    return 0;
}

//匹配列表，告诉驱动我们要的是什么设备
static const struct of_device_id beep_of_match[] ={
    {.compatible = "alientek,beep"},
    {}
};

//接下来就是platform驱动结构体，第一种和第四中匹配方式，以及匹配成功之后和注销的时候要那些函数
static struct platform_driver beep_driver ={
    .driver ={
        .name ="stm32mp1-beep",//platform下的驱动名词同时也是第四种匹配方式
        .of_match_table =beep_of_match,//设备树匹配队列
    },
    .probe =miscbeep_probe,
    .remove =miscbeep_remove,
};

static int __init miscbeep_init(void) {
    return platform_driver_register(&beep_driver);
}

static void __exit miscbeep_exit(void) {
    platform_driver_unregister(&beep_driver);
}

module_init(miscbeep_init);
 module_exit(miscbeep_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");