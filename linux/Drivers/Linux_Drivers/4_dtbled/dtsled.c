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
 #include <asm/mach/map.h>
 #include <asm/uaccess.h>
 #include <asm/io.h>

 #define dtsled_CNT    1     //设备数
 #define  dtsled_NAME  "dtsled"
 #define  LEDOFF  0
 #define   LEDON   1

 /* 映射后的寄存器虚拟地址指针 */
 static void __iomem *MPU_AHB4_PERIPH_RCC_PI;
 static void __iomem *GPIOI_MODER_PI;
 static void __iomem *GPIOI_OTYPER_PI;
 static void __iomem *GPIOI_OSPEEDR_PI;
 static void __iomem *GPIOI_PUPDR_PI;
 static void __iomem *GPIOI_BSRR_PI;

 /* dtsled 设备结构体 */
 struct dtsled_dev{
 dev_t devid; /* 设备号 */
 struct cdev cdev; /* cdev */
 struct class *class; /* 类 */
 struct device *device; /* 设备 */
 int major; /* 主设备号 */
 int minor; /* 次设备号 */
 struct device_node *nd;/*多了一个设备树节点，不再需要
 自己编写物理信息了*/
 };
 //注意，此设备结构体并不是文件操作集合。

 //接下来是实例化一个结构体出来
 struct dtsled_dev   dtsled;


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
 {
    filp->private_data = &dtsled;//设置私有属性，这一步就是老式驱动程序没有的。
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

 static struct file_operations dtsled_fops = {
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
 u32 regdata[12];
 const char * str;
 struct property * proper;

 /*获取设备树中的属性数据*/
 /*1.获取设备节点：stm32mp1_led*/
 dtsled.nd =of_find_node_by_path("/stm32mp1_led");
 if(dtsled.nd == NULL)
 {
   printk("stm32mp1_led not found!\r\n");
   return -EINVAL;
 }else{
   printk("stm32mp1_led found\r\n");
 }

 /*2.获取compatible属性内容
 返回一个结构体*/
 proper = of_find_property(dtsled.nd ,"compatible",NULL);
 if(proper == NULL)
 {
   printk("compatible property find failed!\r\n");
 }else{
   printk("compatible  = %s\r\n",(char *)proper->value);
 }

 /*3.获取status属性内容*/
 ret = of_property_read_string(dtsled.nd,"status",&str);
 if(ret <0)
 {
printk("status read failed!\r\n");
 }else{
printk("status = %s\r\n",str);
 }

 /*4.获取reg属性*/
 ret = of_property_read_u32_array(dtsled.nd,"reg",regdata,12);
 if(ret<0)
 {
   printk("reg property read failed!\r\n");
 }else{
   u8 i=0;
   printk("reg data = \r\n");
   for(i=0; i<12; i++)
      printk("%#X",regdata[i]);//获得物理地址以及大小。
   printk("\r\n");
 }

 /* 初始化 LED ,这里开始的那些宏定义的地址就要使用设备数里面的了*/
 /* 1、寄存器地址映射 */
 MPU_AHB4_PERIPH_RCC_PI = of_iomap(dtsled.nd, 0);
 GPIOI_MODER_PI = of_iomap(dtsled.nd, 1);
 GPIOI_OTYPER_PI = of_iomap(dtsled.nd, 2);
 GPIOI_OSPEEDR_PI = of_iomap(dtsled.nd, 3);
 GPIOI_PUPDR_PI = of_iomap(dtsled.nd, 4);
 GPIOI_BSRR_PI = of_iomap(dtsled.nd, 5);

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
if(dtsled.major)//如果说定义了设备号那就不需要申请了
{
dtsled.devid = MKDEV(dtsled.major,0);//这里表示用定义的朱设备号生成32位的设备号
ret = register_chrdev_region(dtsled.devid,dtsled_CNT,dtsled_NAME);//依次传入设备号/设备数量/设备名称

if(ret<0)//判断下是否登记设备号成功了。
{
pr_err("cannot register %s char driver [ret=%d]\n",dtsled_NAME, dtsled_CNT);
 goto fail_map;

}
}else{//如果没有定义设备号的话，就需要我们申请一个设备号了
ret = alloc_chrdev_region(&dtsled.devid,0,dtsled_CNT,dtsled_NAME);/*这里传入的是设备号的地址，是用来得到设备号的。
让函数内部的局部变量作为devid的一个别名，这样可以保证数据传出函数*/
if(ret < 0) {
 pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n",dtsled_NAME, ret);//此函数就算printk的宏定义
 goto fail_map;
}
//此时设备号已经申请完毕了，但我们还不知道高12位和低20位是什么
dtsled.major = MAJOR(dtsled.devid);//这里用到了MAJOR函数
dtsled.minor = MINOR(dtsled.devid);
}
printk("newcheled major=%d,minor=%d\r\n",dtsled.major,dtsled.minor);

/*2.cdev初始化，之前说将设备号注册与文件操作集合的注册分开了，这里就是文件操作集合的注册了*/
dtsled.cdev.owner =THIS_MODULE; 
cdev_init(&dtsled.cdev,&dtsled_fops);//这里是要给cdev结构体赋值，其中一个变量三文集操作集合的地址，所以要传入后面的&，第一个&是为了把初始化赋值带出来

/*3.cdev添加*/
ret = cdev_add(&dtsled.cdev,dtsled.devid,dtsled_CNT);/*这里要添加的就是cdev中的文件操作集合
后边的两个参数是告诉linux作为哪个设备的文件操作集合*/
if(ret<0)
{
   goto del_unregister;
}
/*接下来就是自动生成设备投影了*/
/* 4、创建类 */
 dtsled.class = class_create(THIS_MODULE, dtsled_NAME);
 if (IS_ERR(dtsled.class)) {
 goto del_cdev;
 }

 /*5/创建设备*/
 dtsled.device = device_create(dtsled.class, NULL,dtsled.devid, NULL, dtsled_NAME);
 if (IS_ERR(dtsled.device)) {
 goto destroy_class;
 }
return 0;

/*接下来就是处理下那些各种情况失败的*/
destroy_class:
class_destroy(dtsled.class);

del_cdev:
cdev_del(&dtsled.cdev);

del_unregister:
 unregister_chrdev_region(dtsled.devid, dtsled_CNT);
 fail_map:
 led_unmap();
 return -EIO;

 }

 static void __exit led_exit(void)
 {
   /*取消映射*/
   led_unmap();

/*注销字符设备驱动*/
cdev_del(&dtsled.cdev);/* 删除 cdev */
 unregister_chrdev_region(dtsled.devid, dtsled_CNT);

 device_destroy(dtsled.class, dtsled.devid);
 class_destroy(dtsled.class);
 }

 module_init(led_init);
 module_exit(led_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");