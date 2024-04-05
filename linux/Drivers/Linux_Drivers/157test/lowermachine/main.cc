#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include "tensorflow/lite/examples/LowerMachine/main_data.h"
#include "tensorflow/lite/examples/LowerMachine/main_dsp.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"
#include "tensorflow/lite/examples/LowerMachine/shm.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/config.h"
#include "tensorflow/lite/examples/LowerMachine/main_tcp.h"
#include "tensorflow/lite/examples/LowerMachine/data_ADC_main.h"
#include "tensorflow/lite/examples/LowerMachine/data_IMU_main.h"
#include "tensorflow/lite/examples/LowerMachine/data_FOOT_main.h"
#include "tensorflow/lite/examples/LowerMachine/main_tcp.h"
#include "tensorflow/lite/examples/LowerMachine/main_can.h"
#include "tensorflow/lite/examples/LowerMachine/dsp_RCG_main.h"

//锟斤拷志�?
static int proc_status = STATUS_ON;

//PID
static pid_t pid_data;
static pid_t pid_dsp;
static pid_t pid_tcp;
static pid_t pid_main;
static pid_t pid_adc;
static pid_t pid_imu;
static pid_t pid_rcg;
static pid_t pid_foot;
//tcp_flag
int tcp_flag =0;
static void APP_exit()
{
	Sem_P(semid_glbs);
	pid_adc = p_shm_GLB->PID.PID_ADC;
	pid_data = p_shm_GLB->PID.PID_DATA;
	pid_dsp = p_shm_GLB->PID.PID_DSP;
	pid_imu = p_shm_GLB->PID.PID_IMU;
	pid_tcp = p_shm_GLB->PID.PID_MAIN_TCP;
	pid_rcg = p_shm_GLB->PID.PID_RCG;
	pid_foot = p_shm_GLB->PID.PID_FOOT;
	Sem_V(semid_glbs);


	kill(pid_data, 12);
	kill(pid_dsp, 12);
	kill(pid_tcp, 12);
	#ifdef ADC_MODE_OL
	kill(pid_adc, 12);
	kill(pid_imu, 12);
	#endif
	kill(pid_rcg, 12);
	#ifdef ADC_MODE_OL
	kill(pid_foot, 12);
	#endif


	waitpid(pid_data, NULL, 0);
	waitpid(pid_dsp, NULL, 0);
	waitpid(pid_tcp, NULL, 0);
	#ifdef ADC_MODE_OL
	waitpid(pid_adc, NULL, 0);
	waitpid(pid_imu, NULL, 0);
	#endif
	waitpid(pid_rcg, NULL, 0);
	#ifdef ADC_MODE_OL
	waitpid(pid_foot, NULL, 0);
	#endif


	Shm_remove(semid_ACT, shmid_ACT, p_shm_ACT);
	Shm_remove(semid_adcs, shmid_adcs, p_shm_ADC);
	Shm_remove(semid_DATA, shmid_DATA, p_shm_DATA);
	Shm_remove(semid_imus, shmid_imus, p_shm_IMU);
	Shm_remove(semid_foot, shmid_foot, p_shm_FOOT);
	Shm_remove(semid_TCP, shmid_TCP, p_shm_TCP);
	Shm_remove(semid_glbs, shmid_glbs, p_shm_GLB);
	printf("=============== MAIN EXIT! ==============\n");
	exit(0);
}

//	锟脚号达拷锟斤拷
static void SigH(int sig)
{
	switch (sig) 
	{
	case SIGINT:		//锟解部锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷锟斤拷顺锟斤拷锟�
		proc_status = STATUS_OFF;
		break;
	default:
		break;
	}
}


static bool init_sig()
{
	signal(SIGINT, SigH);
	return true;
}

