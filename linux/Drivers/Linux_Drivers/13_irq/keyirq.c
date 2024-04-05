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
 #include <linux/of_irq.h>
 #include <linux/irq.h>
 #include <asm/mach/map.h>
 #include <asm/uaccess.h>
 #include <asm/io.h>

 #define KEY_CNT 1 //设备号数量
 #define KEY_NAME  "key"  //名字

 //定义按键状态
 enum key_status{
    KEY_PRESS =0,//按键按下
    KEY_RELEASE,//按键松开
    KEY_KEEP,//按键状态保持
 };

 //key设备结构体
 struct key_dev{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    int key_gpio;//设备编号
    struct timer_list timer;//按键值
    int irq_num;//中断号
    spinlock_t spinlock;//自旋锁
 };

 static struct key_dev key;//按键设备
 static int status = KEY_KEEP;//按键状态
//中断服务函数，下面是回调函数
 static irqreturn_t key_interrupt(int irq,void *dev_id) {
mod_timer(&key.timer,jiffies +msecs_to_jiffies(15));//按键防抖处理，开启定时器延时15ms，这是重新激活定时器实现每隔15ms就中断一次
return IRQ_HANDLED;
 }//此函数有点像中断里嵌套中断

 //初始化IO，包括设备数到配置IO口。
 static int key_parse_dt(void)
 {
    int ret ;
    const char *str;

    //设置LED所使用的GPIO
    //1.获取设备节点key
    key.nd = of_find_node_by_path("/key");//第一步是获取节点
    if(key.nd == NULL){
        printk("key node not find!\r\n");
        return -EINVAL;
    }
    //第二步是对比属性了
    ret =of_property_read_string(key.nd,"status",&str);
    if(ret <0)
    return -EINVAL;
    if(strcmp(str,"okay"))
    return -EINVAL;
    //第三步，获取compatible属性值并进行匹配
    ret = of_property_read_string(key.nd,"compatible",&str);
    if(ret<0){
        printk("key:Failed to get compatible property\n");
        return -EINVAL;
    }
    if(strcmp(str,"alientek,key")){
        printk("key/:Compatible match failed\n");
        return -EINVAL;
    }
    //走到这里说明这个节点时可以用的，接下来才是获取信息
    key.key_gpio = of_get_named_gpio(key.nd,"key-gpio",0);
    //要获取GPIO编号，
    if(key.key_gpio<0){
        printk("can't get key-gpio\n");
        return -EINVAL;
    }
    //5.获取GPIO对应的中断号
    key.irq_num =irq_of_parse_and_map(key.nd,0);
    if(!key.irq_num){
        return -EINVAL;
    }
    printk("key-gpio num =%d\r\n",key.key_gpio);
    return 0;
    //gpio口还没有配置，只是获取了gpio编号和中断号而已。
 }

 //接下来就是gpio初始化了
 static int key_gpio_init(void){
    int ret;
    unsigned long irq_flags;
    ret = gpio_request(key.key_gpio,"KEY0");//之前只是获取GPIO编号，现在是要使用这个GPIO编号了
//得先获取gpio编号，这是gpio子系统给出的，然后是告诉他我们要使用这个编号
    if(ret){
        printk(KERN_ERR "key:Failed to request key-gpio\n");
        return ret;
    }
    //将GPIO设置位输入模式
    gpio_direction_input(key.key_gpio);

    //获取设备树中指定的中断出发类型
    irq_flags =irq_get_trigger_type(key.irq_num);
    if(IRQF_TRIGGER_NONE==irq_flags)
    irq_flags=IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING;
    //申请中断
    ret = request_irq(key.irq_num,key_interrupt,irq_flags,"Key0_IRQ",NULL);
    if(ret){
        gpio_free(key.key_gpio);//释放之前申请的gpio编号
        return ret;
    }
return 0;
 }
