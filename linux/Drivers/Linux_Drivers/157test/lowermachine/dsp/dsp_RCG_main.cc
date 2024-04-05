#include <signal.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "tensorflow/lite/examples/LowerMachine/dsp_RCGClass.h"
#include "tensorflow/lite/examples/LowerMachine/dsp_RCG_main.h"
#include "tensorflow/lite/examples/LowerMachine/shm.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/main_data.h"
#include "tensorflow/lite/examples/LowerMachine/dsp_ACT.h"
#include "tensorflow/lite/examples/LowerMachine/main_tcp.h"
#include "tensorflow/lite/examples/LowerMachine/BP_DATA.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"



static FILE *fp_De;

//��ʱ����pid
static pid_t pid_main_tcp = 0;
static pid_t pid_RCG = 0;

//״̬־
static int en_RCG_flag;
static int ACT_STATUS = 0;
static int T_status;

//��������
static dsp_RCGClass *RCG = new dsp_RCGClass;

//״̬־
static int proc_status = STATUS_ON;
static int TCP_status = STATUS_OFF;

//����
static float ST_tmp[6] = { 0 };    //0  վ��   1 �¶�   2 ����  3 ƽ��  BPʶ������
static int T_Flag = 0;  //��־λ T_Flag = 0ʱ���վ�����¶ף�1ʱ�����Ծ����У� 2�����ؼ�ƽ��  3������ƽ���������
static int Label = 0;  // 0 站立 1 预备起跳 2 起跳 3 空中  4 落地下蹲 5 起身
static int Count = 0;  //

//��ʱ����
static timeval T_start;
static timeval T_end;
static timeval T_last_end;
static double DUR;

static void APP_exit()
{
	BP_delete();
	
	#ifdef ANS
	BP_off.close();
	#endif
	delete RCG;

	printf("=============== RCG  EXIT! ==============\n");
	exit(0);

}


static void SigH(int sig)
{
	switch (sig) 
	{
	case SIGINT:			//�ⲿ������Ҫ������˳���
		break;
	case 10:				
		Sem_P(semid_glbs);
		//RCG��־
		en_RCG_flag = p_shm_GLB->proc_status.en_rcg;
		p_shm_GLB->proc_status.en_rcg = STATUS_OFF;
		//ACT������־
		ACT_STATUS = p_shm_GLB->proc_status.proc_ACT_status;
		//��ʱ��־��
		T_status = p_shm_GLB->proc_status.T_status;
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

//��RCG���̺�д�룬����ȡtcp���̺�
static void init_glbs()
{
	pid_RCG = getpid();
	Sem_P(semid_glbs);
	p_shm_GLB->PID.PID_RCG = pid_RCG;
	Sem_V(semid_glbs);
	do
	{
		usleep(300);
		Sem_P(semid_glbs);
		pid_main_tcp = p_shm_GLB->PID.PID_MAIN_TCP;
		Sem_V(semid_glbs);
	} while (pid_main_tcp == 0);
	
}

static void wait_TCP()
{
	do 
	{
		usleep(300);
		Sem_P(semid_glbs);
		TCP_status = p_shm_GLB->proc_status.proc_TCP_status;
		Sem_V(semid_glbs);
	} while (TCP_status == STATUS_OFF);
	printf("TCP.........OK!!!\n");
}


static void OF_L()
{
	fp_De = fopen("A_BP_DE.csv", "w");
}

static void Get_timer()
{
	switch (T_status)
	{
	case 0:
		break;
	case 1:
		//
		Sem_P(semid_glbs);
		p_shm_GLB->proc_status.T_status = STATUS_OFF;
		Sem_V(semid_glbs);
		//
		Sem_P(semid_timer);
		T_start.tv_sec = p_shm_TIMER->Timer1.tv_sec;
		T_start.tv_usec = p_shm_TIMER->Timer1.tv_usec;
		Sem_V(semid_timer);
		T_status = 0;
		break;
	case 2:
		//���ı�־λ��
		Sem_P(semid_glbs);
		p_shm_GLB->proc_status.T_status = STATUS_OFF;
		Sem_V(semid_glbs);

		//��ȡ200ms�ĵ�һ�����ݰ���
		Sem_P(semid_timer);
		T_start.tv_sec = p_shm_TIMER->Timer5.tv_sec;
		T_start.tv_usec = p_shm_TIMER->Timer5.tv_usec;
		Sem_V(semid_timer);

		//========
		T_start.tv_sec = T_last_end.tv_sec;
		T_start.tv_usec = T_last_end.tv_usec;

		T_status = 0;
		break;
	default:
		break;
	}
}

void dsp_RCG_main()
{
	init_sig();		//��ʼ���ź�
	init_glbs();	//��ʼ��ȫ��
	BP_DATA();
	RCG->Init();		//������ݴ�����ʼ��
	wait_TCP();
	OF_L();
	while (true)
	{
		//usleep(100);
		//	�����˳�
		if (proc_status == STATUS_OFF)
		{
			APP_exit();
		}
		//��ȡ��ʱ��ʼʱ�䡣
		Get_timer();
		//����Ƿ�������������0״̬��ʼ��
		if (ACT_STATUS == 1)
		{
			ACT_STATUS = 0;
			Sem_P(semid_glbs);
			p_shm_GLB->proc_status.proc_ACT_status = STATUS_OFF;
			Sem_V(semid_glbs);
			T_Flag = 0;
			printf("ACT OK!!!\n");
			fprintf(fp_De, "ACT OK!!!\n");
			#ifdef ANS
			BP_off<<"=====================\n\n"<<std::endl;
			#endif
		}
		//�������ݴ���
		if (en_RCG_flag)
		{
			en_RCG_flag = STATUS_OFF;
			//1.���ݱ���
			Sem_P(semid_ACT);
			RCG->BackUp(p_shm_ACT);
			Sem_V(semid_ACT);
			//2.������ݼ����Լ�����ģʽʶ��
			int t_now = RCG->ACTD.Tnow;
			RCG->Recg();
			int U = RCG->actionU;
			int V = RCG->gaitV;
			static int count_2 = 0;
			gettimeofday(&T_end, NULL);
			DUR = (double)(1000 * (T_end.tv_sec - T_start.tv_sec) + (T_end.tv_usec - T_start.tv_usec)/1000.0);
			fprintf(fp_De, "%d,%d,%d,%ld,%ld\n",t_now,U,V,T_end.tv_sec,T_end.tv_usec);
			fflush(fp_De);

			T_last_end.tv_sec = T_end.tv_sec;
			T_last_end.tv_usec = T_end.tv_usec;

			//TCP�����ڴ档
			Sem_P(semid_TCP);
			p_shm_TCP->Status = U*10+V;
			p_shm_TCP->DurTime = DUR;
			Sem_V(semid_TCP);
			//֪ͨTCP
			Sem_P(semid_glbs);
			p_shm_GLB->proc_status.en_tcp = STATUS_ON;
			Sem_V(semid_glbs);
			kill(pid_main_tcp, 10); 
		}
	}
}