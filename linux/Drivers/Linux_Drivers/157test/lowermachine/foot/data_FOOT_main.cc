#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <wiringPi.h>

#include "tensorflow/lite/examples/LowerMachine/data_FOOT_main.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"

//�ĸ�����
#define PIN_L_P	5		//���ǰ����
#define PIN_L_H 6		//��ź����
#define PIN_R_P 29		//�ҽ�ǰ����
#define PIN_R_H 28		//�ҽź���� 


static int FT_L_P; //���ǰ����
static int FT_L_H; //��ź����
static int FT_R_P; //�ҽ�ǰ����
static int FT_R_H; //�ҽź����

static int proc_status = STATUS_ON;

static pid_t pid_foot;


static void APP_exit()
{
	printf("=============== FOOT EXIT! ==============\n ");
	exit(0);
}


static void SigH(int Sig)
{
	switch (Sig)
	{
	case SIGINT:
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


static void init_sig()
{
	signal(SIGINT, SigH);
	signal(10, SigH);
	signal(12, SigH);
}

static void init_glbs()
{
	pid_foot = getpid();
	Sem_P(semid_glbs);
	p_shm_GLB->PID.PID_FOOT = pid_foot;
	Sem_V(semid_glbs);
}

static void init_FT()
{
	wiringPiSetup();
	pinMode(PIN_L_P, INPUT);		//��ǰ
	pinMode(PIN_L_H, INPUT);		//���
	pinMode(PIN_R_P, INPUT);		//��ǰ
	pinMode(PIN_R_H, INPUT);		//�Һ�
}



void data_FOOT_main()
{
	init_sig();
	init_glbs();
	init_FT();
	while (1)
	{
		usleep(100);
		if (proc_status == STATUS_OFF)
		{
			APP_exit();
		}
		FT_L_P = digitalRead(PIN_L_P);
		FT_L_H = digitalRead(PIN_L_H);
		FT_R_P = digitalRead(PIN_R_P);
		FT_R_H = digitalRead(PIN_R_H);
		Sem_P(semid_foot);
		p_shm_FOOT->FT_DATA[0] = FT_L_P;
		p_shm_FOOT->FT_DATA[1] = FT_L_H;
		p_shm_FOOT->FT_DATA[2] = FT_R_P;
		p_shm_FOOT->FT_DATA[3] = FT_R_H;
		p_shm_FOOT->nflag = true;
		Sem_V(semid_foot);
	}
}
