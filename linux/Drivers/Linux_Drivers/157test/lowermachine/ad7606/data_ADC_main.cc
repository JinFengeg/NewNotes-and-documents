#include "tensorflow/lite/examples/LowerMachine/global.h"

//锟脚猴拷锟斤�?
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
GLB *p_shm_GLB;//�����Զ���Ľṹ�������н��̺ź�proc_status_t����״̬�ṹ��
SHM_ADC_t *p_shm_ADC;/*�Զ���Ľṹ�壬��������������������*/
SHM_IMU_t *p_shm_IMU;
SHM_FT_DATA *p_shm_FOOT;
SHM_DATA_t *p_shm_DATA; //锟缴硷拷同锟斤拷之锟斤拷锟斤拷锟斤拷锟斤拷锟�?40锟斤拷锟斤拷
SHM_ACT_DATA *p_shm_ACT; //识锟斤拷锟斤拷锟斤拷
SHM_TCP_t *p_shm_TCP;
SHM_TIMER_t *p_shm_TIMER;
void SigH(int arg)
{
switch(arg)
	{
	case SIGINT:/*���յ�SIGINTû������*/
		break;
	case 10:
		break;
	case 12:/*�����յ��źź�Ϊ12�����ý���״̬��־λΪOFF*/
		proc_status = STATUS_OFF;
		break;
	default:
		break;
	}
}
/*ע���źŴ�����*/
static void init_sig()
{
	signal(SIGINT, SigH);
	signal(10, SigH);
	signal(12, SigH);
}

//??adc?????��??glb
static void init_glbs()/*���ڳ�ʼ��ȫ�ֱ���*/
{
	pid_ADC = getpid();/*��ȡ��ǰ���̺�*/
	do {
		usleep(300);
		Sem_P(semid_glbs);/*semid_glbs��Ϊ�ź����ı�ʶ�����Ը��ź�������P����Ҳ���ǻ�ȡ�ź����Ĳ���*/
		p_shm_GLB->PID.PID_ADC = pid_ADC;//����ǰ�Ľ��̺Ŵ����Զ���Ľṹ����
		pid_main = p_shm_GLB->PID.PID_MAIN;//�����в�֪���Ǹ�ʲô�õ�
		Sem_V(semid_glbs);/*����ͷŻ�ȡ�����ź���*/
	} while (pid_main == 0);
}

static void APP_exit()
{
#ifdef ADC_MODE_OL
	delete ADC;
#endif
	printf("=============== ADC  EXIT! ==============\n");
	fflush(stdout);/*ˢ�±�׼����ļ��Ļ�������ˢ�µ��ں���*/
	exit(0);/*�������̣���ʵ���ն��ǵ���do_exit����ʵ�ֽ��̵��սᣬ�ͷŴ󲿷���Դ�����ǽ��̿��ƿ�Ҳ���ǽ�����������thread_info
	�Լ��ں�ջ��û�б��ͷţ��Ǹ�����Ϊ����ʬ��ʱ����Ҫ��ȡ�ӽ���ʬ���ϵ���Ϣ������ɸ������ͷŵ�*/
}

/*���ļ���Ҫ�ǶԲɼ�����ad7606��ͨ�����ݽ����˲���Ȼ���������ź���ʵ�ֽ��̼�ͨ�ţ����ǹ⿴���ļ�������ʲô*/


void data_ADC_main()
{
	init_sig();/*ע���źŴ�����*/
	init_glbs();/*�Ե�ǰ��������ʲô���ɸ��˵Ĳ���*/
	//ADC???????
#ifdef ADC_MODE_OL
	ADC->init();//��������ad7606оƬ
	printf("ONLINE***********\n");
#else
	//ADC_init();
	printf("OFFLINE***********\n");
#endif
	Sem_P(semid_adcs);
	p_shm_ADC->nflag = false;
	Sem_V(semid_adcs);
	while (1)/*���������ѭ������ѯ�ж�*/
	{
		usleep(200);
		if (proc_status == STATUS_OFF)/*������յ��ź�12��������ѭ��*/
		{
			APP_exit();
		}
#ifdef ADC_MODE_OL
		ADC->getData(SRC_Data);//ADC_OL�ṩ�Ļ�ȡ���ݺ��������ն�ȡ������õ��ĸ����������SRC_Data������
#endif
		ADC_filt(FLT_Data, SRC_Data);/*ͨ���˺������Զ�FLT_Data���鸳ֵ����SRC_Data���д���������ֵ��FLT_Data����*/
		//
		Sem_P(semid_adcs);/*�����adcӦ�þ��Ƕ�Ӧad7606�Ľ����˰�*/
		for (int _CH = 0; _CH < 8; _CH++) 
		{
			p_shm_ADC->Data_src_CH[_CH] = SRC_Data[_CH];
			p_shm_ADC->Data_filt_CH[_CH] = FLT_Data[_CH];/*����ȡ���İ�ͨ�������Լ����˺�����ݴ���p_shm_ADC
			*/
		}
		p_shm_ADC->nflag = true;
		Sem_V(semid_adcs);
	}
}