#include <stdio.h>
#include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <stdlib.h>
 #include <string.h>
#include <poll.h>
 int main(int argc, char *argv[])
{
 int fd, ret;
 int key_val;
fd_set readfds;

 /* 判断传参个数是否正确 */
 if(2 != argc) {
 printf("Usage:\n"
 "\t./keyApp /dev/key\n"
 );
 return -1;
 }

 /* 打开设备 */
 fd = open(argv[1], O_RDONLY|O_NONBLOCK);
 if(0 > fd) {
    printf("ERROR: %s file open failed!\n", argv[1]);
    return -1;
 }
FD_ZERO(&readfds);
FD_SET(fd,&readfds);//这里是将我们要判断的文件描述符之一
 /* 循环读取按键数据 */
 for ( ; ; ) {
ret = select(fd+1,&readfds,NULL,NULL,NULL);//检查下是否有可读取的文件
 switch(ret){
   case 0:
   printf("over time");
   break;
   case -1:
   printf("ERROR");
   break;
   default:
   if(FD_ISSET(fd,&readfds)){
read(fd, &key_val, sizeof(int));

 if (0 == key_val)
 printf("Key Press\n");
 else if (1 == key_val)
 printf("Key Release\n");
   }
   break;
 }
 }

 /* 关闭设备 */
 close(fd);
 return 0;
 }