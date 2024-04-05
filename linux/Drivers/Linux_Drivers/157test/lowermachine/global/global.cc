#include "tensorflow/lite/examples/LowerMachine/global.h"

//锟脚猴拷锟斤拷
int semid_adcs;
int semid_imus;
int semid_foot;
int semid_DATA;
int semid_ACT;
int semid_TCP;
int semid_glbs;
int semid_timer;




//锟斤拷锟斤拷锟节达拷
int shmid_adcs;
int shmid_imus;
int shmid_foot;
int shmid_DATA;
int shmid_ACT;
int shmid_TCP;
int shmid_glbs;
int shmid_timer;




//指锟斤拷
GLB *p_shm_GLB;
SHM_ADC_t *p_shm_ADC;
SHM_IMU_t *p_shm_IMU;
SHM_FT_DATA *p_shm_FOOT;
SHM_DATA_t *p_shm_DATA; //锟缴硷拷同锟斤拷之锟斤拷锟斤拷锟斤拷锟斤拷锟�40锟斤拷锟斤拷
SHM_ACT_DATA *p_shm_ACT; //识锟斤拷锟斤拷锟斤拷
SHM_TCP_t *p_shm_TCP;
SHM_TIMER_t *p_shm_TIMER;

