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
 "/sys/bus/iio/devices/iio:device2/in_voltage_scale",
 };

 enum path_index {
 in_voltage0_raw = 0,
 in_voltage_scale,
 };

 struct ad7606_dev{
    int in_voltage0_raw;
    float in_voltage_scale;
    float in_voltage0_act;
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
 SENSOR_FLOAT_DATA_GET(ret, in_voltage_scale, str, in_voltage_scale);
 SENSOR_INT_DATA_GET(ret, in_voltage0_raw, str, in_voltage0_raw);

 dev->in_voltage0_act =dev->in_voltage0_raw *dev->in_voltage_scale;
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
 //if(ret == 0) { /* 数据读取成功 */
 /*printf("\r\n 原始值:\r\n");
 printf("in_voltage0_raw = %d",ad7606.in_voltage0_raw);
 printf("\r\n实际值:\r\n");
 printf("in_voltage0_act = %f\n",ad7606.in_voltage0_act);
 printf("\r\n量程:\r\n");
 printf("in_voltage_scale =%f",ad7606.in_voltage_scale);
 }else{
   printf("read error\r\n");
 }*/
 usleep(100000);
 }
 return 0;
 }