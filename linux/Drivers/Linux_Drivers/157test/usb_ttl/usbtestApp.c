#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h> 
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>



/*设置串口参数*/
int init_tty(int fd)
{
	struct termios termios_rfid;/*这是一个常规的终端接口
	用于控制非同步通信端口，这个东西压根就没接触过*/
	
	bzero(&termios_rfid, sizeof(termios_rfid));//清空结构体
	/*将终端配置位原始模式*/
	cfmakeraw(&termios_rfid);//设置终端属性，激活选项
	/*POSIX规定波特率存储在termios结构体和宗，但是位精确指定他的位置
	规定在结构体中但是找不到具体位置，通过下面两行函数存取*/
	cfsetispeed(&termios_rfid, B115200);//输入波特率
	cfsetospeed(&termios_rfid, B115200);//输出波特率
	/*这个c_cflag是控制模式的标志位
	下面一行表示忽略modem控制线，打开接受者*/
	termios_rfid.c_cflag |= CLOCAL | CREAD;//本地连接和接收使能
	
	termios_rfid.c_cflag &= ~CSIZE;//清空数据位
	termios_rfid.c_cflag |= CS8;//数据位为8位
	
	termios_rfid.c_cflag &= ~PARENB;//无奇偶校验
	
	termios_rfid.c_cflag &= ~CSTOPB;//一位停止位
	/*上面都是对c_cflag标志的操作*/
	tcflush(fd,TCIFLUSH);//此函数会清空输入/输出缓冲区的数据
	/*下面就是特殊控制字符*/
	termios_rfid.c_cc[VTIME] = 0;//设置等待时间
	termios_rfid.c_cc[VMIN] = 0;
	
	tcflush(fd,TCIFLUSH);//对接受到而未被读取到的数据进行清空处理
	
	if(tcsetattr(fd, TCSANOW, &termios_rfid))//将配置参数写入到终端设备，使其生效
		return 0;
	
	return 1;
}

int main(void)
{
	int fd;
	char ch[]="good job!\r\n";
	int buf[200] ={0};
	int nread;
		fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);//O_NOCTTY标志不会成为进程的控制终端
		if(fd < 0)
		{
			printf("open stm32f103_serial1 failed!\n");
		}	
	if(fcntl(fd, F_SETFL, 0)<0)//将进程设置为阻塞
        printf("fcntl failed!\n");
    else
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));


		if(init_tty(fd) == -1)//初始化串口
		{
			printf("init_tty in failed!\n");
		}else{
			printf("init_tty successful!\n");
		}
		

	while(1)
	{
		write(fd, ch, sizeof(ch));//写入
		tcflush(fd, TCOFLUSH);//对尚未传输成功的输出数据进行清空处理
	
		nread =read(fd, buf, 3);
		printf("nread=%d, data =%x",nread,*buf);
		tcflush(fd, TCOFLUSH);
		memset(buf, 0x0, sizeof(buf));
		sleep(1);
	}
close(fd);

	exit(1);
}