static void init_sem()
{
	semid_glbs = Sem_creat_GLB();
	Sem_set(semid_glbs); //锟脚猴拷锟斤拷锟斤拷�?1
	semid_ACT = Sem_creat_ACT();
	Sem_set(semid_ACT);
	semid_TCP = Sem_creat_TCP();
	Sem_set(semid_TCP);
	semid_adcs = Sem_creat_ADC();
	Sem_set(semid_adcs);
	semid_imus = Sem_creat_IMU();
	Sem_set(semid_imus);
	semid_foot = Sem_creat_FOOT();
	Sem_set(semid_foot);
	semid_DATA = Sem_creat_DATA();
	Sem_set(semid_DATA);
	semid_timer = Sem_creat_TIMER();
	Sem_set(semid_timer);
	//printf("semid_GLB: %d\nsemid_ACT: %d\nsemid_TCP: %d\nsemid_ADC: %d\nsemid_IMU: %d\nsemid_DATA: %d\n", semid_glbs, semid_ACT, semid_TCP, semid_adcs, semid_imus, semid_DATA);
}


//	锟斤拷始锟斤拷锟斤拷锟斤拷锟节达�?
static void init_shm()
{
	shmid_glbs = Shm_creat_glb(sizeof(GLB));
	void *shm_tmp = NULL;
	//映锟戒本锟斤拷指锟斤�?
	shm_tmp = Shm_mat(shmid_glbs);
	p_shm_GLB = (GLB *)shm_tmp;


	//*********************ADC*******************
	//
	//****************************************
	shmid_adcs = Shm_creat_adc(sizeof(SHM_ADC_t));
	shm_tmp = NULL;
	//映锟戒本锟斤拷指锟斤�?
	shm_tmp = Shm_mat(shmid_adcs);
	p_shm_ADC = (SHM_ADC_t *)shm_tmp;

	//*********************IMU*******************
	//
	//****************************************
	shmid_imus = Shm_creat_imu(sizeof(SHM_IMU_t));
	shm_tmp = NULL;
	//映锟戒本锟斤拷指锟斤�?
	shm_tmp = Shm_mat(shmid_imus);
	p_shm_IMU = (SHM_IMU_t *)shm_tmp;

	//*********************FOOT*******************
	//
	//******************************************
	shmid_foot = Shm_creat_foot(sizeof(SHM_FT_DATA));
	shm_tmp = NULL;
	//映锟戒本锟斤拷指锟斤�?
	shm_tmp = Shm_mat(shmid_foot);
	p_shm_FOOT = (SHM_FT_DATA *)shm_tmp;

	//*********************DATA*******************
	//
	//****************************************
	shmid_DATA = Shm_creat_data(sizeof(SHM_DATA_t));
	shm_tmp = NULL;
	//映锟戒本锟斤拷指锟斤�?
	shm_tmp = Shm_mat(shmid_DATA);
	p_shm_DATA = (SHM_DATA_t *)shm_tmp;



//*********************ACT*******************
//
//******************************************
	shmid_ACT = Shm_creat_ACT(sizeof(SHM_ACT_DATA));
	shm_tmp = NULL;
	//映锟戒本锟斤拷指锟斤�?
	shm_tmp = Shm_mat(shmid_ACT);
	p_shm_ACT = (SHM_ACT_DATA *)shm_tmp;


//*********************TCP*******************
//
//******************************************
	shmid_TCP = Shm_creat_TCP(sizeof(SHM_TCP_t));
	shm_tmp = NULL;
	//映锟戒本锟斤拷指锟斤�?
	shm_tmp = Shm_mat(shmid_TCP);
	p_shm_TCP = (SHM_TCP_t *)shm_tmp;


//*********************TIMER*******************
//
//******************************************
	shmid_timer = Shm_creat_TIMER(sizeof(SHM_TIMER_t));
	shm_tmp = NULL;
	//映锟戒本锟斤拷指锟斤�?
	shm_tmp = Shm_mat(shmid_timer);
	p_shm_TIMER = (SHM_TIMER_t *)shm_tmp;

}

