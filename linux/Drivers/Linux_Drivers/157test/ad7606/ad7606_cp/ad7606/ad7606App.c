#include "stdio.h"
 #include "unistd.h"
 #include "sys/types.h"
 #include "sys/stat.h"
 #include "sys/ioctl.h"
 #include "fcntl.h"
 #include "stdlib.h"
 #include "string.h"
 #include <poll.h>
 #include <sys/select.h>
 #include <sys/time.h>
 #include <signal.h>
 #include <fcntl.h>
 #include <errno.h>

 /* 字符串转数字，将浮点小数字符串转换为浮点数数值 */
 #define SENSOR_FLOAT_DATA_GET(ret, index, str, member)\
 ret = file_data_read(file_path[index], str);\
 dev->member = atof(str);\
 
 /* 字符串转数字，将整数字符串转换为整数数值 */
 #define SENSOR_INT_DATA_GET(ret, index, str, member)\
 ret = file_data_read(file_path[index], str);\
 dev->member = atoi(str);\

 static char *file_path[] = {
 "/sys/bus/iio/devices/iio:device2/in_voltage0_raw",
  "/sys/bus/iio/devices/iio:device2/in_voltage1_raw",
   "/sys/bus/iio/devices/iio:device2/in_voltage2_raw",
    "/sys/bus/iio/devices/iio:device2/in_voltage3_raw",
     "/sys/bus/iio/devices/iio:device2/in_voltage4_raw",
      "/sys/bus/iio/devices/iio:device2/in_voltage5_raw",
       "/sys/bus/iio/devices/iio:device2/in_voltage6_raw",
        "/sys/bus/iio/devices/iio:device2/in_voltage7_raw",
 "/sys/bus/iio/devices/iio:device2/in_voltage_scale",
 };

 enum path_index {
 voltage0_raw = 0,
 voltage1_raw,
  voltage2_raw,
voltage3_raw,
  voltage4_raw,
  voltage5_raw,
   voltage6_raw,
  voltage7_raw,
 voltage_scale,
 };

 struct ad7606_dev{
    int in_voltage0_raw;
    int in_voltage1_raw;
    int in_voltage2_raw;
    int in_voltage3_raw;
    int in_voltage4_raw;
    int in_voltage5_raw;
    int in_voltage6_raw;
    int in_voltage7_raw;
    float in_voltage_scale;
    float in_voltage0_act;
    float in_voltage1_act;
    float in_voltage2_act;
    float in_voltage3_act;
    float in_voltage4_act;
    float in_voltage5_act;
    float in_voltage6_act;
    float in_voltage7_act;
 };

 struct ad7606_dev ad7606;

 static int file_data_read(char *filename, char *str)
{
    int ret = 0;
    FILE *data_stream;
    data_stream = fopen(filename, "r"); /* 只读打开 */
    if(data_stream == NULL) {
    printf("can't open file %s\r\n", filename);
    return -1;
    }
    ret = fscanf(data_stream, "%s", str);
    if(!ret) {
    printf("file read error!\r\n");
    } else if(ret == EOF) {
    /* 读到文件末尾的话将文件指针重新调整到文件头 */
    fseek(data_stream, 0, SEEK_SET); 
    }
    fclose(data_stream); /* 关闭文件 */ 
    return 0;
 }

 static int sensor_read(struct ad7606_dev *dev)
 {
 int ret = 0;
 char str[50];

 /* 1、获取陀螺仪原始数据 */
 SENSOR_FLOAT_DATA_GET(ret, voltage_scale, str, in_voltage_scale);
 SENSOR_INT_DATA_GET(ret, voltage0_raw, str, in_voltage0_raw);
 SENSOR_INT_DATA_GET(ret, voltage1_raw, str, in_voltage1_raw);
SENSOR_INT_DATA_GET(ret, voltage2_raw, str, in_voltage2_raw);
SENSOR_INT_DATA_GET(ret, voltage3_raw, str, in_voltage3_raw);
 SENSOR_INT_DATA_GET(ret, voltage4_raw, str, in_voltage4_raw);
   SENSOR_INT_DATA_GET(ret, voltage5_raw, str, in_voltage5_raw);
   SENSOR_INT_DATA_GET(ret, voltage6_raw, str, in_voltage6_raw);
    SENSOR_INT_DATA_GET(ret, voltage7_raw, str, in_voltage7_raw);

 dev->in_voltage0_act =dev->in_voltage0_raw *dev->in_voltage_scale;
 dev->in_voltage1_act =dev->in_voltage1_raw *dev->in_voltage_scale;
 dev->in_voltage2_act =dev->in_voltage2_raw *dev->in_voltage_scale;
 dev->in_voltage3_act =dev->in_voltage3_raw *dev->in_voltage_scale;
 dev->in_voltage4_act =dev->in_voltage4_raw *dev->in_voltage_scale;
 dev->in_voltage5_act =dev->in_voltage5_raw *dev->in_voltage_scale;
 dev->in_voltage6_act =dev->in_voltage6_raw *dev->in_voltage_scale;
 dev->in_voltage7_act =dev->in_voltage7_raw *dev->in_voltage_scale;
 return ret;
 }

 int main(int argc, char *argv[]){
    int ret = 0;
    if (argc != 1) {
 printf("Error Usage!\r\n");
 return -1;
 }

 while (1) {
 ret = sensor_read(&ad7606);
 if(ret == 0) {
 printf("\r\n 原始值:\r\n");
 printf("in_voltage0_raw = %d\n",ad7606.in_voltage0_raw);
  printf("in_voltage1_raw = %d\n",ad7606.in_voltage1_raw);
   printf("in_voltage2_raw = %d\n",ad7606.in_voltage2_raw);
    printf("in_voltage3_raw = %d\n",ad7606.in_voltage3_raw);
     printf("in_voltage4_raw = %d\n",ad7606.in_voltage4_raw);
      printf("in_voltage5_raw = %d\n",ad7606.in_voltage5_raw);
 printf("in_voltage6_raw = %d\n",ad7606.in_voltage6_raw);
       printf("in_voltage7_raw = %d\n",ad7606.in_voltage7_raw);
 printf("\r\n实际值:\r\n");
 printf("in_voltage0_act = %f\n",ad7606.in_voltage0_act);
 printf("in_voltage1_act = %f\n",ad7606.in_voltage1_act);
 printf("in_voltage2_act = %f\n",ad7606.in_voltage2_act);
printf("in_voltage3_act = %f\n",ad7606.in_voltage3_act);
printf("in_voltage4_act = %f\n",ad7606.in_voltage4_act);
printf("in_voltage5_act = %f\n",ad7606.in_voltage5_act);
printf("in_voltage6_act = %f\n",ad7606.in_voltage6_act);
 printf("in_voltage7_act = %f\n",ad7606.in_voltage7_act);
 printf("\r\n量程:\r\n");
 printf("in_voltage_scale =%f\n",ad7606.in_voltage_scale);
 }else{
   printf("read error\r\n");
 }
 sleep(1);
 }
 return 0;
 }