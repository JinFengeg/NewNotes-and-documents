#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "tensorflow/lite/examples/LowerMachine/main_dsp.h"
#include "tensorflow/lite/examples/LowerMachine/dsp_RCG_main.h"
#include "tensorflow/lite/examples/LowerMachine/dsp_ACT.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"
#include "tensorflow/lite/examples/LowerMachine/shm.h"
//#include "BP_DATA.h"

//pid
static pid_t pid_RCG = 0;
static pid_t pid_dsp = 0;
static pid_t pid_tcp = 0;

//��μ��
static dsp_ACTClass *ACT = new dsp_ACTClass;

//����
static int en_DSP_flag = false;
static int proc_status = STATUS_ON;

//static FILE* fp_ACT;



static void APP_exit()
{
	delete ACT;
	printf("=============== DSP  EXIT! ==============\n");
	exit(0);
}

static void SigH(int sig)
{
	switch (sig)
	{
	case SIGINT:
		break;
	case 10:				//main_data���̼���35���㣨ȫ�ֱ�����dspΪoff���˴����������Ϊoff�����������ݴ�����
		Sem_P(semid_glbs);
		//�Լ�����dsp��
		//proc_status = p_shm_GLB->proc_status.proc_status_proc;
		en_DSP_flag = p_shm_GLB->proc_status.en_dsp;
		p_shm_GLB->proc_status.en_dsp = STATUS_OFF;
		Sem_V(semid_glbs);
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

//dSP���̺�д�룬����ȡRCG���̺�
static void init_glbs()
{
	pid_dsp = getpid();
	Sem_P(semid_glbs);
	p_shm_GLB->PID.PID_DSP = pid_dsp;
	Sem_V(semid_glbs);

	do
	{
		usleep(300);
		Sem_P(semid_glbs);
		pid_RCG = p_shm_GLB->PID.PID_RCG;
		pid_tcp = p_shm_GLB->PID.PID_MAIN_TCP;
		Sem_V(semid_glbs);
	} while (pid_RCG == 0 || pid_tcp == 0);
	//printf("PID_RCG = %d\n", pid_RCG);
}


void main_dsp()
{
	init_sig();		//��ʼ���ź�
	init_glbs();
	
	ACT->Init(pid_RCG);
	while (true)
	{
		usleep(100);
		if (proc_status == STATUS_OFF)
		{
			APP_exit();
		}
		if (en_DSP_flag)
		{
			//printf("dsp!*********\n");
			en_DSP_flag = false;
			//
			Sem_P(semid_DATA);
			ACT->CpyD2A(p_shm_DATA); //  //�����µ����ݿ�����ACT��DATW�ĳ�Ա
			Sem_V(semid_DATA);
			//
			ACT->Check();
			//
			if (ACT->ACT_STATUS == 1)
			{
				ACT->ACT_STATUS = 0;
				Sem_P(semid_glbs);
				p_shm_GLB->proc_status.proc_ACT_status = STATUS_ON;
				Sem_V(semid_glbs);
				kill(pid_RCG, 10);

				//通知tcp进程站立状态
				Sem_P(semid_TCP);
				p_shm_TCP->Status = 0;
				p_shm_TCP->DurTime = 0.0;
				Sem_V(semid_TCP);

				Sem_P(semid_glbs);
				p_shm_GLB->proc_status.en_tcp = STATUS_OFF;
				p_shm_GLB->proc_status.ACT_OVER = STATUS_ON;
				Sem_V(semid_glbs);
				kill(pid_tcp, 10);
			}
			//
			if (ACT->EnAble())
			{
				Sem_P(semid_ACT);
				ACT->CpyA2A(p_shm_ACT);
				Sem_V(semid_ACT);
				ACT->MoveForward();
				Sem_P(semid_glbs);
				p_shm_GLB->proc_status.en_rcg = STATUS_ON;
				Sem_V(semid_glbs);
				kill(pid_RCG, 10);
			}

		}
	}
}