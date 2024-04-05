#include <wiringSerial.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include "config.h"
#include <fstream>
#include <iostream>
using namespace std;

static int IMU_OL_fd;
static ofstream imu_file;
static const int botelv = 230400;
void init_IMU()
{
#ifdef IMU_MODE_OL
	for(int i=0;i<1;i++)
	{
		if ((IMU_OL_fd = serialOpen("/dev/ttyUSB0", botelv)) < 0)
		{
			printf("open USB0 serial error!\n");
			exit(1);
		}
	}

	int bite_rec = 0;
	unsigned char CMD[8] = {0x50,0x03,0x00,0x34,0x00,0x0C,0x09,0x80};
	unsigned char buff[100] = {0};
	serialFlush(IMU_OL_fd);
	write(IMU_OL_fd, CMD, 8);
	bite_rec = read(IMU_OL_fd, buff, 29);
	if(bite_rec>0)
	{
		printf("open IMU_USB0 success!\n");
		serialFlush(IMU_OL_fd);
	}
	else
	{
		serialClose(IMU_OL_fd);
		IMU_OL_fd = -1;
		if ((IMU_OL_fd = serialOpen("/dev/ttyUSB1", botelv)) < 0)
		{
			printf("open IMU_USB1 serial error!\n"); 
			exit(1);
		}
		serialFlush(IMU_OL_fd);
		write(IMU_OL_fd, CMD, 8);
		bite_rec = read(IMU_OL_fd, buff, 29);
		if(bite_rec > 0)
		{
			printf("open IMU_USB1 success!\n");
		 	serialFlush(IMU_OL_fd);
		}
	}
	
#endif



}

void IMU_getdata(float data[8][12])
{
#ifdef IMU_MODE_OL
	static unsigned char CMD[8][8] = {{0x50,0x03,0x00,0x34,0x00,0x0C,0x09,0x80},\
	 {0x51,0x03,0x00,0x34,0x00,0x0C,0x08,0x51},\
	 {0x52,0x03,0x00,0x34,0x00,0x0C,0x08,0x62},\
	 {0x53,0x03,0x00,0x34,0x00,0x0C,0x09,0xB3},\
	 {0x54,0x03,0x00,0x34,0x00,0x0C,0x08,0x04},\
	 {0x55,0x03,0x00,0x34,0x00,0x0C,0x09,0xD5},\
	 {0x56,0x03,0x00,0x34,0x00,0x0C,0x09,0xE6},\
	 {0x57,0x03,0x00,0x34,0x00,0x0C,0x08,0x37} };
	static unsigned char CMD2[8][8] = {{0x50,0x03,0x00,0x51,0x00,0x04,0x18,0x59},\
	{0x51,0x03,0x00,0x51,0x00,0x04,0x19,0x88},\
	{0x52,0x03,0x00,0x51,0x00,0x04,0x19,0xBB},\
	{0x53,0x03,0x00,0x51,0x00,0x04,0x18,0x6A},\
	{0x54,0x03,0x00,0x51,0x00,0x04,0x19,0xDD},\
	{0x55,0x03,0x00,0x51,0x00,0x04,0x18,0x0C},\
	{0x56,0x03,0x00,0x51,0x00,0x04,0x18,0x3F},\
	{0x57,0x03,0x00,0x51,0x00,0x04,0x19,0xEE}};
	static unsigned char tmp_buf[100];
	for (int _CH = 0; _CH < IMU_CH; _CH++)
	{
		// CMD[0] = 0x50 + _CH;
		// CMD2[0] = 0x50 + _CH;
		serialFlush(IMU_OL_fd);
		write(IMU_OL_fd, CMD[_CH], 8);
		read(IMU_OL_fd, tmp_buf, 29);
                //printf("read %d imu ok\n",_CH);
		//加速度
		data[_CH][0] = (float)((short)((tmp_buf[3] << 8) | tmp_buf[4])) / 32768.0 * 16.0;  //xx g  (g=9.8m/s2)
		data[_CH][1] = (float)((short)((tmp_buf[5] << 8) | tmp_buf[6])) / 32768.0 * 16.0;  //xx g  (g=9.8m/s2)
		data[_CH][2] = (float)((short)((tmp_buf[7] << 8) | tmp_buf[8])) / 32768.0 * 16.0;  //xx g  (g=9.8m/s2)																		   
		//角速度
		data[_CH][3] = (float)((short)((tmp_buf[9] << 8) | tmp_buf[10])) / 32768 * 2000;
		data[_CH][4] = (float)((short)((tmp_buf[11] << 8) | tmp_buf[12])) / 32768 * 2000;
		data[_CH][5] = (float)((short)((tmp_buf[13] << 8) | tmp_buf[14])) / 32768 * 2000;
		//磁场
		data[_CH][6] = (float)((short)((tmp_buf[15] << 8) | tmp_buf[16]));
		data[_CH][7] = (float)((short)((tmp_buf[17] << 8) | tmp_buf[18]));
		data[_CH][8] = (float)((short)((tmp_buf[19] << 8) | tmp_buf[20]));
		//角度
		data[_CH][9] = (float)((short)((tmp_buf[21] << 8) | tmp_buf[22])) / 32768 * 180;
		data[_CH][10] = (float)((short)((tmp_buf[23] << 8) | tmp_buf[24])) / 32768 * 180;
		data[_CH][11] = (float)((short)((tmp_buf[25] << 8) | tmp_buf[26])) / 32768 * 180;
        /*
		write(IMU_OL_fd, CMD2[_CH], 8);
		read(IMU_OL_fd, tmp_buf, 13);
		//四元数
		data[_CH][6] = (float)((short)((tmp_buf[3] << 8) | tmp_buf[4])) / 32768.0;
		data[_CH][7] = (float)((short)((tmp_buf[5] << 8) | tmp_buf[6])) / 32768.0;
		data[_CH][8] = (float)((short)((tmp_buf[7] << 8) | tmp_buf[8])) / 32768.0;
		data[_CH][9] = (float)((short)((tmp_buf[9] << 8) | tmp_buf[10])) / 32768.0;
        */
	}
#else
#endif
}

