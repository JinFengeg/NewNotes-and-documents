#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <wiringSerial.h>
#include <pthread.h> 
#include<sys/time.h>
#include "data_IMU_main.h"
#include "IMU_data.h"
#include "config.h"
#include <fstream>
#include <iostream>
using namespace std;

static ofstream imu_file;
static pid_t pid_imu;

static int proc_status =1;
static IMU_CFG imu_L;
static int imufdl = -1;
static int imufdr = -1;
static IMU_CFG imu_R;
pthread_t tid_L,tid_R;
float IMU_DATA[ROW][COLUMN] = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} };

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

static void APP_exit()
{
	imu_L.flag = 1;
	imu_R.flag = 1;
	pthread_join(tid_L,NULL);
	pthread_join(tid_R,NULL);
	serialClose(imufdl);
	serialClose(imufdr);
	pthread_mutex_destroy(&(imu_L.mptr));
	pthread_mutex_destroy(&(imu_R.mptr));
	imu_file.close();
	imu_L.imu_file.close();
	imu_R.imu_file.close();
	printf("=============== IMU  EXIT! ==============\n ");
	exit(0);
}
static void int_imu_cfg()
{
	imu_L.ch = IMU_CH_L;
	imu_R.ch = IMU_CH_R;
	imu_L.flag = 0;
	imu_R.flag = 0;
	imu_L.base = 4;
	imu_R.base = 0;
	pthread_mutex_init(&(imu_L.mptr),NULL);
	pthread_mutex_init(&(imu_R.mptr),NULL);
	imu_L.imu_fd = imufdl;
	imu_R.imu_fd = imufdr;

	imu_L.imu_file.open("./file_IMU_L.csv");
	for (int i=0;i<IMU_CH_L;i++)
	{
	    imu_L.imu_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}
	imu_L.imu_file<<endl;

	imu_R.imu_file.open("./file_IMU_R.csv");
	for (int i=0;i<IMU_CH_R;i++)
	{
	    imu_R.imu_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}
	imu_R.imu_file<<endl;

}

void SigH(int Sig)
{
	switch (Sig)
	{
	case SIGINT:
		proc_status =0 ;
		break;
	default:
		break;
	}
}

static void init_sig()
{
	signal(SIGINT, SigH);

}


void data_IMU_main()
{
	init_sig();
	imufdl = init_IMU("/dev/ttyUSB0");
	imufdr = init_IMU("/dev/ttyUSB1");
	int_imu_cfg();
	init_file();

	if(pthread_create(&tid_L,NULL,thread_imu,&imu_L)){
		printf("16\n");
		serialClose(imufdl);
		exit(1);
	}
	if(pthread_create(&tid_L,NULL,thread_imu,&imu_R)){
		printf("17\n");
		serialClose(imufdr);
		exit(1);
	}
	printf("begin\n");
	timeval time_begin;
    float time;
    timeval time_end;
	usleep(50000);
	while (1)
	{
		gettimeofday(&time_begin, NULL);
		usleep(12000);
		if (proc_status == 0)
		{
			APP_exit();
		}
		pthread_mutex_lock(&imu_R.mptr);
		for(int _ch=0;_ch<IMU_CH_R;++_ch){
			for(int i=0;i<12;++i){
				IMU_DATA[_ch][i] = imu_R.data[_ch][i];
			}
		}
		pthread_mutex_unlock(&imu_R.mptr);

		pthread_mutex_lock(&imu_L.mptr);
		for(int _ch=0;_ch<IMU_CH_L;++_ch){
			for(int i=0;i<12;++i){
				IMU_DATA[_ch+IMU_CH_R][i] = imu_L.data[_ch][i];
			}
		}
		pthread_mutex_unlock(&imu_L.mptr);
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

int main(int argc, char** argv)
{
	data_IMU_main();
}
