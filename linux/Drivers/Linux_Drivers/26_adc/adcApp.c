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

 #define SENSOR_FLOAT_DATA_GET(ret, index, str, member)\
 ret = file_data_read(file_path[index], str);\
 dev->member = atof(str);\

  #define SENSOR_INT_DATA_GET(ret, index, str, member)\
 ret = file_data_read(file_path[index], str);\
 dev->member = atoi(str);\

 static char *file_path[] = {
 "/sys/bus/iio/devices/iio:device0/in_voltage_scale",
 "/sys/bus/iio/devices/iio:device0/in_voltage19_raw",
 };

 enum path_index {
IN_VOLTAGE_SCALE =0,
IN_VOLTAGE_RAW,
 };

 struct adc_dev{
    int raw;//要读取的院士值
    float scale;//分辨率
    float act;
 };

 struct adc_dev stm32adc;

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

 static int adc_read(struct adc_dev *dev)
 {
    int ret =0;
    char str[50];
    SENSOR_FLOAT_DATA_GET(ret, IN_VOLTAGE_SCALE, str, scale);
    SENSOR_INT_DATA_GET(ret, IN_VOLTAGE_RAW, str, raw);
    dev->act = (dev->scale * dev->raw)/1000.f;
     return ret;
 }

 int main(int argc, char *argv[])
 {
 int ret = 0;

 if (argc != 1) {
 printf("Error Usage!\r\n");
 return -1;
 }

 while (1) {
 ret = adc_read(&stm32adc);
 if(ret == 0) { /* 数据读取成功 */
 printf("ADC院士值:%d,电压值:%.3fV\\r\n",stm32adc.raw,stm32adc.act);
 }
 usleep(100000); /*100ms */
 }

 return 0;
 }