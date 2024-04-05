#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
/* 五个参量 fd打开文件 speed设置波特率 bit数据位设置   neent奇偶校验位 stop停止位 */
    struct termios newtio,oldtio;
    if ( tcgetattr( fd,&oldtio) != 0) {
        perror("SetupSerial 1");
        exit(-1);
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag |= CLOCAL | CREAD;/*忽略调试解调器的
    控制线，使能接受*/
    /*下面两行是设置数据未为8位*/
    newtio.c_cflag &= ~CSIZE;
    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
    break;
    case 8:
        newtio.c_cflag |= CS8;
    break;
    }
    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);//对接收到的数据进行奇偶校验，去除第八位
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;//使能奇偶校验
        newtio.c_cflag &= ~PARODD;//使用奇校验，不使用偶校验
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }
switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
         cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if( nStop == 1 )
        newtio.c_cflag &= ~CSTOPB;//一个停止位
    else if ( nStop == 2 )
    newtio.c_cflag |= CSTOPB;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)//配置好的参数传给终端
    {
        perror("com set error");
        exit(-1);
    }
    printf("set done!\n");
    return 0;
}
int open_port(int fd,int comport)
{
/* fd 打开串口 comport表示第几个串口 */
    char *dev[]={"/dev/ttyUSB0","/dev/ttyS1","/dev/ttyS2"};
    long vdisable;
    if (comport==1)
    {    fd = open( "/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
        if (-1 == fd){
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
            printf("open ttyUSB0 .....\n");
    }
    else if(comport==2)
    {    fd = open( "/dev/ttyS1", O_RDWR|O_NOCTTY|O_NDELAY);
 
    if (-1 == fd){
        perror("Can't Open Serial Port");
            return(-1);
        }
        else
            printf("open ttyS1 .....\n");
    }
    else if (comport==3)
    {
        fd = open( "/dev/ttyS2", O_RDWR|O_NOCTTY|O_NDELAY);
        if (-1 == fd){
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
            printf("open ttyS2 .....\n");
    }
    if(fcntl(fd, F_SETFL, 0)<0)//将进程设置为阻塞
        printf("fcntl failed!\n");
    else
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
    if(isatty(STDIN_FILENO)==0)//检查下我们打开的文件是不是一个终端
        printf("standard input is not a terminal device\n");
    else
        printf("isatty success!\n");
    printf("fd-open=%d\n",fd);
    return fd;
}
int main(void)
{
    int fd;
    int nread,i;
    int buff[200]={0};
    char ch[]="good job!\r\n";
    if((fd=open_port(fd,1))<0){
        perror("open_port error");
        exit(-1);
    }
    if((i=set_opt(fd,115200,8,'N',1))<0){
    perror("set_opt error");
        exit(-1);
    }
    printf("fd=%d\n",fd);
//    fd=3;
while(1){
    write(fd, ch, sizeof(ch));
    tcflush(fd,TCOFLUSH);
    nread=read(fd,buff,3);
    printf("nread=%d,%x\n",nread,*buff);
        tcflush(fd,TCIFLUSH);
memset(buff,0x0,sizeof(buff));
    sleep(1);
}
printf("按下ctrl+c");
    close(fd);
    exit(1);
 
}