void init_file(){

    imu_file.open("./file_IMU.csv");
    if(!imu_file.is_open())
    {
        cout<<"open tcp file error\n"<<endl;
		exit(1);
    }
    imu_file<<"beginsec,beginusec";
   
	for (int i=0;i<IMU_CH;i++)
	{
	    imu_file<<",ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw";
	}
    imu_file<<",endsec,endusec";
	imu_file<<endl;

}
static void SigH(int Sig)
{
	switch (Sig)
	{
	case SIGINT:
    {
        imu_file.close();
        serialClose(IMU_OL_fd);
        cout<<"close file"<<endl;
        exit(0);
		break;
    }
	default:
		break;
	}
}
static void init_sig()
{
	signal(SIGINT, SigH);
}
int main(int argc, char** argv)
{
    init_sig();
    init_file();
    init_IMU();
    timeval time_begin;
    float time;
    timeval time_end;
    float IMU_DATA[8][12]={ {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} };
    while(1)
    {
        gettimeofday(&time_begin, NULL);
        IMU_getdata(IMU_DATA);
        gettimeofday(&time_end, NULL);

        imu_file<<time_begin.tv_sec<<","<<time_begin.tv_usec<<",";
        for(int _ch =0 ;_ch<IMU_CH;++_ch){
            for(int n=0;n<12;++n){
                imu_file<<IMU_DATA[_ch][n]<<",";
            }
        }
        imu_file<<time_end.tv_sec<<","<<time_end.tv_usec<<",";
        time = (float)(1000 * (time_end.tv_sec - time_begin.tv_sec) + (time_end.tv_usec - time_begin.tv_usec)/1000.0);
        imu_file<<time;
        imu_file<<endl;

    }
}
