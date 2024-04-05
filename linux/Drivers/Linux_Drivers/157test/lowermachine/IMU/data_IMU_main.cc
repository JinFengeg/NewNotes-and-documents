#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <unistd.h>
#include <wiringSerial.h>
#include <pthread.h> 
#include "tensorflow/lite/examples/LowerMachine/data_IMU_main.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/shm.h"
#include "tensorflow/lite/examples/LowerMachine/IMU_data.h"
#include "tensorflow/lite/examples/LowerMachine/config.h"

#include <fstream>
#include <iostream>
using namespace std;/*命名空间，呵呵了，进程创建的时候也会分配进程命名空间*/

static pid_t pid_imu;

static int proc_status = STATUS_ON;


static ofstream imu_file;

static int imufdl = -1;
static int imufdr = -1;
static int imufdchest = -1;

static IMU_CFG imu_L;
static IMU_CFG imu_R;
static IMU_CFG imu_chest;

pthread_t tid_L,tid_R,tid_chest;
/**/
float IMU_DATA[ROW][COLUMN] = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} };
float IMU_DATA_orignal[ROW][COLUMN] = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} };
/*析构函数，进程退出时需要执行的操作*/
static void APP_exit()
{/*初始化flag = 1 表示thread_imu函数直接退出不会采集并检测数据了*/
	imu_L.flag = 1;
	imu_R.flag = 1;
	imu_chest.flag = 1;

	/*注销三个线程*/
	pthread_cancel(tid_L);
	pthread_cancel(tid_R);
	pthread_cancel(tid_chest);


	/*串口关闭*/
	serialClose(imufdl);
	serialClose(imufdr);
	serialClose(imufdchest);
	/*线程互斥锁销毁*/
	pthread_mutex_destroy(&(imu_L.mptr));
	pthread_mutex_destroy(&(imu_L.Bptr));
	pthread_mutex_destroy(&(imu_L.Eptr));

	pthread_mutex_destroy(&(imu_R.mptr));
	pthread_mutex_destroy(&(imu_R.Bptr));
	pthread_mutex_destroy(&(imu_R.Eptr));

	pthread_mutex_destroy(&(imu_chest.mptr));
	pthread_mutex_destroy(&(imu_chest.Bptr));
	pthread_mutex_destroy(&(imu_chest.Eptr));

	
	/*关闭打开的文件*/
	imu_file.close();
	/*分为左右腿和胸部的IMU*/
	imu_L.imu_file.close();
	imu_R.imu_file.close();
	imu_chest.imu_file.close();

	imu_L.imu_original_file.close();
	imu_R.imu_original_file.close();
	imu_chest.imu_original_file.close();



	printf("=============== IMU  EXIT! ==============\n ");
	exit(0);
}
/*初始化总文件，为接下来传入的数据做标识工作*/
static void init_file(){
	/*打开总文件*/
    imu_file.open("./file_IMUALL.csv");
    if(!imu_file.is_open())
    {
        cout<<"open tcp file error\n"<<endl;
		exit(1);
    }
	/*传入开始秒、微秒标题*/
    imu_file<<"beginsec,beginusec,";
	/*传入十二个数据的标题*/
	for (int i=0;i<IMU_CH;i++)
	{
	    imu_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}/*传入结束时间秒、微秒标题*/
    imu_file<<"endsec,endusec";
	imu_file<<endl;

}

