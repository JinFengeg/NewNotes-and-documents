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
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define TIMER_CNT 1
#define TIMER_NAME "timer"
#define CLOSE_CMD (_IO(0XEF, 0X1))
#define OPEN_CMD (_IO(0XEF, 0X2))
#define SETPERIOD_CMD (_IO(0XEF, 0X3))
#define LEDON 1
#define LEDOFF 0

// TIMER设备结构体
struct timer_dev
{
    dev_t devid;             //设备号
    struct cdev cdev;        // cdev
    struct class *class;     //类
    struct device *device;  //设备
    int major;               //朱设备号
    int minor;               //此设备号
    struct device_node *nd;  //设备树中的节点
    int led_gpio;            //本次所使用的gpio子系统的编号
    int timeperiod;          //定时周期，单位ms
    struct timer_list timer; //定义了一个定时器
    spinlock_t lock;         //定义自选锁
};
struct timer_dev timerdev; // timer设备
//这次先写驱动入口
static int led_init(void)
{
    int ret;
    const char *str;

    //先从设备数开始获取信息
    // 1.获取设备节点timerdev
    timerdev.nd = of_find_node_by_path("/gpioled");
    if(timerdev.nd == NULL)
    {
        printk("timerdev node not find!\r\n");
        return -EINVAL;
    }
    // 2.读取status属性
    ret = of_property_read_string(timerdev.nd, "status", &str);
    if (ret < 0)
    {
        return -EINVAL;
    }
    if (strcmp(str, "okay"))
    {
        return -EINVAL;
    }
    // 3.获取compatible属性值并进行匹配
    ret = of_property_read_string(timerdev.nd, "compatible", &str);
    if (ret < 0)
    {
        printk("timerdev:Failed to get compatible property\n");
        return -EINVAL;
    }
    if (strcmp(str, "alientek,led"))
    {
        printk("timerdev:Compatible match failed\n");
        return -EINVAL;
    }
    // 4.获取设备树中的gpio属性，得到gpio所使用的gpio编号
    timerdev.led_gpio = of_get_named_gpio(timerdev.nd, "led-gpio", 0);
    if (timerdev.led_gpio < 0)
    {
        printk("can't get led-gpio");
        return -EINVAL;
    }
    // 5.向gpio子系统申请使用GPIO
    ret = gpio_request(timerdev.led_gpio, "led");
    if (ret)
    {
        printk(KERN_ERR "timerdev:Failed to request led-gpio\n");
        return ret;
    }
    //设置PI0为输出，并且输出高电平，默认关闭LED灯
    ret = gpio_direction_output(timerdev.led_gpio, 1);
    if (ret < 0)
    {
        printk("can't set gpio!\r\n");
        return ret;
    }
    return 0;
}

//打开设备
static int timer_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    filp->private_data = &timerdev; //将设备结构体传入内核
    timerdev.timeperiod = 1000;     //默认周期是1s
    ret = led_init();               //初始化LED
    if (ret < 0)
    {
        return ret;
    }
    return 0;
}

//这个是在应用程序中不断设置定时器，不理解为什么这样写
static long timer_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct timer_dev *dev = (struct timer_dev *)filp->private_data;
    int timerperiod;
    unsigned long flags;
    switch (cmd)
    {
    case CLOSE_CMD:
        del_timer_sync(&dev->timer);
        break;
    case OPEN_CMD:
        spin_lock_irqsave(&dev->lock, flags);
        timerperiod = dev->timeperiod; //临界区，就是把1000节拍数传进局部变量
        spin_unlock_irqrestore(&dev->lock, flags);
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(timerperiod));
        break;
    case SETPERIOD_CMD: //设置定时器周期
        spin_lock_irqsave(&dev->lock, flags);
        dev->timeperiod = arg;
        spin_unlock_irqrestore(&dev->lock, flags);
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(arg));
        break;
    default:
        break;
    }
    return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
    struct timer_dev *dev = filp->private_data;
    gpio_set_value(dev->led_gpio, 1); // app结束的时候关闭led
    gpio_free(dev->led_gpio);         //释放led
    del_timer_sync(&dev->timer);      //同步多个处理器关闭定时器
    return 0;
}
//设备操作函数
static struct file_operations timer_fops = {
    .owner = THIS_MODULE,
    .open = timer_open,
    .unlocked_ioctl = timer_unlocked_ioctl, //看起来在应用程序中也有对应的函数，所以看不懂也是正常的。
    .release = led_release,
};

