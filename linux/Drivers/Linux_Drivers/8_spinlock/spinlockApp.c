#include "stdio.h"
 #include "unistd.h"
 #include "sys/types.h"
 #include "sys/stat.h"
 #include "fcntl.h"
 #include "stdlib.h"
 #include "string.h"

 #define LEDOFF 0
 #define LEDON 1

 int main(int argc,char *argv[])
 {
    int fd,retvalue;
    char *filename;
    unsigned char cnt = 0;
    unsigned char databuf[1];

    if(argc!=3){
        printf("Error Usage!\r\n");
        return -1;
    }
    filename =argv[1];
    //打开beep驱动
    fd =open(filename,O_RDWR);
    if(fd<0){
        printf("file %s open failed!\r\n",argv[1]);
        return -1;
    }

    databuf[0] = atoi(argv[2]);
    //向dev/gpioled文件写入数据
    retvalue = write(fd,databuf,sizeof(databuf));
    if(retvalue<0){
        printf("LED Control Failed!\r\n");
        close(fd);
        return -1;
    }

    //模拟占用LED
    while(1){
        sleep(5);
        cnt++;
        printf("App runing times:%d\r\n",cnt);
        if(cnt>=5)break;
    }
    printf("App running finished!");
    retvalue = close(fd);
    if(retvalue<0)
    {
        printf("file %s close failed!\r\n",argv[1]);
        return -1;
    }
    return 0;
    //这个代码是先打开文件，然后写入数据，延迟一段时间后关闭。
 }