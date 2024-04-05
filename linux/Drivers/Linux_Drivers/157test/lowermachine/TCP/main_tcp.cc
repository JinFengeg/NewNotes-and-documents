/*********************************************************
*			TCP���ݴ���
*********************************************************/
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <pthread.h>
#include <cstring>
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/config.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"
#include "tensorflow/lite/examples/LowerMachine/shm.h"
#include "tensorflow/lite/examples/LowerMachine/main_data.h"
#include "tensorflow/lite/examples/LowerMachine/main_dsp.h"
#include "tensorflow/lite/examples/LowerMachine/main_tcp.h"
#include "tensorflow/lite/examples/LowerMachine/tcp_TCPClass.h"

#include <fstream>
using namespace std;

static tcp_TCPClass *TCP = new tcp_TCPClass;
static TCP_Data_t TCP_Data;
static double Dur_time;



//��ʱ����pid
static pid_t pid_TCP;

//��־λ
static int en_TCP_flg = STATUS_OFF;
static int proc_status = STATUS_ON;
static int act_over = STATUS_OFF;


static timeval time_tcp;
static timeval last_time_tcp;
static ofstream tcp_file;

static void APP_exit()
{
	delete TCP;
	tcp_file.close();
	std::cout << "=============== TCP  EXIT! ==============" << std::endl;
	exit(0);
}


/******************	�źŴ�������	**********************************************/
static void SigH(int sig)
{
	switch (sig)
	{
	case SIGINT:			//�ⲿ������Ҫ������˳���
		break;
	case 10:
		Sem_P(semid_glbs);
		en_TCP_flg = p_shm_GLB->proc_status.en_tcp;
		p_shm_GLB->proc_status.en_tcp = STATUS_OFF;
		act_over = p_shm_GLB->proc_status.ACT_OVER;
		p_shm_GLB->proc_status.ACT_OVER = STATUS_OFF;
		Sem_V(semid_glbs);
		break;
	case 12:
		proc_status = STATUS_OFF;
		break;
	default:
		break;
	}
}/*******************************************/
 //	��ʼ���ź�
static bool init_sig()
{
	signal(SIGINT, SigH);
	signal(10, SigH);
	signal(12, SigH);
	return true;
}

//	TCP���̺�д��GLB
static void init_glbs()
{
	pid_TCP = getpid();
	Sem_P(semid_glbs);
	p_shm_GLB->PID.PID_MAIN_TCP = pid_TCP;
	Sem_V(semid_glbs);
}
static void init_file()
{
	tcp_file.open("./tcp_file_time.csv");
	if(!tcp_file.is_open())
	{
		cout<<"open tcp file error\n"<<endl;
		exit(1);
	}
}
/*******************************************************************************/
void main_tcp()
{
	int first = 0;
	init_file();
	const int count = sizeof(Result);
	init_sig();
	init_glbs();	//��ʼ��ȫ��
	//TCP->init();
	//���ı�־λ����ʾTCP�Ѿ���ʼ����ɡ������ӳɹ���
	Sem_P(semid_glbs);
	p_shm_GLB->proc_status.proc_TCP_status = STATUS_ON;
	Sem_V(semid_glbs);
	while (true) 
	{
		usleep(100);
		if (proc_status == STATUS_OFF)
		{
			APP_exit();
		}
		if(act_over)
		{
			act_over = STATUS_OFF;
			Result buff;
			buff.labelaction = -1;
			buff.labelgait = -1;
			buff.time_process = 0;
			buff.second = 0;
			buff.usecond = 0;
			char buffer[count];


			tcp_file<<-1<<"\n";
			// memcpy(buffer,&buff,count);
			// TCP->send(buffer,count);
		}
		if (en_TCP_flg)
		{
			en_TCP_flg = STATUS_OFF;
			static int ST;
			Sem_P(semid_TCP);
			ST = p_shm_TCP->Status;
			Dur_time = p_shm_TCP->DurTime;
			Sem_V(semid_TCP);
		
			gettimeofday(&time_tcp, NULL);
			float time = (float)(1000 * (time_tcp.tv_sec - last_time_tcp.tv_sec) + (time_tcp.tv_usec - last_time_tcp.tv_usec)/1000.0);
			last_time_tcp.tv_usec = time_tcp.tv_usec;
			last_time_tcp.tv_sec = time_tcp.tv_sec;


			Result buff;
			buff.labelaction = ST/10;
			buff.labelgait = ST%10;
			buff.time_process = (float)Dur_time;
			buff.second = time_tcp.tv_sec;
			buff.usecond = time_tcp.tv_usec;
			char buffer[count];
			tcp_file<<buff.labelaction<<','<<buff.labelgait<<','<<buff.time_process<<'\n';
			std::cout<<"action:"<<buff.labelaction<<" gait: "<<buff.labelgait<<" time: "<<buff.time_process<<std::endl;
			// memcpy(buffer,&buff,count);
			// TCP->send(buffer,count);
		}
	}
}