//定时器回调函数
void timer_function(struct timer_list *arg)
{
    /*from_timer是个宏，可以根据结构体的成员地址获取到这个结构体的首地质
    第一个参数表示结构体，第二个参数表示第一个参数里的一个成员，第三个
    参数表示第二个参数的类型，返回值是第一个参数的首地址*/
    struct timer_dev *dev = from_timer(dev, arg, timer);
    static int sta = 1;
    int timerperiod;
    unsigned long flags;
    sta = !sta; //每次取反，实现led灯反转，也就是每中断一次就亮灭一次
    gpio_set_value(dev->led_gpio, sta);
    //重启定时器
    spin_lock_irqsave(&dev->lock, flags);
    timerperiod = dev->timeperiod; //咋就这么帕1000被换了呢
    spin_unlock_irqrestore(&dev->lock, flags);
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timeperiod)); //你都不用这个局部变量，为啥还要设个林街区
}

//驱动入口函数
static int __init timer_init(void)
{
    int ret;
    //因为led初始化放到open函数中了，所以这里直接注册设备就可以了
    spin_lock_init(&timerdev.lock);

    //注册字符设备驱动
    // 1.创建设备号
    if (timerdev.major)
    {
        //如果注册了的话
        timerdev.devid = MKDEV(timerdev.major, 0);
        ret = register_chrdev_region(timerdev.devid, TIMER_CNT, TIMER_NAME);
        if (ret < 0)
        {
            pr_err("can't register %s char driver [ret=%d]\n", TIMER_NAME, TIMER_CNT);
            return -EIO;
        }
    }
    else
    {
        ret = alloc_chrdev_region(&timerdev.devid, 0, TIMER_CNT, TIMER_NAME); //申请设备号
           if (ret < 0)
        {
            pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n",TIMER_NAME, ret);
            return -EIO;
        }
        //分配完设备号之后换成朱设备号和此设备号
        timerdev.major = MAJOR(timerdev.devid);
        timerdev.minor = MINOR(timerdev.devid);
    }
    printk("timerdev major=%d,minor=%d\r\n",timerdev.major,timerdev.minor);
    //初始化cdev
    timerdev.cdev.owner = THIS_MODULE;cdev_init(&timerdev.cdev,&timer_fops);
    //添加cdev
    cdev_add(&timerdev.cdev,timerdev.devid, TIMER_CNT);
    if(ret<0)
    {
        goto del_unregister;
    }
    //创建类
    timerdev.class = class_create(THIS_MODULE,TIMER_NAME);
    if(IS_ERR(timerdev.class)){
        goto del_cdev;
    }
    timerdev.device = device_create(timerdev.class, NULL,timerdev.devid, NULL, TIMER_NAME);
if (IS_ERR(timerdev.device)) {
 goto destroy_class;
 }
 timer_setup(&timerdev.timer, timer_function, 0);
 return 0;
 destroy_class:
 device_destroy(timerdev.class, timerdev.devid);
 del_cdev:
 cdev_del(&timerdev.cdev);
 del_unregister:
 unregister_chrdev_region(timerdev.devid, TIMER_CNT);
 return -EIO;
}

static void __exit timer_exit(void)
 {
 del_timer_sync(&timerdev.timer); /* 删除 timer */
 #if 0
 del_timer(&timerdev.tiemr);
 #endif

 /* 注销字符设备驱动 */
 cdev_del(&timerdev.cdev); /* 删除 cdev */
 unregister_chrdev_region(timerdev.devid, TIMER_CNT);

 device_destroy(timerdev.class, timerdev.devid);
 class_destroy(timerdev.class);
 }

 module_init(timer_init);
 module_exit(timer_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");