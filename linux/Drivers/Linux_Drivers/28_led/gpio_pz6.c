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
 #include <linux/irq.h>
 #include <linux/wait.h>
 #include <linux/poll.h>
 #include <linux/fs.h>
 #include <linux/fcntl.h>
 #include <linux/platform_device.h>
 #include <asm/mach/map.h>
 #include <asm/uaccess.h>
 #include <asm/io.h>

 #define GPIOPZ6_CNT 1 /* 设备号长度 */
 #define GPIOPZ6_NAME "pz6" /* 设备名字 */
 #define PZ6OFF 0
 #define PZ6ON 1

 struct gpiopz6_dev{
 dev_t devid; /* 设备号 */
 struct cdev cdev; /* cdev*/
 struct class *class; /* 类 */
 struct device *device; /* 设备 */ 
 struct device_node *node; /* LED 设备节点 */
 int gpio_pz6; /* LED 灯 GPIO 标号 */
 };