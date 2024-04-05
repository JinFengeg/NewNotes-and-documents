#include "stdio.h"
 #include "unistd.h"
 #include "sys/types.h"
 #include "sys/stat.h"
 #include "fcntl.h"
 #include "stdlib.h"
 #include "string.h"

 /* 定义按键值 ,不是运行在内核太所以只能自己设定了*/
 #define KEY0VALUE 0XF0
 #define INVAKEY 0X00

 int main(int argc,char *argv[]) {
    int fd,ret;
    char*filename;
    int keyvalue;
    if(argc !=2){
        printf("Error Usage!\r\n");
        return -1;
    }
filename = argv[1];
fd = open(filename,O_RDWR);
if(fd<0){
    printf("file %s open    failed!\r\n",argv[1]);
    return -1;
}

while(1){
    read(fd,&keyvalue,sizeof(keyvalue));
if (keyvalue == KEY0VALUE) { /* KEY0 */
 printf("KEY0 Press, value = %#X\r\n", keyvalue);/* 按下 */
 }
}
ret= close(fd); /* 关闭文件 */
if(ret < 0){
     printf("file %s close failed!\r\n", argv[1]);
      return -1;
 }
 return 0;
 }