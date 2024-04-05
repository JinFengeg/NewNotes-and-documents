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
 #include <linux/i2c.h>
 #include <asm/mach/map.h>
 #include <asm/uaccess.h>
 #include <asm/io.h>
 #include "ap3216creg.h"

 #define AP3216C_CNT  1
 #define AP3216C_NAME  "ap3216c"

 struct ap3216c_dev{
    struct i2c_client *client;//i2c设备
    dev_t devid;//设备号
    struct cdev cdev;//cdev 
    struct class *class;//如果使用misc框架的话就不需要这些了
    struct device *device;//设备
    struct device_node *nd;//设备节点
    unsigned short ir,als,ps;//三个光传感器数据
 };

 //从ap3216c读取多个寄存器数据
 static int ap3216c_read_regs(struct ap3216c_dev *dev,u8 reg,void *val,int len) {
    int ret ;
    struct i2c_msg msg[2];
    struct i2c_client *client=(struct i2c_client *)dev->client;//这里把设备结构体传进来了，里面的adapter要用到
    //msg为发送要读取的首地址
    msg[0].addr =client->addr;//ap3216地址这个地址是内核从
    msg[0].flags =0;//标记为发送数据
    msg[0].buf=&reg;//读取的首地址
    msg[0].len =1;//reg长度

    //msg[1]读取数据
    msg[1].addr =client->addr;//发送和接受是共用一个地址阿
    msg[1].flags =I2C_M_RD;//标记为读取数据
    msg[1].buf =val;//读取数据缓冲区，传进来的是一个地址
    msg[1].len =len;//第二次要读取的数据长度
    ret =i2c_transfer(client->adapter,msg,2);//这是有两个msg结构体
    if(ret ==2){
        ret =0;//表示数据成功读取了
    }else{
        printk("i2c rd failed =%d reg=%06x len=%d",ret,reg,len);
        ret =-EREMOTEIO;
    }
    return ret;
 }

 //向ap3216c多个寄存器写入数据
 static s32 ap3216c_write_regs(struct ap3216c_dev *dev,u8 reg,u8 *buf,u8 len) {
    u8 b[256];//这里我就不太明白了，为什么加上这个缓冲区
    struct i2c_msg msg;
    struct i2c_client *client=(struct i2c_client *)dev->client;

    b[0] =reg;//寄存器首地址
    memcpy(&b[1],buf,len);//这里就是实现将多个数值存入一个数组中，然后写入len+1个数据就是县将多个数据写入多个寄存器中了
    msg.addr =client->addr;//ap3216c地址也就是0x1e
    msg.flags =0;//标记为写数据
    msg.buf =b;//要挟如的数据缓冲区，这是把整个数组都要用米
    msg.len =len+1;//要挟如的数据长度

    return i2c_transfer(client->adapter,&msg,1);//这里是消息数量，不管是一个数组还是一个数，看的是msg结构体的数量，而不是成员的数量
//理解为每次要写入的数据长度
 }

 //读取ap3216c制定寄存器值，读取一个寄存器
 static unsigned char ap3216c_read_reg(struct ap3216c_dev *dev, u8 reg) {
u8 data =0;//这个数据会被覆盖
ap3216c_read_regs(dev,reg,&data,1);//要读取的数据存放在&data地址里，读取个数由传入数量决定，最后是1表示读取1个数据

return data;//返回值就是我们读取到的值
 }

 //向ap3216c制定寄存器写入指定值
 static void ap3216c_write_reg(struct ap3216c_dev *dev,u8 reg,u8 data) {
    u8 buf =0;
    buf =data;
    ap3216c_write_regs(dev,reg,&buf,1);//这里的1表示是msg成员中的len长度

 }

 //读取AP3216C的数据，包括ALS/PS/IR，注意，如果同时打开ALS/IR+PS两次数据的读取时间大雨112.5ms
 void ap3216c_readdata(struct ap3216c_dev *dev){
    unsigned char i=0;
    unsigned char buf[6];
    //循环读取所有传感器数据
for(i=0;i<6;i++){
    buf[i]=ap3216c_read_reg(dev,AP3216C_IRDATALOW+i);//这里并没有延时112.5ms

}
if(buf[0] & 0x80)//如果IR_OF为1则数据无效，这是手册提到的
dev->ir =0;
else //读取IR传感器的数据，注意，此时获得了高8位的数据和第八位的数据，还需要合并起来
dev->ir =((unsigned short)buf[1] << 2)|(buf[0] &0x03);

dev->als =((unsigned short)buf[3] << 8) | buf[2];
if(buf[4] & 0x40)//同理，但是为什么被判断下als传感器的值呢
dev->ps =0;
else
dev->ps =((unsigned short)(buf[5] & 0x3f) << 4) |(buf[4] &0x0F);

 }
 //打开设备
 static int ap3216c_open(struct inode *inode,struct file *filp){
    //从file结构体获得cdev指针，在根据cdev获取ap3216c_dev首地址，
 //之前一直不知道cdev有什么用，但是走的廷深的，
 struct cdev *cdev =filp->f_path.dentry->d_inode->i_cdev;
 struct ap3216c_dev *ap3216cdev =container_of(cdev,struct ap3216c_dev,cdev);
 //这个container_of函数我也不认识，好像是返回一个地址。
//这是创建ap3216c_dev类型的结构体，之前没有创建，现在配置了但是从filp中获得的，反正是现在没有配置。暂时拿过来用而已。
 //初始化AP3216c
 ap3216c_write_reg(ap3216cdev,AP3216C_SYSTEMCONG,0x04);
//这里是配置ap3216c的寄存器，并不是157的，在内核就已经配置完了。
//至于为什么传入的是0x04就是芯片手册知道了。
mdelay(50);
ap3216c_write_reg(ap3216cdev,AP3216C_SYSTEMCONG,0x03);
return 0;
//这个open文件就是操作硬件。
 }

 //从设备读取数据，read函数，之前都是单片机水平的，现在要把独到的数据传入用户空间
 static ssize_t ap3216c_read(struct file *filp,char __user *buf,size_t cnt,loff_t *off){
    short data[3];
    long err =0;
    struct cdev *cdev =filp->f_path.dentry->d_inode->i_cdev;
    struct ap3216c_dev *dev =container_of(cdev,struct ap3216c_dev,cdev);
    //怎么又来了一遍，对于这种局部结构体还是临时用用被。
    ap3216c_readdata(dev);//读取三个传感器中的数据保存到dev结构体中。

    data[0] =dev->ir;//把独到的数据存入data中
    data[0]=dev->als;
    data[0] = dev->ps;
    err =copy_to_user(buf,data,sizeof(data));//把数组传给应用程序
    return 0;
 }

 //关闭设备
 static int ap3216c_release(struct inode *inode,struct file *filp){
    return 0;
 }

