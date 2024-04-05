#include "stdio.h"
 #include "unistd.h"
 #include "sys/types.h"
#include "sys/stat.h"
 #include "fcntl.h"
#include "stdlib.h"
 #include "string.h"


 #define LEDON 1
 #define LEDOFF 0

 int main(int argc, char *argv[])
 {
    int fd,retvalue;
    char *filename;
    unsigned char databuf[1];
    if(argc!=3)
    {
        printf("Error usage!\r\n");
        return -1;
    }
filename = argv[1];
fd = open(filename,O_RDWR);
if(fd<0)
{
    printf("Error opening %s!\r\n",filename);
    return -1;
}

databuf[0] = atoi(argv[2]);/*要执行的操作是打开还是关闭*/

retvalue = write(fd,databuf,sizeof(databuf));
if(retvalue<0)
{
    printf("fd = %d,databuf = %s",fd,databuf[0]);
    printf("LED Control Failed!\r\n");
 close(fd);
 return -1;
}

retvalue = close(fd); /* 关闭文件 */
 if(retvalue < 0){
 printf("file %s close failed!\r\n", argv[1]);
 return -1;
 }
 return 0;
 }