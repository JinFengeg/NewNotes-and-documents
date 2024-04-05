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


 struct ad7606_dev{
    unsigned short data[8];
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

 void ad7606_trigger_set(void)
 {
      //system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_timestamp_en");
      system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_voltage0_en");
      system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_voltage1_en");
      system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_voltage2_en");
      system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_voltage3_en");
      system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_voltage4_en");
      system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_voltage5_en");
      system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_voltage6_en");
      system("echo 1 >/sys/bus/iio/devices/iio:device2/scan_elements/in_voltage7_en");

    system("echo ad7606-8-dev2 > /sys/bus/iio/devices/iio:device2/trigger/current_trigger");
    system("echo 18 > /sys/bus/iio/devices/iio:device2/buffer/length");
    system("echo 1 > /sys/bus/iio/devices/iio:device2/buffer/enable");
 }

 static int file_data_read(char *filename, char *str)
 {
 int ret = 0;
 FILE *data_stream;
 
 data_stream = fopen(filename, "r"); 
 if(data_stream == NULL) {
 printf("can't open file %s\r\n", filename);
 return -1;
 }
 
 ret = fscanf(data_stream, "%s", str);
 if(!ret) {
 printf("file read error!\r\n");
 } else if(ret == EOF) {

 fseek(data_stream, 0, SEEK_SET); 
 }
 fclose(data_stream);
 return 0;
 }

 int ad7606_read(int fd, struct ad7606_dev *dev)
 {
 int ret = 0;
 char str[50];
 int i = 0;
 
 file_data_read("/sys/bus/iio/devices/iio:device2/in_voltage_scale", str);
 dev->in_voltage_scale = atof(str);
 ret = read(fd, dev->data, 16);
 perror(" \n");
 dev->in_voltage0_raw = (signed short)(dev->data[0]);
 dev->in_voltage1_raw = (signed short)(dev->data[1]);
 dev->in_voltage2_raw = (signed short)(dev->data[2]);
 dev->in_voltage3_raw = (signed short)(dev->data[3]);
 dev->in_voltage4_raw = (signed short)(dev->data[4]);
 dev->in_voltage5_raw = (signed short)(dev->data[5]);
 dev->in_voltage6_raw = (signed short)(dev->data[6]);
 dev->in_voltage7_raw = (signed short)(dev->data[7]);

 dev->in_voltage0_act =dev->in_voltage0_raw * dev->in_voltage_scale;
 dev->in_voltage1_act =dev->in_voltage1_raw * dev->in_voltage_scale;
 dev->in_voltage2_act =dev->in_voltage2_raw * dev->in_voltage_scale;
 dev->in_voltage3_act =dev->in_voltage3_raw * dev->in_voltage_scale;
 dev->in_voltage4_act =dev->in_voltage4_raw * dev->in_voltage_scale;
 dev->in_voltage5_act =dev->in_voltage5_raw * dev->in_voltage_scale;
 dev->in_voltage6_act =dev->in_voltage6_raw * dev->in_voltage_scale;
 dev->in_voltage7_act =dev->in_voltage7_raw * dev->in_voltage_scale;
return ret;
 }

 int main(void) {
   int fd;
   int ret =0;


ad7606_trigger_set();
printf("successful set trigger\n");
fd = open("/dev/iio:device2", O_RDONLY);
 if(0 > fd) {
 printf("ERROR: %s file open failed!\n", "/dev/iio:device2");
 return -1;
 }
printf("successful open file\n");
 while (1) {
 ret = ad7606_read(fd,&ad7606);
 if(ret == 16) { /* 数据读取成功 */
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
   printf("read failed\r\n");
 }
usleep(100000);
 }
 }