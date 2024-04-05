/*
*   使用usb转can通信
*/


#include "tensorflow/lite/examples/LowerMachine/main_can.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <pthread.h>
#include <cstring>
#include <cstring>
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/config.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"
#include "tensorflow/lite/examples/LowerMachine/shm.h"
#include "tensorflow/lite/examples/LowerMachine/main_data.h"
#include "tensorflow/lite/examples/LowerMachine/main_dsp.h"
#include "tensorflow/lite/examples/LowerMachine/CAN_class.h"

#include <fstream>
using namespace std;

static CAN_class *can = new CAN_class;
static double Dur_time;



//��ʱ����pid
static pid_t pid_TCP;

//��־λ
static int en_TCP_flg = STATUS_OFF;
static int proc_status = STATUS_ON;
static int act_over = STATUS_OFF;

static timeval time_tcp;
static timeval last_time_tcp;
static ofstream can_file;

static void APP_exit()
{
	delete can;
	can_file.close();
	std::cout << "=============== CAN  EXIT! ==============" << std::endl;
	exit(0);
}

static void init_file()
{
    can_file.open("./can_file_time.csv");
    if(!can_file.is_open())
    {
        cout<<"open can file error"<<endl;
        exit(1);
    }
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

/*******************************************************************************/
void main_can()
{
	const int count = sizeof(CAN_RES);
	init_file();
	init_sig();
	init_glbs();	//��ʼ��ȫ��
	char identiy = '0';
    if(can->init() == 0)
    {
        exit(-1);
    }
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
			CAN_RES buff;
			buff.label = '0';
            buff.time = 0.0;
			char buffer[count];
			memcpy(buffer,&buff,count);
			can->send_data(buffer,count);
            usleep(40000);
            can->send_data(buffer,count);
			can_file<<"actok";
			can_file<<endl<<endl;
		}
		if (en_TCP_flg)
		{
			en_TCP_flg = STATUS_OFF;
			static int ST;
			Sem_P(semid_TCP);
			ST = p_shm_TCP->Status;
			Dur_time = p_shm_TCP->DurTime;
			Sem_V(semid_TCP);
			//printf("+.......ST............%d\n", ST);
			switch (ST)   // ������ 0 վ��  1 �¶�  2����   3����   4 ���  5 ���ƽ�ȡ�
			{
			case 0:
				identiy = '0';
				break;
			case 1:
				identiy = '1';
				break;
			case 2:
				identiy = '2';
				break;
			case 3:
				identiy = '3';
				break;
			case 4:
				identiy = '4';
				break;
			case 5:
				identiy = '5';
				break;
			default:
				identiy = '0';
				break;
			}

			gettimeofday(&time_tcp, NULL);
			float time = (float)(1000 * (time_tcp.tv_sec - last_time_tcp.tv_sec) + (time_tcp.tv_usec - last_time_tcp.tv_usec)/1000.0);
			can_file<<ST<<','<<time_tcp.tv_sec<<','<< time_tcp.tv_usec<<','<<time<<endl;
			last_time_tcp.tv_usec = time_tcp.tv_usec;
			last_time_tcp.tv_sec = time_tcp.tv_sec;


			CAN_RES buff;
			buff.label = identiy;
			buff.time = (float)Dur_time;
			buff.seconds = time_tcp.tv_sec;
			buff.usecond = time_tcp.tv_usec;
			char buffer[count];
			memcpy(buffer,&buff,count);
			can->send_data(buffer,count);
		}
	}
}