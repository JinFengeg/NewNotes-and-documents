#include <linux/types.h> 
#include <linux/kernel.h> 
#include <linux/delay.h> 
#include <linux/ide.h> 
#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/errno.h> 
#include <linux/gpio.h> 
#include <asm/mach/map.h>
 #include <asm/uaccess.h>
 #include <asm/io.h>

 #define LED_MAJOR      200
 #define LED_NAME       "led"

 #define    LEDON          1
 #define LEDOFF             0

 /*寄存器物理地址
 这个就算因为没有库函数只能自己敲了*/

 #define   PERIPH_BASE  (0X40000000)
 #define   MPU_AHB4_PERIPH_BASE    (PERIPH_BASE+0x10000000)
 #define   RCC_BASE        (MPU_AHB4_PERIPH_BASE+0X0000)
 #define   RCC_MP_AHB4ENSETR        (RCC_BASE+0XA28)
 #define   GPIOI_BASE        (MPU_AHB4_PERIPH_BASE+0XA000)
 #define   GPIOI_MODER   (GPIOI_BASE+0X0000)
 #define   GPIOI_OTYPER  (GPIOI_BASE+0X0004)
 #define   GPIOI_OSPEEDR    (GPIOI_BASE+0X0008)
 #define   GPIOI_PUPDR     (GPIOI_BASE+0X000C)
 #define   GPIOI_BSRR   (GPIOI_BASE+0X0018)

/*映射后寄存器虚拟地址的指针*/

static void __iomem *   MPU_AHB4_PERIPH_BASE_PI;
static void __iomem *   GPIOI_MODER_PI;
static void __iomem *   GPIOI_OTYPER_PI;
static void __iomem *   GPIOI_OSPEEDR_PI;
static void __iomem *   GPIOI_PUPDR_PI;
static void __iomem *   GPIOI_BSRR_PI;

/*LED打开关闭*/
void  led_switch(u8 sta)
{
    u32 val = 0;
    if(sta == LEDON)
    {
        val = readl(GPIOI_BSRR_PI);//读取下bsrr寄存器中的值
        val |= (1<<16);//最高位之一
        writel(val,GPIOI_BSRR_PI);//重新写入该寄存器中。
    }else{
        val = readl(GPIOI_BSRR_PI);//读取下bsrr寄存器中的值
        val |= (1<<0);//最低位为1
        writel(val,GPIOI_BSRR_PI);//重新写入该寄存器中。
    }
}

/*取消映射*/
void  led_unmap(void)
{
    iounmap(MPU_AHB4_PERIPH_BASE_PI);
    iounmap(GPIOI_MODER_PI);
    iounmap(GPIOI_OTYPER_PI);
    iounmap(GPIOI_OSPEEDR_PI);
    iounmap(GPIOI_PUPDR_PI);
    iounmap(GPIOI_BSRR_PI);
}

/*打开设备*/
static  int  led_open(struct inode *inode, struct file *filp)
{
    return 0;
}

/*从设备读取数据*/
static int  led_read( struct file *filp,char __user *buf,size_t cnt, loff_t *offt)
{
    return 0;
}

/*向设备写数据*/
static ssize_t led_write( struct file *filp,const char __user *buf,size_t cnt, loff_t  *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned  char ledstat;

    retvalue = copy_from_user(databuf,buf,cnt);/*这里可以看出，控制led状态的就是由APP中的write传入*/
    if(retvalue<0)
    {
        printk("kernel write failed !\r\n");
        return -EFAULT;
    }
    ledstat =databuf[0];/*获取状态值*/
if(ledstat == LEDON)
{
    led_switch(LEDON);
}else if(ledstat == LEDOFF){
    led_switch(LEDOFF);
}
return 0;
}

/*关闭/释放led设备*/

static int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/*设备操作结构体*/
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .release = led_release,
    .write = led_write,
};

/*驱动入口函数*/
static int __init led_init(void)
{
    int revalue = 0;
    u32 val = 0;

    /*初始化led*/
    /*1.寄存器地址映射*/
    //ioremap是提供物理地址，然后返回转换后的虚拟地址
    MPU_AHB4_PERIPH_BASE_PI = ioremap(RCC_MP_AHB4ENSETR, 4);
    GPIOI_MODER_PI = ioremap(GPIOI_MODER, 4);
    GPIOI_OTYPER_PI = ioremap(GPIOI_OTYPER, 4);
    GPIOI_OSPEEDR_PI = ioremap(GPIOI_OSPEEDR, 4);
    GPIOI_PUPDR_PI = ioremap(GPIOI_PUPDR, 4);
    GPIOI_BSRR_PI = ioremap(GPIOI_BSRR, 4);

    /*2.使能PI时钟*/
    val = readl(MPU_AHB4_PERIPH_BASE_PI);
    val &=~(0X1<<8);
    val |= (0x1<<8);
    writel(val,MPU_AHB4_PERIPH_BASE_PI);

    /*设置PI0的IO端口为通用输出模式*/
    val =readl(GPIOI_MODER_PI);
    val &=~(0x3<<0);/*bit01位清零*/
    val |=(0x1<<0);/*bit01位置为10*/
    writel(val,GPIOI_MODER_PI);

    /*设置为推挽输出模式*/
    val =readl(GPIOI_OTYPER_PI);
    val &=~(0x1<<0);/*bit0位清零，设置为上拉*/
    writel(val,GPIOI_OTYPER_PI);

    /*设置为PI0为高速*/
    val = readl(GPIOI_OSPEEDR_PI);
    val &=~(0x3<<0);//bit01清零
    val |=(0x2<<0);//bit01设置为01
    writel(val,GPIOI_OSPEEDR_PI);

    /* 5、设置 PI0 为上拉。*/
    val = readl(GPIOI_PUPDR_PI);
    val &= ~(0X3 << 0); /* bit0:1 清零 */
    val |= (0x1 << 0); /*bit0:1 设置为 01 */
    writel(val,GPIOI_PUPDR_PI);

    /* 6、默认关闭 LED */
    val = readl(GPIOI_BSRR_PI);
    val |= (0x1 << 0); 
    writel(val, GPIOI_BSRR_PI);

    /*以上属于初始化IO口，接下来就是注册字符设备了*/
    revalue = register_chrdev(LED_MAJOR,LED_NAME,&led_fops);
    if(revalue<0)
    {
        printk("register chrdev failed!\r\n");
         goto fail_map;
    }
    return 0;

fail_map:
led_unmap();/*既然注册字符设备失败了那就取消映射，这个注册字符设备是为了挂载到系统上，
为了可以投影到用户空间，使得用户可以操作设备，至于初始化引脚并不需要挂载就可以完成对底层寄存器的操作*/

return -EIO;

}

/*驱动出口函数*/
static void __exit led_exit(void)
{
    /*取消映射地址*/
    led_unmap();

    /*注销字符设备*/
    unregister_chrdev(LED_MAJOR,LED_NAME);

}

 module_init(led_init);
 module_exit(led_exit);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");
