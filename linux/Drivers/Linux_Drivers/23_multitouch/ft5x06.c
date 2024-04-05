#include <linux/module.h>
 #include <linux/i2c.h>
 #include <linux/input/mt.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

/* FT5426 寄存器相关宏定义 */
 #define FT5426_DEVIDE_MODE_REG 0x00 /* 模式寄存器 */ 
 #define FT5426_TD_STATUS_REG 0x02 /* 状态寄存器 */ 
 #define FT5426_TOUCH_DATA_REG 0x03 /* 触摸数据读取的起始寄存器 */ 
 #define FT5426_ID_G_MODE_REG 0xA4 /* 中断模式寄存器 */ 

 #define MAX_SUPPORT_POINTS 5 /* ft5426 最大支持 5 点触摸 */ 

 #define TOUCH_EVENT_DOWN 0x00 /* 按下 */ 
 #define TOUCH_EVENT_UP 0x01 /* 抬起 */ 
 #define TOUCH_EVENT_ON 0x02 /* 接触 */ 
 #define TOUCH_EVENT_RESERVED 0x03 /* 保留 */

 //县创建ft5426设备的结构体
 struct edt_ft5426_dev{
    struct i2c_client *client;//从这个结构体中获得内核定义的底层通信函数
    struct input_dev *input;//input子系统的结构体，到时候需要申请这个结构体
    int reset_gpio;
    int irq_gpio;//之前并没有配置中断，阿内核帮我们把硬件寄存器都初始化了？

 };

 static int edt_ft5426_ts_write(struct edt_ft5426_dev *ft5426,u8 addr,u8 *buf,u16 len)
 {
    //县根据iic中的通信函数把和触摸IC的函数封装出来
    struct i2c_client *client =ft5426->client;//把内核中的结构体拿出来
    struct i2c_msg msg;//iic通信信息的载体
    u8 send_buf[6] ={0};
    int ret;
    send_buf[0] =addr;//要发送的第一个字节就是寄存器地址
    memcpy(&send_buf[1],buf,len);//将我们传入的参数赋值给第二个字节
    msg.flags =0;//写
    msg.addr =client->addr;//这里输入的是从机在总线上的地址
    msg.buf =send_buf;//将要写入的数据传进来
    msg.len =len+1;//加上寄存器地址的字节
    ret =i2c_transfer(client->adapter,&msg,1);
    if(1 == ret)
    return 0;
    else{
        dev_err(&client->dev,"%s write error,addr =0x%x len =%d.\n",__func__,addr,len);
        return -1;
    }

 }

 static int edt_ft5426_ts_read(struct edt_ft5426_dev *ft5426,u8 addr,u8*buf,u16 len)
 {
    struct i2c_client *client =ft5426->client;
    struct i2c_msg msg[2];
    int ret;
    msg[0].flags =0;
    msg[0].addr =client->addr;
    msg[0].buf =&addr;
    msg[0].len =1;//因为读数据之前要先发送读的信息，这里的msg是数组，而spi需要县申请信息的结构体

    msg[1].flags =I2C_M_RD;
    msg[1].addr =client->addr;
    msg[1].buf =buf;
    msg[1].len =len;

    ret =i2c_transfer(client->adapter,msg,2);
    if(2 == ret){
        return 0;
    }else{
        dev_err(&client->dev,"%s:read error,addr=0x%x len=%d.\n",__func__,addr,len);
        return -1;
    }
 }

 static int edt_ft5426_ts_reset(struct edt_ft5426_dev *ft5426){
    //这里明白了，原来之所以空着这两个引脚是为了写复位函数，以及中断函数，中断到底是让谁中断
    struct i2c_client*client =ft5426->client;
    int ret;
    //首先是获取副为引脚，然后初始化引脚，其实这部分应该 放到probe函数中写
    ft5426->reset_gpio =of_get_named_gpio(client->dev.of_node,"reset-gpios",0);//申请GPIO编号
    if(!gpio_is_valid(ft5426->reset_gpio)){
        dev_err(&client->dev,"Failed to get is reset gpio\n");
        return ft5426->reset_gpio;
    }
    //申请编号以后就要申请使用了，这个函数怎么还跟一个one，而且不光是申请，后面怎么还跟一个标志位
    ret =devm_gpio_request_one(&client->dev,ft5426->reset_gpio,GPIOF_OUT_INIT_HIGH,"ft5426 reset");
    //这里不需要手动释放引脚了
    if(ret < 0){
        return ret;
    }
    msleep(20);
    gpio_set_value_cansleep(ft5426->reset_gpio,0);//既然引脚申请，初始化都完成了，接下来就是
//这里设置引脚值也不太对，怎么跟一个cansleep
    gpio_set_value_cansleep(ft5426->reset_gpio,1);
    return 0;
 }

 static irqreturn_t edt_ft5426_ts_isr(int irq,void*dev_id){
    struct edt_ft5426_dev *ft5426 =dev_id;
    u8 rdbuf[30]={0};
    int i,type,x,y,id;
    bool down;
    int ret;

    //此函数就是读取芯片触摸点的函数了
    ret =edt_ft5426_ts_read(ft5426,FT5426_TD_STATUS_REG,rdbuf,29);
    if(ret)
    goto out;
    for(i =0;i<MAX_SUPPORT_POINTS;i++) {
        u8 *buf =&rdbuf[i *6+1];
        //第一个触摸点为例，寄存器TOUCH1_XH（地址0x03
        type = buf[0] >> 6; /* 获取触摸点的 Event Flag ，这里是最高两位就是flag，这里的flag是什么*/ 
        if (type == TOUCH_EVENT_RESERVED)
        continue;
        x =((buf[2] << 8) | buf[3]) &0x0fff;  
        y =((buf[0] << 8)| buf[1]) &0x0fff; //循环的这部分都是硬件相关的

        id =(buf[2] >> 4) &0x0f;//这里获得的就是哪个触摸点，这是触摸IC给出的
        down = type != TOUCH_EVENT_UP;//这里决定是否是连续触摸

        input_mt_slot(ft5426->input,id);     //这里开始就不是硬件了，而是上报部分，不是应该说把上报放在中断中么  
        input_mt_report_slot_state(ft5426->input,MT_TOOL_FINGER,down); 
        
        if(!down){
            continue;
        }
        input_report_abs(ft5426->input,ABS_MT_POSITION_X,x);
         input_report_abs(ft5426->input,ABS_MT_POSITION_Y,y);


    }
    input_mt_report_pointer_emulation(ft5426->input,true);//判断下当前追踪的数量是否有效
    input_sync(ft5426->input);
out:
return IRQ_HANDLED;
 }
 //上面这个函数要说有趣的地方就是满足了上报时序，但中断加循环了

 static int edt_ft5426_ts_irq(struct edt_ft5426_dev *ft5426){
    struct i2c_client *client =ft5426->client;
    int ret;
    //之前还有一个引脚没有初始化，就是中断引脚
    ft5426->irq_gpio =of_get_named_gpio(client->dev.of_node,"irq-gpios",0);
    if(!gpio_is_valid(ft5426->irq_gpio)){
        dev_err(&client->dev, "Failed to get ts interrupt gpio\n");
         return ft5426->irq_gpio;
    }
    //申请使用管脚
    ret =devm_gpio_request_one(&client->dev,ft5426->irq_gpio,GPIOF_IN,"ft5426 interrupt");
    if(ret < 0){
        return ret;
    }

    //注册中断服务函数,这个函数的参数真多
    ret =devm_request_threaded_irq(&client->dev,gpio_to_irq(ft5426->irq_gpio),NULL,edt_ft5426_ts_isr,IRQF_TRIGGER_FALLING |IRQF_ONESHOT,client->name,ft5426);
printk("中断号=%d",gpio_to_irq(ft5426->irq_gpio));
printk("gpio编号=%d",ft5426->irq_gpio);
    if(ret){
        dev_err(&client->dev, "Failed to request touchscreen IRQ.\n");
 return ret;
    }
    return 0;
 }

 //此时中断服务函数写完了，并且初始化的函数写万了
 static int edt_ft5426_ts_probe(struct i2c_client *client,const struct i2c_device_id *id)
 {
    struct edt_ft5426_dev *ft5426;
    struct input_dev *input;
    u8 data;
    int ret ;
    //实例化一个struct edt_ft5426_dev对象,需要申请一个设备结构体
    ft5426 =devm_kzalloc(&client->dev,sizeof(struct edt_ft5426_dev),GFP_KERNEL);
    if(!ft5426){
        dev_err(&client->dev, "Failed to allocate ft5426 driver data.\n");
         return -ENOMEM;
    }
    ft5426->client =client;
    //复位ft5426触摸芯片
    ret =edt_ft5426_ts_reset(ft5426);//每次挂载的时候执行复位
    if(ret)
    return ret;
    msleep(5);
    //初始化ft5426
    data =0;
    edt_ft5426_ts_write(ft5426,FT5426_DEVIDE_MODE_REG,&data,1);
    data =1;
    edt_ft5426_ts_write(ft5426,FT5426_ID_G_MODE_REG,&data,1);

    //申请/注册中断服务函数
    ret =edt_ft5426_ts_irq(ft5426);
    if(ret)
    return ret;

    //注册input设备，首先是申请input结构体，然后是配置结构体，再注册回去
    input =devm_input_allocate_device(&client->dev);
    if(!input){
        dev_err(&client->dev, "Failed to allocate input device.\n");
        return -ENOMEM;
    }
    ft5426->input =input;
    input->name ="FocalTech FT5426 TouchScreen";
    input->id.bustype =BUS_I2C;//这里怎么总线类型还写出来

    //接下来是设置绝对数据的范围
    input_set_abs_params(input, ABS_MT_POSITION_X, 0, 1024, 0, 0);
 input_set_abs_params(input, ABS_MT_POSITION_Y, 0, 600, 0, 0);
    ret =input_mt_init_slots(input,MAX_SUPPORT_POINTS,INPUT_MT_DIRECT);//初始化MT
if (ret) {
 dev_err(&client->dev, "Failed to init MT slots.\n");
 return ret;
 }

 ret =input_register_device(input);
 if(ret)return ret;
 i2c_set_clientdata(client, ft5426);//将ft5426这个我们定义的结构体存入client中，
 return 0;
 }//input结构体存入我们定义的结构体中，然后再存入内核中的client中。

 static int edt_ft5426_ts_remove(struct i2c_client *client){
    struct edt_ft5426_dev *ft5426 =i2c_get_clientdata(client);
    input_unregister_device(ft5426->input);
    return 0;
 }

 static const struct of_device_id edt_ft5426_of_match[] = {
 { .compatible = "edt,edt-ft5426", },
 { /* sentinel */ }
 };
  static struct i2c_driver edt_ft5426_ts_driver = {
 .driver = {
 .owner = THIS_MODULE,
 .name = "edt_ft5426",
 .of_match_table = of_match_ptr(edt_ft5426_of_match),
 },
 .probe = edt_ft5426_ts_probe,
 .remove = edt_ft5426_ts_remove,
 };
 module_i2c_driver(edt_ft5426_ts_driver);
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("ALIENTEK");
 MODULE_INFO(intree, "Y");