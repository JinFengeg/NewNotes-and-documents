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
 #include <asm/mach/map.h>
 #include <asm/uaccess.h>
 #include <asm/io.h>

 #define NEWCHRLED_CNT    1     //设备数
 #define  NEWCHRLED_NAME  "newchrled"
 #define  LEDOFF  0
 #define   LEDON   1

/* 寄存器物理地址 */
 #define PERIPH_BASE (0x40000000)
 #define MPU_AHB4_PERIPH_BASE (PERIPH_BASE + 0x10000000)
 #define RCC_BASE (MPU_AHB4_PERIPH_BASE + 0x0000)
 #define RCC_MP_AHB4ENSETR (RCC_BASE + 0XA28)
 #define GPIOI_BASE (MPU_AHB4_PERIPH_BASE + 0xA000)
 #define GPIOI_MODER (GPIOI_BASE + 0x0000) 
 #define GPIOI_OTYPER (GPIOI_BASE + 0x0004)
 #define GPIOI_OSPEEDR (GPIOI_BASE + 0x0008) 
 #define GPIOI_PUPDR (GPIOI_BASE + 0x000C) 
 #define GPIOI_BSRR (GPIOI_BASE + 0x0018)

 /* 映射后的寄存器虚拟地址指针 */
 static void __iomem *MPU_AHB4_PERIPH_RCC_PI;
 static void __iomem *GPIOI_MODER_PI;
 static void __iomem *GPIOI_OTYPER_PI;
 static void __iomem *GPIOI_OSPEEDR_PI;
 static void __iomem *GPIOI_PUPDR_PI;
 static void __iomem *GPIOI_BSRR_PI;
/*新型字符设备的注册方式暴露了更多的内核对象，为此
建议创建结构体描述设备*/
 /* newchrled 设备结构体 */
 struct newchrled_dev{
 dev_t devid; /* 设备号 */
 struct cdev cdev; /* cdev */
 struct class *class; /* 类 */
 struct device *device; /* 设备 */
 int major; /* 主设备号 */
 int minor; /* 次设备号 */
 };
 //注意，此设备结构体并不是文件操作集合。

 //接下来是实例化一个结构体出来
 struct newchrled_dev   newchrled;


 //led打开/关闭
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