/*配置左右腿、胸部的IMU文件以及线程相关参数*/
static void int_imu_cfg()
{/*首轮数据、数据没有发生突变、三次突变计数归零*/
	imu_L.firstflag = true;
	imu_L.changeflag = false;
	imu_L.count = 0;

	imu_R.firstflag = true;
	imu_R.changeflag = false;
	imu_R.count = 0;

	imu_chest.firstflag = true;
	imu_chest.changeflag = false;
	imu_chest.count =0;


/*设置左右腿和胸部的IMU个数*/
	imu_L.ch = IMU_CH_L;
	imu_R.ch = IMU_CH_R;
	imu_chest.ch = 1;
/*设置thread_imu函数的执行权限*/
	imu_L.flag = 0;
	imu_R.flag = 0;
	imu_chest.flag = 0;
/*设置左右腿和胸部IMU的文件描述符*/
	imu_L.imu_fd = imufdl;
	imu_R.imu_fd = imufdr;
	imu_chest.imu_fd = imufdchest;
	/*设置线程*/
	pthread_mutex_init(&(imu_L.mptr),NULL);
	pthread_mutex_init(&(imu_L.Bptr),NULL);
	pthread_mutex_init(&(imu_L.Eptr),NULL);

	pthread_mutex_init(&(imu_R.mptr),NULL);
	pthread_mutex_init(&(imu_R.Bptr),NULL);
	pthread_mutex_init(&(imu_R.Eptr),NULL);

	pthread_mutex_init(&(imu_chest.mptr),NULL);
	pthread_mutex_init(&(imu_chest.Bptr),NULL);
	pthread_mutex_init(&(imu_chest.Eptr),NULL);
	/*左腿的IMU从4号开始，右腿从1号开始，胸部是第零个*/
	imu_L.base = 1+IMU_CH_L;
	imu_R.base = 1;
	imu_chest.base = 0;

	/*打开左腿IMU文件以及原始文件、右腿和胸部*/
	imu_L.imu_file.open("./file_IMU_L.csv");
	imu_L.imu_file<<"beginsec,beginusec,";
	for (int i=0;i<IMU_CH_L;i++)
	{
	    imu_L.imu_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}
	imu_L.imu_file<<"endsec,endusec";
	imu_L.imu_file<<endl;


	imu_L.imu_original_file.open("./file_IMU_L_original.csv");
	imu_L.imu_original_file<<"beginsec,beginusec,";
	for (int i=0;i<IMU_CH_L;i++)
	{
	    imu_L.imu_original_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}
	imu_L.imu_original_file<<"endsec,endusec";
	imu_L.imu_original_file<<endl;


	
	imu_R.imu_file.open("./file_IMU_R.csv");
	imu_R.imu_file<<"beginsec,beginusec,";
	for (int i=0;i<IMU_CH_R;i++)
	{
	    imu_R.imu_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}
	imu_R.imu_file<<"endsec,endusec";
	imu_R.imu_file<<endl;

	imu_R.imu_original_file.open("./file_IMU_R_original.csv");
	imu_R.imu_original_file<<"beginsec,beginusec,";
	for (int i=0;i<IMU_CH_R;i++)
	{
	    imu_R.imu_original_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}
	imu_R.imu_original_file<<"endsec,endusec";
	imu_R.imu_original_file<<endl;


	imu_chest.imu_file.open("./file_IMU_chest.csv");
	imu_chest.imu_file<<"beginsec,beginusec,";
	for (int i=0;i<1;i++)
	{
	    imu_chest.imu_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}
	imu_chest.imu_file<<"endsec,endusec";
	imu_chest.imu_file<<endl;

	imu_chest.imu_original_file.open("./file_IMU_chest_original.csv");
	imu_chest.imu_original_file<<"beginsec,beginusec,";
	for (int i=0;i<1;i++)
	{
	    imu_chest.imu_original_file<<"ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw,";
	}
	imu_chest.imu_original_file<<"endsec,endusec";
	imu_chest.imu_original_file<<endl;


}
/*信号处理函数，之前提到过这里，这回写的明确了很多
proc_status决定着是否退出进程，不过这次不是退出thread_imu函数了
而是退出data_IMU_main函数*/
void SigH(int Sig)
{
	switch (Sig)
	{
	case SIGINT:
		proc_status = STATUS_OFF;
		break;
	case SIGUSR1:
		break;
	case 12:
		proc_status = STATUS_OFF;
		break;
	default:
		break;
	}
}
/*注册信号处理函数*/
static void init_sig()
{
	signal(SIGINT, SigH);
	signal(10, SigH);
	signal(12, SigH);
}
/*获取当前进程号，信号量在data_ADC_main文件中定义*/
static void init_glbs()
{
	pid_imu = getpid();
	Sem_P(semid_glbs);
	p_shm_GLB->PID.PID_IMU = pid_imu;/*临界区，将进程号传入结构体中*/
	Sem_V(semid_glbs);
}

