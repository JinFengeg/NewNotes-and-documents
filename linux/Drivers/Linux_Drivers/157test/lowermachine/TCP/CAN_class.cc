#include "tensorflow/lite/examples/LowerMachine/CAN_class.h"
#include <iostream>
#include <stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<cstring>
CAN_class::CAN_class()
{
    init_flag = false;
    serial_fd = 0;
}

int CAN_class::init()
{
    char buff[20] = {0};
	if ((serial_fd = serialOpen("/dev/ttyUSB1", 115200)) < 0)
	{
		printf("open  can_USB1 serial error!\n");
		exit(1);
	}
	char test_CMD[7] = {0x41,0x54,0x2B,0x45,0x54,0x0D,0x0A};
	serialFlush(serial_fd);
	write(serial_fd,test_CMD,7);
	int bitesc=read(serial_fd,buff,20);
	if(bitesc > 0)
	{
		serialFlush(serial_fd);
		printf("Open CAN_USB1 serial success!\n");
		printf("%d,,,,,%s\n",bitesc,buff);
		
	}
	else
	{//此时打开的串口为IMU 串口 更改为打开CAN
		serialClose(serial_fd);
		serial_fd = -1;
		if ((serial_fd = serialOpen("/dev/ttyUSB0", 115200)) < 0)
		{
			printf("open CAN_USB0 serial error!\n");
			exit(1);
		}
		serialFlush(serial_fd);
		write(serial_fd,test_CMD,7);
		bitesc=read(serial_fd,buff,20);
		if(bitesc > 0)
		{
			serialFlush(serial_fd);
			printf("Open CAN_USB0 serial success!\n");
			printf("%d,,,,,%s\n",bitesc,buff);
			
		}
	}


    init_flag = true;

    
    return 1;
}

void CAN_class::send_data(char data[],int count)
{
    if(init_flag)
    {
        serialFlush(serial_fd);
        write(serial_fd, data, count);
    }

}