//取消映射，毕竟占用了虚拟地址
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

 //led打开设备
 static int led_open(struct inode *inode, struct file *filp)
 {/*相当于将自己定义的设备结构体注册到file文件中*/
    filp->private_data = &newchrled;//设置私有属性，这一步就是老式驱动程序没有的。
    return 0;
 }

 static ssize_t led_read(struct file *filp, char __user *buf,size_t cnt, loff_t *offt)
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
 } else if(ledstat == LEDOFF) {
 led_switch(LEDOFF); /* 关闭 LED 灯 */
 }
 return 0;
 }

 static int led_release(struct inode *inode, struct file *filp)
 {
 return 0;
 }

 static struct file_operations newchrled_fops = {
 .owner = THIS_MODULE,
 .open = led_open,
 .read = led_read,
 .write = led_write,
 .release = led_release,
 };

 static int __init led_init(void)
 {
 u32 val = 0;
 int ret;

 /* 初始化 LED */
 /* 1、寄存器地址映射 */
 MPU_AHB4_PERIPH_RCC_PI = ioremap(RCC_MP_AHB4ENSETR, 4);
 GPIOI_MODER_PI = ioremap(GPIOI_MODER, 4);
 GPIOI_OTYPER_PI = ioremap(GPIOI_OTYPER, 4);
 GPIOI_OSPEEDR_PI = ioremap(GPIOI_OTYPER, 4);
 GPIOI_PUPDR_PI = ioremap(GPIOI_PUPDR, 4);
 GPIOI_BSRR_PI = ioremap(GPIOI_BSRR, 4);

 /* 2、使能 PI 时钟 */
 val = readl(MPU_AHB4_PERIPH_RCC_PI);
 val &= ~(0X1 << 8); /* 清除以前的设置 */
 val |= (0X1 << 8); /* 设置新值 */
 writel(val, MPU_AHB4_PERIPH_RCC_PI);

 /* 3、设置 PI0 通用的输出模式。*/
 val = readl(GPIOI_MODER_PI);
 val &= ~(0X3 << 0); /* bit0:1 清零 */
 val |= (0X1 << 0); /* bit0:1 设置 01 */
 writel(val, GPIOI_MODER_PI);

 /* 3、设置 PI0 为推挽模式。*/
 val = readl(GPIOI_OTYPER_PI);
 val &= ~(0X1 << 0); /* bit0 清零，设置为上拉*/
 writel(val, GPIOI_OTYPER_PI);

 /* 4、设置 PI0 为高速。*/
 val = readl(GPIOI_OSPEEDR_PI);
 val &= ~(0X3 << 0); /* bit0:1 清零 */
 val |= (0x2 << 0); /* bit0:1 设置为 10 */
 writel(val, GPIOI_OSPEEDR_PI);

 /* 5、设置 PI0 为上拉。*/
 val = readl(GPIOI_PUPDR_PI);
 val &= ~(0X3 << 0); /* bit0:1 清零 */
 val |= (0x1 << 0); /*bit0:1 设置为 01 */
 writel(val,GPIOI_PUPDR_PI);

 /* 6、默认关闭 LED */
 val = readl(GPIOI_BSRR_PI);
 val |= (0x1 << 0);
 writel(val, GPIOI_BSRR_PI);

 /*以上都是属于硬件部分没有什么变化*/

 /*接下来要注册字符驱动*/
 /*区别在于，这里我们注册设备和注册文件操作集合分开，并且还要自动创建设备投影，
 1.首先判断设备号是否存在
 2.如果不存在就申请设备号
 3.有了设备号以后就注册设备号和设备名称
 4.根据设备号添加相应的文件操作集合
 5.接下来添加自动创建设备投影，先创建类
 6.然后创建设备结构体
*/

/*创建设备号*/
if(newchrled.major)//如果说定义了设备号那就不需要申请了
{
newchrled.devid = MKDEV(newchrled.major,0);//这里表示用定义的朱设备号生成32位的设备号
ret = register_chrdev_region(newchrled.devid,NEWCHRLED_CNT,NEWCHRLED_NAME);//依次传入设备号/设备数量/设备名称
/*这里多了_region后缀，形参增加设备数量。*/
if(ret<0)//判断下是否登记设备号成功了。
{
pr_err("cannot register %s char driver [ret=%d]\n",NEWCHRLED_NAME, NEWCHRLED_CNT);
 goto fail_map;

}
}else{//如果没有定义设备号的话，就需要我们申请一个设备号了
ret = alloc_chrdev_region(&newchrled.devid,0,NEWCHRLED_CNT,NEWCHRLED_NAME);/*这里传入的是设备号的地址，是用来得到设备号的。
让函数内部的局部变量作为devid的一个别名，这样可以保证数据传出函数*/
if(ret < 0) {
 pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n",NEWCHRLED_NAME, ret);//此函数就算printk的宏定义
 goto fail_map;
}
//此时设备号已经申请完毕了，但我们还不知道高12位和低20位是什么
newchrled.major = MAJOR(newchrled.devid);//这里用到了MAJOR函数
newchrled.minor = MINOR(newchrled.devid);
}
printk("newcheled major=%d,minor=%d\r\n",newchrled.major,newchrled.minor);

/*2.cdev初始化，之前说将设备号注册与文件操作集合的注册分开了，这里就是文件操作集合的注册了*/
newchrled.cdev.owner =THIS_MODULE; 
cdev_init(&newchrled.cdev,&newchrled_fops);//这里是要给cdev结构体赋值，其中一个变量三文集操作集合的地址，所以要传入后面的&，第一个&是为了把初始化赋值带出来
/*将cdev与file_ops指针联系起来*/
/*3.cdev添加*/
ret = cdev_add(&newchrled.cdev,newchrled.devid,NEWCHRLED_CNT);/*这里要添加的就是cdev中的文件操作集合
后边的两个参数是告诉linux作为哪个设备的文件操作集合*/
if(ret<0)
{
   goto del_unregister;
}
/*接下来就是自动生成设备投影了，在/class/目录下生成设备节点*/
/* 4、创建类 */
 newchrled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
 if (IS_ERR(newchrled.class)) {
 goto del_cdev;
 }

 /*5/创建设备*/
 /*利用创建的class生成设备，此时在/dev/目录下生成设备节点*/
 newchrled.device = device_create(newchrled.class, NULL,newchrled.devid, NULL, NEWCHRLED_NAME);
 if (IS_ERR(newchrled.device)) {
 goto destroy_class;
 }
return 0;

/*接下来就是处理下那些各种情况失败的*/
destroy_class:
class_destroy(newchrled.class);

del_cdev:
cdev_del(&newchrled.cdev);

del_unregister:
 unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT);
 fail_map:
 led_unmap();
 return -EIO;

 }

 static void __exit led_exit(void)
 {
   /*取消映射*/
   led_unmap();

/*注销字符设备驱动*/
/*没想到这个cdev不需要申请*/
cdev_del(&newchrled.cdev);/* 删除 cdev */
 unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT);

 device_destroy(newchrled.class, newchrled.devid);
 class_destroy(newchrled.class);
 }

 module_init(led_init);
 module_exit(led_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");