void data_IMU_main()
{
	init_sig();
	init_glbs();
	init_file();/*
	返回值是文件描述符，传入设备文件路径，IMU基数和IMU数量*/
	imufdl = init_IMU("/dev/ttyUSB1",1+IMU_CH_R,IMU_CH_L);
	imufdr = init_IMU("/dev/ttyUSB2",1,IMU_CH_R);
	imufdchest = init_IMU("/dev/ttyUSB0",0,1);
	int_imu_cfg();

/*创建三个线程，在这三个线程中读取数据并检测是否发生突变存入数据流中*/
	if(pthread_create(&tid_L,NULL,thread_imu,&imu_L) != 0 ){
		printf("16\n");
		serialClose(imufdl);
		exit(1);
	}
	if(pthread_create(&tid_R,NULL,thread_imu,&imu_R) != 0 ){
		printf("17\n");
		serialClose(imufdr);
		exit(1);
	}

	if(pthread_create(&tid_chest,NULL,thread_imu,&imu_chest) != 0 ){
		printf("18\n");
		serialClose(imufdchest);
		exit(1);
	}

	timeval time_begin;
    float time;
    timeval time_end;
	printf("begin\n");
	while (1)/*接下来就是轮询*/
	{
		gettimeofday(&time_begin, NULL);
		usleep(1000);
		if (proc_status == STATUS_OFF)
		{
			//退出
			APP_exit();
		}
		//采集左右腿
		pthread_mutex_unlock(&(imu_R.Bptr));//解锁Eptr
		pthread_mutex_unlock(&(imu_L.Bptr));//解锁Eptr
		pthread_mutex_unlock(&(imu_chest.Bptr));//解锁Eptr
		

		//采集胸前
		pthread_mutex_lock(&imu_chest.Eptr);

		pthread_mutex_lock(&imu_chest.mptr);
		for(int _ch=0;_ch<1;++_ch){/*将胸部线程采集到的处理数据数组传入本进程数组*/
			for(int i=0;i<12;++i){
				IMU_DATA[_ch][i] = imu_chest.data[_ch][i];
				//IMU_DATA_orignal[_ch][i] = imu_chest.data_original[_ch][i];
			}
		}
		pthread_mutex_unlock(&imu_chest.mptr);

		//采集右腿
		pthread_mutex_lock(&imu_R.Eptr);

		pthread_mutex_lock(&imu_R.mptr);
		for(int _ch=0;_ch<IMU_CH_R;++_ch){
			for(int i=0;i<12;++i){
				IMU_DATA[_ch+1][i] = imu_R.data[_ch][i];
			//	IMU_DATA_orignal[_ch+1][i] = imu_R.data_original[_ch][i];
			}
		}
		pthread_mutex_unlock(&imu_R.mptr);

		//采集左腿
		pthread_mutex_lock(&imu_L.Eptr);

		pthread_mutex_lock(&imu_L.mptr);
		for(int _ch=0;_ch<IMU_CH_L;++_ch){
			for(int i=0;i<12;++i){
				IMU_DATA[_ch+1+IMU_CH_L][i] = imu_L.data[_ch][i];
			//	IMU_DATA_orignal[_ch+1+IMU_CH_L][i] = imu_L.data_original[_ch][i];
			}
		}
		pthread_mutex_unlock(&imu_L.mptr);

		/*将左腿、右腿、胸部的处理数据数组传入IMU_DATA数组中*/
		if (!Sem_P(semid_imus))
		{
			printf("16\n");
			exit(1);
		}/*再把数据传入另一个结构体中国*/
		for (int _CH = 0; _CH <IMU_CH; _CH++) 
		{
			for (int n = 0; n < COLUMN; n++)
			{
				p_shm_IMU->Data_src[_CH][n] = IMU_DATA[_CH][n];
			//	p_shm_IMU->Data_src_original[_CH][n] = IMU_DATA_orignal[_CH][n];
			}
		}
		p_shm_IMU->nflag = true;/*这个有啥用*/
		Sem_V(semid_imus);

		/*下面是同样的操作，把数据存入总文件中，而线程是把左腿右腿胸部数据分别存入
		自己文件中*/
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