//写锟斤拷MAIN锟斤拷PID锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷�?0锟斤�?
static void init_glbs()
{
	pid_main = getpid();
	Sem_P(semid_glbs);
	p_shm_GLB->PID.PID_ADC = 0;
	p_shm_GLB->PID.PID_DATA = 0;
	p_shm_GLB->PID.PID_DSP = 0;
	p_shm_GLB->PID.PID_IMU = 0;
	p_shm_GLB->PID.PID_MAIN_TCP = 0;
	p_shm_GLB->PID.PID_RCG = 0;
	p_shm_GLB->PID.PID_FOOT = 0;
	p_shm_GLB->PID.PID_MAIN = pid_main;
	p_shm_GLB->proc_status.en_dsp = 0;
	p_shm_GLB->proc_status.en_rcg = 0;
	p_shm_GLB->proc_status.en_tcp = 0;
	p_shm_GLB->proc_status.proc_TCP_status = STATUS_OFF;
	p_shm_GLB->proc_status.proc_status_proc = STATUS_ON;
	p_shm_GLB->proc_status.proc_ACT_status = STATUS_OFF;
	p_shm_GLB->proc_status.T_status = STATUS_OFF;
	p_shm_GLB->proc_status.ACT_OVER = 0;
	Sem_V(semid_glbs);
}

static void proc_monitor()
{
	while (1)
	{
		usleep(1000);
		if (proc_status == STATUS_OFF)
		{
			APP_exit();
		}
	}
}


static void  init_proc()
{
	pid_t pid;
	if ((pid = fork()) < 0)
	{
		perror("fork error!");
	}
	else if (pid == 0)
	{
// DATA ***********
		main_data();
	}
	else
	{
		if ((pid = fork()) < 0)
		{
			perror("fork error!");
		}
		else if (pid == 0)
		{

//DSP   *************
			main_dsp();
		}
		else
		{
			if ((pid = fork()) < 0)
			{
				perror("fork error!");
			}
			else if (pid == 0)
			{
//TCP   ***************
				if(tcp_flag == 1)
				{
					main_can();
				}
				else
				{
					main_tcp();
				}
			}
			else
			{
				if ((pid = fork()) < 0)
				{
					perror("fork error!");
				}
				else if (pid == 0)
				{
//ADC   *****************
					#ifdef ADC_MODE_OL
					data_ADC_main();
					#else
					pid_t temppid_ADC = getpid();
					Sem_P(semid_glbs);
					p_shm_GLB->PID.PID_ADC = temppid_ADC;
					Sem_V(semid_glbs);
					printf("adc offline\n");
					#endif
				}
				else
				{
					if ((pid = fork()) < 0)
					{
						perror("fork error!");
					}
					else if (pid == 0)
					{
//IMU   ****************
						#ifdef IMU_MODE_OL
						data_IMU_main();
						#else
						pid_t temppid_imu = getpid();
						Sem_P(semid_glbs);
						p_shm_GLB->PID.PID_IMU = temppid_imu;
						Sem_V(semid_glbs);
						printf("imu offline\n");
						#endif
					}
					else
					{
						if ((pid = fork()) < 0)
						{
							perror("fork error!");
						}
						else if (pid == 0)
						{
//RCG  ****************
							dsp_RCG_main();
						}
						else
						{
							if ((pid = fork()) < 0) 
							{
								perror("fork error!");
							}
							else if (pid == 0)
							{
//FOOT							
								#ifdef FOOT_MODE_OL
								data_FOOT_main();
								#else
								pid_t temppid_foot = getpid();
								Sem_P(semid_glbs);
								p_shm_GLB->PID.PID_FOOT = temppid_foot;
								Sem_V(semid_glbs);
								printf("foot offline\n");
								#endif
							}
							else 
							{
//MAIN	*****************
								proc_monitor();
							}	
						}
					}
				}
			}
		}
	}
}



using namespace std;
int main(int argc, char** argv)
{
	if(argc >1 )
    {
        if(strcmp(argv[1],"tcp") == 0)
		{
			tcp_flag = 0;
		}
		else if(strcmp(argv[1],"can") == 0)
		{
			tcp_flag = 1;
		}
		else
		{
			tcp_flag = 0;
		}
    }
	else
		tcp_flag = 0;
	


	init_sig();		//	1.锟斤拷始锟斤拷锟脚猴拷锟斤拷�?
	init_sem();		//	2.锟斤拷始锟斤拷锟脚猴拷锟斤拷
	init_shm();		//	3.锟斤拷始锟斤拷锟斤拷锟斤拷锟节达�?
	init_glbs();
	init_proc();
}