//ap3216c操作函数

static const struct file_operations ap3216c_ops ={
    .owner =THIS_MODULE,
    .open =ap3216c_open,
    .read =ap3216c_read,
    .release =ap3216c_release,
};

//目前操作函数都写完了，匹配队列，注册注销，入口出口还美协，
static int ap3216c_probe(struct i2c_client *client,const struct i2c_device_id *id ){
    int ret;
    struct ap3216c_dev *ap3216cdev;//总算是创建了局部结构体
    ap3216cdev =devm_kzalloc(&client->dev,sizeof(*ap3216cdev),GFP_KERNEL);
    //这里算是申请了内存在内核中，这种申请内存应该就属于全局变量了吧..
    if(!ap3216cdev)
    return -ENOMEM;
    //既然创建了设备结构体接下来就进行初始化被，引脚啥的内核就完成初始化了
    ret =alloc_chrdev_region(&ap3216cdev->devid,0,AP3216C_CNT,AP3216C_NAME);
    //申请设备号了，真的是好久没申请了，毕竟platform自动申请了，misc朱设备号默认就是10input是13朱设备号
    if(ret <0){
        pr_err("%s Couldnt alloc_chrdev_region,ret=%d\r\n",AP3216C_NAME,ret);
        return -ENOMEM;
    }

    //初始化cdev，这个之前一直在用，和ap3216c_dev之间的关系
    ap3216cdev->cdev.owner =THIS_MODULE;
    cdev_init(&ap3216cdev->cdev,&ap3216c_ops);

    //添加一个cdev
    ret =cdev_add(&ap3216cdev->cdev,ap3216cdev->devid,AP3216C_CNT);
    if(ret < 0){
        goto del_unregister;
    }    

    //创建类
    ap3216cdev->class =class_create(THIS_MODULE,AP3216C_NAME);
    if(IS_ERR(ap3216cdev->class)){
        goto del_cdev;
    }

    //创建设备
    ap3216cdev->device =device_create(ap3216cdev->class,NULL,ap3216cdev->devid,NULL,AP3216C_NAME);
    if(IS_ERR(ap3216cdev->device)){
        goto destroy_class;
    }
    ap3216cdev->client =client;//把内核创建的设备结构体传进来
    //保存ap3216cdev结构体
    i2c_set_clientdata(client,ap3216cdev);//将我们创建的结构体存入内核中就是这一步实现
    return 0;
    destroy_class:
    device_destroy(ap3216cdev->class,ap3216cdev->devid);
    del_cdev:
    cdev_del(&ap3216cdev->cdev);
    del_unregister:
    unregister_chrdev_region(ap3216cdev->devid,AP3216C_CNT);
    return -EIO;
}

//iic驱动的remove函数
static int ap3216c_remove(struct i2c_client *client){
    struct ap3216c_dev *ap3216cdev =i2c_get_clientdata(client);//找到我们注册到内核里的那个ap3216cdev结构体
    //注销字符设备驱动
    //1.删除cdev
    cdev_del(&ap3216cdev->cdev);
    //2.注销设备号
    unregister_chrdev_region(ap3216cdev->devid,AP3216C_CNT);
    //3.注销设备
    class_destroy(ap3216cdev->class);
    return 0;
}

//传统匹配ID列表
static const struct i2c_device_id ap3216c_id[]={
    {"alientek,ap3216c",0},
    {}
};
//probe和remove函数都写完了，接下来就是i2c的匹配设备树了
//设备树匹配列表
static const struct of_device_id ap3216c_of_match[]={
    {.compatible ="alientek,ap3216c"},
    {}
};

//i2c驱动结构体，将匹配列表注册到内核中方便match函数使用
static struct i2c_driver ap3216c_driver ={
    .probe =ap3216c_probe,
    .remove =ap3216c_remove,
    .driver ={
        .owner =THIS_MODULE,
        .name ="ap3216",
        .of_match_table =ap3216c_of_match,
    },
    .id_table =ap3216c_id,//传统无设备树方法，但是没啥用，我们没有自己创建设备文件

};
//驱动入口函数
static int __init ap3216c_init(void){
    //这里就是注册i2c然后就会调用probe了。
    int ret =0;
    ret =i2c_add_driver(&ap3216c_driver);
    return ret;
}

//驱动出口函数
static void __exit ap3216c_exit(void){
    i2c_del_driver(&ap3216c_driver);//需要注意的就是我们使用的是我们用的是i2c子系统

}

module_init(ap3216c_init);
module_exit(ap3216c_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIENTEK");
MODULE_INFO(intree,"Y");