#include "tensorflow/lite/examples/LowerMachine/global.h"

//閿熻剼鐚存嫹閿熸枻鎷?
int semid_adcs;
int semid_imus;
int semid_foot;
int semid_DATA;
int semid_ACT;
int semid_TCP;
int semid_glbs;
int semid_timer;




//閿熸枻鎷烽敓鏂ゆ嫹閿熻妭杈炬嫹
int shmid_adcs;
int shmid_imus;
int shmid_foot;
int shmid_DATA;
int shmid_ACT;
int shmid_TCP;
int shmid_glbs;
int shmid_timer;




//鎸囬敓鏂ゆ嫹
GLB *p_shm_GLB;//这是自定义的结构体里面有进程号和proc_status_t进程状态结构体
SHM_ADC_t *p_shm_ADC;/*自定义的结构体，就是用来存放数据数组的*/
SHM_IMU_t *p_shm_IMU;
SHM_FT_DATA *p_shm_FOOT;
SHM_DATA_t *p_shm_DATA; //閿熺即纭锋嫹鍚岄敓鏂ゆ嫹涔嬮敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟?40閿熸枻鎷烽敓鏂ゆ嫹
SHM_ACT_DATA *p_shm_ACT; //璇嗛敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
SHM_TCP_t *p_shm_TCP;
SHM_TIMER_t *p_shm_TIMER;
void SigH(int arg)
{
switch(arg)
	{
	case SIGINT:/*接收到SIGINT没有区别*/
		break;
	case 10:
		break;
	case 12:/*当接收到信号宏为12就设置进程状态标志位为OFF*/
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

//??adc?????д??glb
static void init_glbs()/*用于初始化全局变量*/
{
	pid_ADC = getpid();/*获取当前进程号*/
	do {
		usleep(300);
		Sem_P(semid_glbs);/*semid_glbs作为信号量的标识符，对该信号量进行P操作也就是获取信号量的操作*/
		p_shm_GLB->PID.PID_ADC = pid_ADC;//将当前的进程号传入自定义的结构体中
		pid_main = p_shm_GLB->PID.PID_MAIN;//这两行不知道是干什么用的
		Sem_V(semid_glbs);/*最后释放获取到的信号量*/
	} while (pid_main == 0);
}

static void APP_exit()
{
#ifdef ADC_MODE_OL
	delete ADC;
#endif
	printf("=============== ADC  EXIT! ==============\n");
	fflush(stdout);/*刷新标准输出文件的缓冲区，刷新到内核中*/
	exit(0);/*结束进程，其实最终都是调用do_exit函数实现进程的终结，释放大部分资源，但是进程控制块也就是进程描述符、thread_info
	以及内核栈并没有被释放，是父进程为其收尸的时候需要提取子进程尸体上的信息，最后由父进程释放掉*/
}

/*本文件主要是对采集到的ad7606八通道数据进行滤波，然后引入了信号量实现进程间通信，但是光看本文件看不出什么*/


void data_ADC_main()
{
	init_sig();/*注册信号处理函数*/
	init_glbs();/*对当前进程做了什么不可告人的操作*/
	//ADC???????
#ifdef ADC_MODE_OL
	ADC->init();//用于配置ad7606芯片
	printf("ONLINE***********\n");
#else
	//ADC_init();
	printf("OFFLINE***********\n");
#endif
	Sem_P(semid_adcs);
	p_shm_ADC->nflag = false;
	Sem_V(semid_adcs);
	while (1)/*最后陷入死循环，轮询判断*/
	{
		usleep(200);
		if (proc_status == STATUS_OFF)/*如果接收到信号12，就跳出循环*/
		{
			APP_exit();
		}
#ifdef ADC_MODE_OL
		ADC->getData(SRC_Data);//ADC_OL提供的获取数据函数，最终读取并处理得到的浮点数存放在SRC_Data数组中
#endif
		ADC_filt(FLT_Data, SRC_Data);/*通过此函数可以对FLT_Data数组赋值，对SRC_Data进行带宽处理结果赋值给FLT_Data数组*/
		//
		Sem_P(semid_adcs);/*这里的adc应该就是对应ad7606的进程了吧*/
		for (int _CH = 0; _CH < 8; _CH++) 
		{
			p_shm_ADC->Data_src_CH[_CH] = SRC_Data[_CH];
			p_shm_ADC->Data_filt_CH[_CH] = FLT_Data[_CH];/*将读取到的八通道数据以及过滤后的数据存入p_shm_ADC
			*/
		}
		p_shm_ADC->nflag = true;
		Sem_V(semid_adcs);
	}
}