//此时io口已经配置完毕，外部中断也配置完毕包括外部中断服务函数，每次中断都会导致一次定时器中断
 static void key_timer_function(struct timer_list *arg)
 {
//接下来就需要配置下定时器中断服务函数了
static int last_val =1;
unsigned long flags ;
int current_val ;
//不希望在我这里进行中断服务的时候被人共享了
//自旋锁上锁
spin_lock_irqsave(&key.spinlock,flags);
//接下来就是临界区了
//读取按键值并判断按键当前状态
current_val =gpio_get_value(key.key_gpio);
if(0 == current_val &&last_val)//按下
status =KEY_PRESS;
else if(1 == current_val &&!last_val)
status = KEY_RELEASE;//松开
else
status =KEY_KEEP;
last_val = current_val;
//自选锁解锁，也就是临界区结束
spin_unlock_irqrestore(&key.spinlock,flags);
 }
 //这个定时器内部中断服务函数做的工作就是判断下当前按键状态
 //open函数
 static int key_open(struct inode *inode,struct file*filp)
 {
    return 0;
 }

 //read函数
 static ssize_t key_read(struct file*filp,char __user *buf,size_t cnt,loff_t *offt)
 {
    unsigned long flags;
    int ret;
    //接下来需要转移数据了，继续上锁
    spin_lock_irqsave(&key.spinlock,flags);
    //之前是把状态读取出来，这回就是把状态信息发送给应用程序
    ret =copy_to_user(buf,&status,sizeof(int));
    //状态重置
    status =KEY_KEEP;
    spin_unlock_irqrestore(&key.spinlock,flags);
    return ret;
 }

 static ssize_t key_write(struct file *filp,const char __user*buf,size_t cnt,loff_t *offt)
 {
    return 0;
 }

 //close文件
 static int key_release(struct inode *inode, struct file *filp)
 {
 return 0;
 }
 //此时，gpio中断都完事了，把设备擦作结构体注册完以后就算驱动入口了
 static struct file_operations key_fops={
    .owner = THIS_MODULE,
    .open =key_open,
    .read =key_read,
    .write=key_write,
    .release =key_release,
 };

//驱动入口
static int __init mykey_init(void){
    int ret;
    //这里需要把gpio初始化，中断初始化，自选锁初始化，定时器初始化
    spin_lock_init(&key.spinlock);
    //设备数解析
    ret =key_parse_dt();
    if(ret)
return ret;
ret =key_gpio_init();
if(ret)
return ret;
//注册字符设备驱动
ret =alloc_chrdev_region(&key.devid,0,KEY_CNT,KEY_NAME);
if(ret<0){
    pr_err("%s couldnt alloc_chrdev_region,ret=%d\r\n",KEY_NAME,ret);
    goto free_gpio;
}
//初始化cdev
key.cdev.owner =THIS_MODULE;
cdev_init(&key.cdev,&key_fops);
cdev_add(&key.cdev, key.devid, KEY_CNT);
if(ret < 0)
goto del_unregister;
key.class = class_create(THIS_MODULE, KEY_NAME);
if (IS_ERR(key.class)) {
 goto del_cdev;
 }
 key.device = device_create(key.class, NULL, key.devid, NULL,KEY_NAME);
 if (IS_ERR(key.device)) {
 goto destroy_class;
 }
 timer_setup(&key.timer, key_timer_function, 0);
return 0;

destroy_class:
 device_destroy(key.class, key.devid);
 del_cdev:
 cdev_del(&key.cdev);
 del_unregister:
 unregister_chrdev_region(key.devid, KEY_CNT);
 free_gpio:
 free_irq(key.irq_num, NULL);
 gpio_free(key.key_gpio);
 return -EIO;
}

static void __exit mykey_exit(void)
 {
 /* 注销字符设备驱动 */
 cdev_del(&key.cdev); /* 删除 cdev */
 unregister_chrdev_region(key.devid, KEY_CNT); /* 注销设备号 */
 del_timer_sync(&key.timer); /* 删除 timer */
 device_destroy(key.class, key.devid); /*注销设备 */
 class_destroy(key.class); /* 注销类 */
 free_irq(key.irq_num, NULL); /* 释放中断 */
 gpio_free(key.key_gpio); /* 释放 IO */
 }

 module_init(mykey_init);
 module_exit(mykey_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");