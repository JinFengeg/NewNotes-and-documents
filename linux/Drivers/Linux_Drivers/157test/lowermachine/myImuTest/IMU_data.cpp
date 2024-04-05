#include <wiringSerial.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "IMU_data.h"
#include "config.h"



int init_IMU(const char path[])
{
	int IMU_FD = -1;
	for(int i=0;i<1;i++)
	{
		if ((IMU_FD = serialOpen(path, 921600)) < 0)
		{
			printf("open USB0 serial error!\n");
			exit(1);
		}
		else{
			printf("open %s success\n",path);
		}
	}

	// int bite_rec = 0;
	// unsigned char CMD[8] = {0x50,0x03,0x00,0x34,0x00,0x0C,0x09,0x80};
	// unsigned char buff[100] = {0};
	// serialFlush(IMU_FD);
	// write(IMU_FD, CMD, 8);
	// bite_rec = read(IMU_FD, buff, 29);
	// if(bite_rec>0)
	// {
	// 	printf("open IMU_USB0 success!\n");
	// 	serialFlush(IMU_FD);
	// }
	// else
	// {
	// 	serialClose(IMU_FD);
	// 	IMU_FD = -1;
	// 	if ((IMU_FD = serialOpen(path, 921600)) < 0)
	// 	{
	// 		printf("open IMU_USB1 serial error!\n"); 
	// 		exit(1);
	// 	}
	// 	serialFlush(IMU_FD);
	// 	write(IMU_FD, CMD, 8);
	// 	bite_rec = read(IMU_FD, buff, 29);
	// 	if(bite_rec > 0)
	// 	{
	// 		printf("open IMU_USB1 success!\n");
	// 	 	serialFlush(IMU_FD);
	// 	}
	// }
	return IMU_FD;
}

void IMU_getdata(int ch_num,int imu_fd,int base,float data[8][12])
{
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
	for (int _CH = 0; _CH < ch_num; _CH++)
	{
		serialFlush(imu_fd);
		write(imu_fd, CMD[_CH+base], 8);
		read(imu_fd, tmp_buf, 29);
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
		
	}
}

void* thread_imu(void* IMU_config){
	IMU_CFG* imuConfig = (IMU_CFG*) IMU_config;
	float temp_data[8][12];
	while(1)
	{
		if(imuConfig->flag == 1){
			break;
		}
		IMU_getdata(imuConfig->ch,imuConfig->imu_fd,imuConfig->base,temp_data);

		pthread_mutex_lock(&(imuConfig->mptr));
		for(int _ch=0;_ch<imuConfig->ch;++_ch){
			for(int i=0;i<12;++i){
				imuConfig->data[_ch][i] = temp_data[_ch][i];
				imuConfig->imu_file<<temp_data[_ch][i]<<",";
			}
		}
		imuConfig->imu_file<<endl;
		pthread_mutex_unlock(&(imuConfig->mptr));
	}
	printf("thread_imu exit\n");
	return NULL;
}
