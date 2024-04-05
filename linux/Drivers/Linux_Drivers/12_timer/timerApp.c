#include "stdio.h"
 #include "unistd.h"
 #include "sys/types.h"
 #include "sys/stat.h"
 #include "fcntl.h"
 #include "stdlib.h"
  #include "string.h"
#include <sys/ioctl.h>

//命令值
#define CLOSE_CMD   (_IO(0XEF,0X1))
#define  OPEN_CMD   (_IO(0XEF,0x2))
#define  SETPERIOD_CMD   (_IO(0XEF,0X3))

int main(int argc,char *argv[])
{
    int fd,ret;
    char *filename;
    unsigned int cmd;
    unsigned int arg;
    unsigned char str[100];
    if(argc !=2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    filename = argv[1];
    fd = open(filename,O_RDWR);
    if (fd < 0) {
 printf("Can't open file %s\r\n", filename);
 return -1;
    }
    //接下来就是往里传入参数了，通过命令行输入的数字决定要传入的命令
    while(1){
        printf("Input CMD: ");
        ret = scanf("%d",&cmd);
        if(ret !=1){
            //如果参数传入cmd错误的话，
            fgets(str,sizeof(str),stdin);
        }
        if(4 == cmd)
        goto out;
        if(cmd == 1)
        cmd = CLOSE_CMD;
        else if(cmd == 2)
        {
            cmd = OPEN_CMD;
        }else if(cmd ==3)
        {
            cmd =SETPERIOD_CMD;
            printf("Input Timer Period:");
            ret = scanf("%d",&arg);//这里把用户设置的定时器周期传入
            if(ret !=1){
                fgets(str,sizeof(str),stdin);
            }
        }
        ioctl(fd,cmd,arg);//这里把定时器参数传入驱动程序中
    }
    out:
    close(fd);
}


