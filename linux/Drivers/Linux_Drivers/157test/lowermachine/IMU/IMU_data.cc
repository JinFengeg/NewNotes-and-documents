#include <wiringSerial.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "tensorflow/lite/examples/LowerMachine/IMU_data.h"
#include "tensorflow/lite/examples/LowerMachine/config.h"


int Baud = 460800;
int init_IMU(const char* path,int imu,int num)
{
	unsigned char tmp_buf[100];
	unsigned char CMD[8][8] = {{0x50,0x03,0x00,0x34,0x00,0x0C,0x09,0x80},\
	 {0x51,0x03,0x00,0x34,0x00,0x0C,0x08,0x51},\
	 {0x52,0x03,0x00,0x34,0x00,0x0C,0x08,0x62},\
	 {0x53,0x03,0x00,0x34,0x00,0x0C,0x09,0xB3},\
	 {0x54,0x03,0x00,0x34,0x00,0x0C,0x08,0x04},\
	 {0x55,0x03,0x00,0x34,0x00,0x0C,0x09,0xD5},\
	 {0x56,0x03,0x00,0x34,0x00,0x0C,0x09,0xE6},\
	 {0x57,0x03,0x00,0x34,0x00,0x0C,0x08,0x37} };
	int IMU_FD = -1;
	if ((IMU_FD = serialOpen(path, Baud)) < 0)
	{
		printf("open %s serial error!\n",path);
		exit(1);
	}
	else{
		for(int i=0;i<num;++i)
		{
			serialFlush(IMU_FD);
			write(IMU_FD, CMD[imu+i], 8);
			int bite_rec = read(IMU_FD, tmp_buf, 29);
			if(bite_rec>0)
			continue;
			else{
				printf("open %s  %d imu failed!\n",path,imu+i);
				return -1;
			}
		}
		
		printf("open %s success!\n",path);
	}

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
		
		// write(imu_fd, CMD2[_CH+base], 8);
		// read(imu_fd, tmp_buf, 13);
		// //四元数
		// data[_CH][6] = (float)((short)((tmp_buf[3] << 8) | tmp_buf[4])) / 32768.0;
		// data[_CH][7] = (float)((short)((tmp_buf[5] << 8) | tmp_buf[6])) / 32768.0;
		// data[_CH][8] = (float)((short)((tmp_buf[7] << 8) | tmp_buf[8])) / 32768.0;
		// data[_CH][9] = (float)((short)((tmp_buf[9] << 8) | tmp_buf[10])) / 32768.0;
		
	}
}
/*这是最后一个函数了，此函数在data_IMU_main.cpp文件中被作为线程的函数使用
作用就是检测是否突变，并将上次数据和本次数据相关信息发送给数据流*/
void* thread_imu(void* IMU_config){
	IMU_CFG* imuConfig = (IMU_CFG*) IMU_config;
	float temp_data[8][12];
	timeval time_begin;
    float time;
    timeval time_end;

	pthread_mutex_lock(&(imuConfig->Bptr));//加锁Bptr
	pthread_mutex_lock(&(imuConfig->Eptr));//加锁Eptr
	/*陷入死循环，让线程一直工作*/
	while(1)
	{/*flag用于跳出循环*/
		if(imuConfig->flag == 1){
			break;
		}
		/*再次上锁*/
		pthread_mutex_lock(&(imuConfig->Bptr));//d等待，加锁Bptr
		gettimeofday(&time_begin, NULL);/*获取当前时间*/
		IMU_getdata(imuConfig->ch,imuConfig->imu_fd,imuConfig->base,temp_data);/*获取信息*/

		if(imuConfig->firstflag)/*读取首轮IMU的数据*/
		{
			//首个数据
			for(int _ch=0;_ch<imuConfig->ch;++_ch){
				for(int i=0;i<12;++i){/*读取8个IMU的12个数据，二维数组*/
					imuConfig->last_data[_ch][i] = temp_data[_ch][i];
				}
			}
			imuConfig->firstflag = false;/*首轮数据读取完毕，将firstflag设置false*/
			imuConfig->changeflag = false;
		}
		else
		{
			//不是首个数据，检查和上一个数据相比，是否发生突变
			for(int _ch=0;_ch<imuConfig->ch;++_ch){/*通过用上一次数据
			-当前数据以及相乘对比下每个IMU的索引10的数据*/
				if(imuConfig->last_data[_ch][10]*temp_data[_ch][10] < 0 || (abs(imuConfig->last_data[_ch][10]-temp_data[_ch][10])) > 5)
				{/*判断突变发生需要进行两次判定*/
					if(imuConfig->last_data[_ch][10]*temp_data[_ch][10] < 0 && (abs(imuConfig->last_data[_ch][10]+temp_data[_ch][10])) < 2)
						continue;/*如果continue表示突变不符合条件*/
					imuConfig->changeflag = true;
					imuConfig->count++;
					if(imuConfig->count>2)/*这里如果发生三次突变就不记录了*/
					{
						//连续3个突变，将地3个变为现在的 imuConfig->changeflag = false;
						imuConfig->count = 0;
						imuConfig->changeflag = false;
					}
					break;
				}
			}

			//数据没有突变，last_data则变成新的
			if(!imuConfig->changeflag)
			{
				for(int _ch=0;_ch<imuConfig->ch;++_ch){
					for(int i=0;i<12;++i){
						imuConfig->last_data[_ch][i] = temp_data[_ch][i];//last_data则变成新的
					}
				}
				imuConfig->count = 0;
			}

			imuConfig->changeflag = false;
		}
		/*首次对mptr进行上锁*/
		pthread_mutex_lock(&(imuConfig->mptr));
		for(int _ch=0;_ch<imuConfig->ch;++_ch){/*将上次数据存入用来处理的数据*/
			for(int i=0;i<12;++i){
				imuConfig->data[_ch][i] = imuConfig->last_data[_ch][i];
			//	imuConfig->data_original[_ch][i] = temp_data[_ch][i];
			}
		}
		pthread_mutex_unlock(&(imuConfig->mptr));/*解锁mptr，此时mptr没有锁了*/

		pthread_mutex_unlock(&(imuConfig->Eptr));//解锁Eptr，此时Eptr没有锁了



		gettimeofday(&time_end, NULL);/*读取当前时间，这个时间段就是我们读取数据并判断
		的时间段*/

		imuConfig->imu_file<<time_begin.tv_sec<<","<<time_begin.tv_usec<<",";/*将开始时间
		的微秒和秒存入数据流中*/
		imuConfig->imu_original_file<<time_begin.tv_sec<<","<<time_begin.tv_usec<<",";/*
		将开始时间的微秒和秒存入原始数据流中*/

		for(int _ch=0;_ch<imuConfig->ch;++_ch){/*将上次数据和本次数据存入数据流中*/
			for(int i=0;i<12;++i){
				imuConfig->imu_file<<imuConfig->last_data[_ch][i]<<",";
				imuConfig->imu_original_file<<temp_data[_ch][i]<<",";
			}
		}/*将结束时间也发送给两个数据流*/
		imuConfig->imu_file<<time_end.tv_sec<<","<<time_end.tv_usec<<",";
		imuConfig->imu_original_file<<time_end.tv_sec<<","<<time_end.tv_usec<<",";
		/*计算出时间段长度，将时间段长度也发送给两个数据流*/
		time = (float)(1000 * (time_end.tv_sec - time_begin.tv_sec) + (time_end.tv_usec - time_begin.tv_usec)/1000.0);
        imuConfig->imu_file<<time;
		imuConfig->imu_original_file<<time;
        imuConfig->imu_file<<endl;/*将换行符也发送过去*/
		imuConfig->imu_original_file<<endl;
	}

	printf("thread_imu %d exit\n",imuConfig->base);
	return NULL;
}
