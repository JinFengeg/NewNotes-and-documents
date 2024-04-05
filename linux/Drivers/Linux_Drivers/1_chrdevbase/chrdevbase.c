#include <linux/types.h> 
 #include <linux/kernel.h> 
 #include <linux/delay.h> 
 #include <linux/ide.h> 
 #include <linux/init.h> 
 #include <linux/module.h>

/***************************************************************
8 Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
9 文件名 : chrdevbase.c
10 作者 : 正点原子
11 版本 : V1.0
12 描述 : chrdevbase 驱动文件。
13 其他 : 无
14 论坛 : www.openedv.com
15 日志 : 初版 V1.0 2020/12/26 正点原子团队创建
16 ***************************************************************/

#define     CHRDEVBASE_MAJOR      200
#define     CHRDEVBASE_NAME     "chrdevbase"

static char readbuf[100]; /* 读缓冲区 */
 static char writebuf[100]; /* 写缓冲区 */
 static char kerneldata[] = {"kernel data!"};

 /*
26 * @description : 打开设备
27 * @param – inode : 传递给驱动的 inode
28 * @param - filp : 设备文件，file 结构体有个叫做 private_data 的成员变量
29 * 一般在 open 的时候将 private_data 指向设备结构体。
30 * @return : 0 成功;其他 失败
31 */

static  int chrdevbase_open(struct       inode   *inode,   struct  file *filp)
{
    //printk("chrdevbase open!\r\n");
    return 0;
}

static ssize_t chrdevbase_read(struct file *filp,char  __user  *buf,size_t cnt,loff_t *offt)
{
    int retvalue=0;
    /* 向用户空间发送数据 */
 memcpy(readbuf, kerneldata, sizeof(kerneldata));
 retvalue = copy_to_user(buf, readbuf, cnt);
 if(retvalue == 0){
 printk("kernel senddata ok!\r\n");
 }else{
 printk("kernel senddata failed!\r\n");
 }
 
 //printk("chrdevbase read!\r\n");
 return 0;
}

static ssize_t chrdevbase_write(struct file *filp,const char __user *buf,size_t cnt, loff_t *offt)
 {
 int retvalue = 0;
 /* 接收用户空间传递给内核的数据并且打印出来 */
 retvalue = copy_from_user(writebuf, buf, cnt);
 if(retvalue == 0){
 printk("kernel recevdata:%s\r\n", writebuf);
 }else{
 printk("kernel recevdata failed!\r\n");
 }
 //printk("chrdevbase write!\r\n");
 return 0;
 }

  static int chrdevbase_release(struct inode *inode,struct file *filp)
 {
 //printk("chrdevbase release！\r\n");
 return 0;
 }
/*这就是设备节点操作函数结构体，对于已打开文件直接从file文件
filp->file_operations，如果是未打开文件，需要从inode->c_dev->file_ops*/
 static struct file_operations chrdevbase_fops = {
 .owner = THIS_MODULE, 
 .open = chrdevbase_open,
 .read = chrdevbase_read,
 .write = chrdevbase_write,
 .release = chrdevbase_release,
 };

/*
109 * @description : 驱动入口函数
110 * @param : 无
111 * @return : 0 成功;其他 失败
112 */
 static int __init chrdevbase_init(void)
 {
 int retvalue = 0;

 /* 注册字符设备驱动 */
 retvalue = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
 if(retvalue < 0){
 printk("chrdevbase driver register failed\r\n");
 }
printk("chrdevbase_init()\r\n");
 return 0;
 }

 /*
127 * @description : 驱动出口函数
128 * @param : 无
129 * @return : 无
130 */
 static void __exit chrdevbase_exit(void)
 {
 /* 注销字符设备驱动 */
 unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
 printk("chrdevbase_exit()\r\n");
 }

 /* 
139 * 将上面两个函数指定为驱动的入口和出口函数
140 */
 module_init(chrdevbase_init);
 module_exit(chrdevbase_exit);

 /* 
145 * LICENSE 和作者信息
146 */
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");