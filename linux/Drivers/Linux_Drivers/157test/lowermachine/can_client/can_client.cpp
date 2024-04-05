#include <iostream>
#include <stdlib.h>
#include<stdio.h>
#include<cstring>
#include<unistd.h>
#include<wiringPi.h>
#include<wiringSerial.h>
struct recv
{
    char label;
    float process_time;
};
int main()
{
    int can_fd=-1;
    recv result;
    char buff[20] = {0};
    if ((can_fd = serialOpen("/dev/ttyUSB2", 115200)) < 0)
	{
		printf("open  can_ serial error!\n");
		exit(1);
	}
    else
    {
        printf("open  can_ serial success!\n");
    }
    while(1)
    {
        usleep(100);
        memset(buff,0,20);
        int recv_bites = read(can_fd,buff,20);
        if(recv_bites > 0)
        {
            recv *result = (recv *)buff;
            std::cout<<"ans: "<<result->label<<std::endl;
            std::cout<<"time: "<<result->process_time<<std::endl;
        }

    }
}
