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

 #define GPIOLED_CNT 1 /*设备数个数*/
 #define GPIOLED_NAME   "gpioled"/*名字*/
 #define    LEDOFF  0 /*关灯*/
 #define    LEDON   1 /*开灯*/

 /*gpioled设备结构体，之前有两个结构体分别是寄存器的结构体，另一个是操作文件的结构体这个结构体需要注册到内核中，这里因为
 使用gpio子系统了，所以新加一个gpioled设备结构体，少了一个寄存器的结构体。*/
 struct gpioled_dev{
dev_t devid ;/*设备号*/
struct cdev cdev;/*cdev*/
struct class *class;/*类*/
struct device *device;/*设备*/
int major; /*主设备号*/
int minor;/*次设备号*/
struct device_node *nd; /*设备节点*/
int led_gpio; /*led所使用的GPIO编号*/
int dev_stats ;//使用状态，0设备为使用，》0设备被使用
spinlock_t lock; /* 自旋锁 */
 };

    struct gpioled_dev gpioled;/*led设备这里属于实例化了上面的结构体还没有赋值*/

    /*接下来写下操作文件的函数，也就是操作设备。*/
    /*打开设备，传递给驱动的inode，设备文件，file结构体有个叫做私人变量的成员变量
    一般在open的时候将private_data指向设备结构体*/
    static int led_open(struct inode *inode,struct file * filp)
    {
       unsigned long flags;
        filp->private_data = &gpioled;/*设置私有数据*/
        spin_lock_irqsave(&gpioled.lock,flags);//保存中断状态，禁止本地中断，并获取自旋锁
       if(gpioled.dev_stats){//当设备被使用了才华进入if
        spin_unlock_irqrestore(&gpioled.lock,flags); //解锁
        return -EBUSY;
       }
       gpioled.dev_stats++;//如果设备没有打开过就+1
       spin_unlock_irqrestore(&gpioled.lock,flags);//解锁
       //实际上就是根据stats这个变量来决定开锁还是解锁的，这里和atomic的区别在于关闭了中断
        return 0;
    }

    /*从设备读取数据*/
    static ssize_t led_read(struct file *filp,char __user *buf,size_t cnt,loff_t *offt)
    {
        return 0;	
    }
    /*向设备写数据*/
    static ssize_t led_write(struct file *filp,const char __user  *buf,size_t cnt,loff_t *offt)
    {
        /*其实这个写函数就是我们要操作的了，控制引脚的高低电平就是这里控制的*/
        int retvalue;/*这个就是判断是否写入成功*/
        unsigned char databuf[1];
        unsigned char ledstat;
        struct gpioled_dev *dev = filp->private_data;/*这里相当于把我们传进来的设备结构体赋值给这个局部变量
        因为我们定义的设备结构体是全局变量，当我们赋值的时候希望只在局部里赋值这样影响会小一些，所以这里就搞一个临时的局部变量*/

    retvalue = copy_from_user(databuf,buf,cnt);
    if(retvalue<0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }

    ledstat = databuf[0];/*获取状态值*/
    if(ledstat == LEDON){
        gpio_set_value(dev->led_gpio,0);//这里的函数就是gpio子系统配置gpio的API函数了
    }else if(ledstat == LEDOFF)
    {
        gpio_set_value(dev->led_gpio,1);//通过我们传入的设备结构体的gpio编号来实现控制引脚电平。
    }
return 0;

    }

    /*关闭/释放设备，释放设备应该注销GPIO以及注销操作设备结构体，但是现在还没有提到这些*/
    static int led_release(struct inode *inode,struct file *filp)
    {
        unsigned long flags;
        //本来这里沙也没有的，但是现在需要释放原子变量其实就是计数值，而这个应该是为了好看...
        struct gpioled_dev *dev =filp->private_data;
        //关闭驱动文件的时候释放原子变量
        spin_lock_irqsave(&dev->lock,flags);//这里也需要上锁
        if(dev->dev_stats){
            dev->dev_stats--;//这里其实是必走的。
        }
        spin_unlock_irqrestore(&dev->lock,flags);//解锁
        return 0;
        //需要注意的是上面添加的atomic是为了防止多个线程操作设备。
    }

    /*设备操作函数，把这些操作设备的函数放到一个结构体中，然后将结构体注册到内核里。*/
    static struct file_operations gpioled_fops = {
        .open = led_open,
        .owner = THIS_MODULE,
        .read = led_read,
        .write = led_write,
        .release = led_release,
    };

    /*驱动入口函数，设备操作结构体需要注册到内核中，设备结构体也需要注册到内核中，这些放在驱动入口函数中统一注册*/
    static int __init led_init(void)
    {
        int ret =0;
        const char *str ;
        //但其实上面是在APP操作的时候检验的，在挂载驱动模块的时候应该对其进行初始化，因为挂载只有一次，而操作文件可能有很多次
      spin_lock_init(&gpioled.lock);
        /*设置LED所使用的GPIO*/
        /*到目前为止，我们还没有读取设备数中gpio属性中的值，到底要对哪个引脚操作还不知道*/
        /*1.获取设备节点：gpioled*/
        gpioled.nd = of_find_node_by_path("/gpioled");
        if(gpioled.nd == NULL)
        {
            printk("gpioled node not find!\r\n");
            return -EINVAL;
        }
        /*2.读取status属性*/
        ret = of_property_read_string(gpioled.nd, "status",&str);//读取下status是否是okay，如果不是的话就错误了
        if(ret<0)
        {
            return -EINVAL;
        }
        if(strcmp(str,"okay"))//当不是okay的时候就是1
        {
            return -EINVAL;
        }
        /*3.获取compatible属性值并进行匹配*/
        ret = of_property_read_string(gpioled.nd,"compatible",&str);
        if(ret<0)
        {
            printk("gpioled:Failed to get compatible property\n");
            return -EINVAL;
        }
        if(strcmp(str,"alientek,led")){
            printk("gpioled: Compatible match failed\n");
            return -EINVAL;
        }
        /*4.获取设备书中的gpio属性，得到LED所使用的LED编号，上面只不过是读取下compatible和status属性而已还没到真正的*/
        gpioled.led_gpio = of_get_named_gpio(gpioled.nd,"led-gpio",0);
        if(gpioled.led_gpio<0)
        {
            printk("can't get led-gpio");
            return -EINVAL;
        }
        printk("led-gpio num = %d\r\n",gpioled.led_gpio);//如果获取gpio编号成功的话，就打印出来编号是多少

        /*5.向gpio子系统申请使用GPIO，之前是获取gpio编号，这就是向gpio子系统注册了*/
        ret = gpio_request(gpioled.led_gpio,"LED-GPIO");
        if(ret)
        {
            printk(KERN_ERR "gpioled: Failed to request led-gpio\n");
            return ret;
        }
        /*6.设置PI0为输出，并且输出高电平，默认关闭LED灯*/
        ret = gpio_direction_output(gpioled.led_gpio,1);
        if(ret<0)
        {
            printk("can't set gpio!\r\n");
        }
        /*注册字符设备驱动*/
        /*1.创建设备号*/
        if(gpioled.major){
            /*定义了设备号*/
            gpioled.devid = MKDEV(gpioled.major,0);
            ret = register_chrdev_region(gpioled.devid,GPIOLED_CNT,GPIOLED_NAME);
            if(ret<0)
            {
                pr_err("cannot register %s char driver [ret=%d]\n",GPIOLED_NAME,GPIOLED_CNT);
                goto free_gpio;
            }
            }else{
                /*没有定义设备号*/
                ret = alloc_chrdev_region(&gpioled.devid,0,GPIOLED_CNT,GPIOLED_NAME);/*申请设备号，这就是为什么设备有设备号的原因了*/
                if(ret<0)
                {
                    pr_err("%s Couldn't alloc_chrdev_region,ret=%d\r\n",GPIOLED_NAME,ret);
                    goto free_gpio;
                }
                gpioled.major = MAJOR(gpioled.devid);//申请的是一个16位的设备号，我们要自己分成朱设备号和此设备号
                gpioled.minor = MINOR(gpioled.devid);
            }
            printk("gpioled major=%d,minor=%d\r\n",gpioled.major,gpioled.minor);
        

        /*2.初始化cdev,cdev明明就是设备号，为啥还有个结构体*/
        gpioled.cdev .owner = THIS_MODULE;
        cdev_init(&gpioled.cdev,&gpioled_fops);

        /*3.添加一个cdev*/
        cdev_add(&gpioled.cdev,gpioled.devid,GPIOLED_CNT);
        if(ret<0)
        {
            goto del_unregister;
        }

        /*4.创建类*/
        gpioled.class = class_create(THIS_MODULE,GPIOLED_NAME);
        if(IS_ERR(gpioled.class))
        {
            goto del_cdev;
        }

        /*5.创建设备*/
        gpioled.device = device_create(gpioled.class,NULL,gpioled.devid,NULL,GPIOLED_NAME);
        if(IS_ERR(gpioled.device))
        {
            goto destroy_class;
        }
        return 0;
        destroy_class:
        class_destroy(gpioled.class);
        del_cdev:
        cdev_del(&gpioled.cdev);
        del_unregister:
        unregister_chrdev_region(gpioled.devid,GPIOLED_CNT);
        free_gpio:
        gpio_free(gpioled.led_gpio);
        return -EIO;
    }

    /*驱动出口函数*/
    static void __exit led_exit(void) {
        /*注销字符设备驱动*/
        cdev_del(&gpioled.cdev);//删除设备号
        unregister_chrdev_region(gpioled.devid,GPIOLED_CNT);
        device_destroy(gpioled.class, gpioled.devid);
        class_destroy(gpioled.class);//注释类，不知道这个类有沙用
        gpio_free(gpioled.led_gpio);/*释放GPIO*/
    }
    module_init(led_init);
    module_exit(led_exit);
    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("ALIENTEK");
    MODULE_INFO(INTREE,"Y");