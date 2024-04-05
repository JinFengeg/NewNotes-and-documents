#pragma once
#include <sys/time.h>
#include "tensorflow/lite/examples/LowerMachine/data_ADC_main.h"
#include "tensorflow/lite/examples/LowerMachine/data_IMU_main.h"
#include"tensorflow/lite/examples/LowerMachine/data_FOOT_main.h"
#include "tensorflow/lite/examples/LowerMachine/main_data.h"
#include "tensorflow/lite/examples/LowerMachine/main_dsp.h"
#include "tensorflow/lite/examples/LowerMachine/main_tcp.h"
#include <unistd.h>

#define STATUS_OFF 0
#define STATUS_ON 1


//锟斤拷时锟斤拷锟结构锟斤拷
struct SHM_TIMER_t
{
	struct timeval Timer1;
	struct timeval Timer2;
	struct timeval Timer3;
	struct timeval Timer4;
	struct timeval Timer5;
	struct timeval Timer6;
};


struct PID_t
{
	pid_t PID_MAIN_TCP; //锟斤拷锟斤拷锟斤拷
	pid_t PID_IMU; 
	pid_t PID_DSP;
	pid_t PID_ADC;
	pid_t PID_RCG;
	pid_t PID_DATA;
	pid_t PID_FOOT;
	pid_t PID_MAIN;
};
//一锟斤拷5锟斤拷锟斤拷锟教ｏ拷锟斤拷锟斤拷main_tcp, 锟接ｏ拷dsp锟斤拷 imu锟斤拷 adc , dsp锟斤拷
struct proc_status_t
{

	int proc_status_proc;
	int en_dsp;
	int en_rcg;
	int en_tcp;
	int ACT_OVER;
	int proc_TCP_status;
	int proc_ACT_status;   //通知rcg锟斤拷锟斤拷
	int T_status;   //锟斤拷时状态志锟斤拷0为锟斤拷始值锟斤拷1锟斤拷始
};



struct GLB 
{
	struct PID_t PID;
	struct proc_status_t proc_status;
};


//锟脚猴拷锟斤拷
extern int semid_adcs;
extern int semid_imus;
extern int semid_foot;
extern int semid_DATA;
extern int semid_ACT;
extern int semid_TCP;
extern int semid_glbs;
extern int semid_timer;


//锟斤拷锟斤拷锟节达拷
extern int shmid_adcs;
extern int shmid_imus;
extern int shmid_foot;
extern int shmid_DATA;
extern int shmid_ACT;
extern int shmid_TCP;
extern int shmid_glbs;
extern int shmid_timer;

//指锟斤拷
extern GLB *p_shm_GLB;
extern SHM_ADC_t *p_shm_ADC;
extern SHM_IMU_t *p_shm_IMU;
extern SHM_FT_DATA  *p_shm_FOOT;
extern SHM_DATA_t *p_shm_DATA;
extern SHM_ACT_DATA *p_shm_ACT;
extern SHM_TCP_t *p_shm_TCP;
extern SHM_TIMER_t *p_shm_TIMER;

