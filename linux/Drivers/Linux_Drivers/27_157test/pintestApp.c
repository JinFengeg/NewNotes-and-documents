#include "stdio.h"
 #include "unistd.h"
 #include "sys/types.h"
 #include "sys/stat.h"
 #include "fcntl.h"
 #include "stdlib.h"
 #include "string.h"



 int main(void) {
    int fd,ret;

    int keyvalue;


fd = open("/dev/ad7606",O_RDWR);
if(fd<0){
    printf("file open failed!\r\n");
    return -1;
}

while(1){
    read(fd,&keyvalue,sizeof(keyvalue));
    sleep(1);
}
ret= close(fd); /* 关闭文件 */
if(ret < 0){
     printf("file close failed!\r\n");
      return -1;
 }
 return 0;
 }