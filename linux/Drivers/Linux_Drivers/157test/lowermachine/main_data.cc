#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

#include "tensorflow/lite/examples/LowerMachine/main_data.h"
#include "tensorflow/lite/examples/LowerMachine/data_ADC_main.h"
#include "tensorflow/lite/examples/LowerMachine/data_IMU_main.h"
#include "tensorflow/lite/examples/LowerMachine/data_FOOT_main.h"
#include "tensorflow/lite/examples/LowerMachine/config.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/data_WIN.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"
#include "tensorflow/lite/examples/LowerMachine/shm.h"
#include "tensorflow/lite/examples/LowerMachine/filt_ADC.h"
//#include "data_ADC_OFL_main.h"


//锟斤拷时锟斤拷锟斤拷pid
static pid_t pid_dsp = 0;
static pid_t pid_DATA = 0;
static pid_t pid_main = 0;

//锟斤拷锟斤拷状态志
static int proc_status = STATUS_ON;

//时锟斤拷锟斤拷锟斤拷锟�
static unsigned int ms_T_count = 0;
static int W_count = 0;

//锟侥硷拷锟斤拷锟斤拷锟斤拷
static FILE* fp_ADC;
static FILE* fp_IMU;
static FILE* fp_IMU_orignal;
static FILE* fp_FT;
static FILE* fp_ADC_OFF;
static FILE* fp_IMU_OFF;

static std::ifstream emgin;
static std::ifstream imuin;




//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
static data_WIN *DATA_WIN = new data_WIN;


static void APP_exit()/*一样的操作，退出的话关闭文件*/
{
	delete[] DATA_WIN;
	fclose(fp_IMU);
	fclose(fp_ADC);
	fclose(fp_FT);
	fclose(fp_IMU_orignal);
	printf("=============== DATA EXIT! ==============\n");
	exit(0);
}

static void SigH(int Sig)/*这是信号处理函数*/
{
	switch (Sig)
	{
	case SIGINT:
		break;
	case SIGALRM:
		ms_T_count += 1;
		break;
	case SIGUSR1:
		break;
	case SIGUSR2:
		proc_status = STATUS_OFF;
		break;
	default:
		break;
	}
}

static void T_WIN_SLD()
{
	Sem_P(semid_timer);
	p_shm_TIMER->Timer1.tv_sec = p_shm_TIMER->Timer2.tv_sec;
	p_shm_TIMER->Timer1.tv_usec = p_shm_TIMER->Timer2.tv_usec;
	p_shm_TIMER->Timer2.tv_sec = p_shm_TIMER->Timer3.tv_sec;
	p_shm_TIMER->Timer2.tv_usec = p_shm_TIMER->Timer3.tv_usec;
	p_shm_TIMER->Timer3.tv_sec = p_shm_TIMER->Timer4.tv_sec;
	p_shm_TIMER->Timer3.tv_usec = p_shm_TIMER->Timer4.tv_usec;
	p_shm_TIMER->Timer4.tv_sec = p_shm_TIMER->Timer5.tv_sec;
	p_shm_TIMER->Timer4.tv_usec = p_shm_TIMER->Timer5.tv_usec;
	p_shm_TIMER->Timer5.tv_sec = p_shm_TIMER->Timer6.tv_sec;
	p_shm_TIMER->Timer5.tv_usec = p_shm_TIMER->Timer6.tv_usec;
	gettimeofday(&(p_shm_TIMER->Timer6), NULL);
	Sem_V(semid_timer);
}

//锟斤拷装锟脚猴拷
static void init_sig()/*注册信号与处理函数*/
{
	signal(SIGINT, SigH);
	signal(SIGUSR1, SigH);
	signal(SIGALRM, SigH);
	signal(SIGUSR2, SigH);
}

//	锟斤拷始锟斤拷锟斤拷时锟斤拷
static bool init_timer()/*初始化定时器*/
{
	struct itimerval timer_val;
	timer_val.it_value.tv_sec = 0;
	timer_val.it_value.tv_usec = 1000;				//1ms之锟斤拷执锟斤拷
	timer_val.it_interval.tv_sec = 0;
	timer_val.it_interval.tv_usec = 1000;			//锟斤拷锟�1ms
	setitimer(ITIMER_REAL, &timer_val, NULL);
	W_count = 0;
	return true;
}


static void wait_ADC()
{
	#ifdef ADC_MODE_OL
	int flag = false;
	while (!flag) {
		if (!Sem_P(semid_adcs))
		{
			printf("3\n");
			exit(1);
		}
		flag = p_shm_ADC->nflag;
		p_shm_ADC->nflag = false;                                
		Sem_V(semid_adcs);
	}
	#endif
	printf(".......ADC.......OK !\n");
}


static void wait_IMU()
{
	#ifdef IMU_MODE_OL
	int flag = false;
	while (!flag) {
		if (!Sem_P(semid_imus))
		{
			printf("4\n");
			exit(1);
		}
		flag = p_shm_IMU->nflag;
		p_shm_IMU->nflag = false;
		Sem_V(semid_imus);
	}
	#endif
	printf(".......IMU.......OK !\n");
}

static void wait_FT()
{
	#ifdef FOOT_MODE_OL
	int flag = false;
	while (!flag)
	{
		if (!Sem_P(semid_foot))
		{
			printf("4\n");
			exit(1);
		}
		flag = p_shm_FOOT->nflag;
		p_shm_FOOT->nflag = false;
		Sem_V(semid_foot);
	}
	#endif
	printf(".......FOOT......OK !\n");
}


static void init_OFL()
{
#ifdef ADC_OF_DATA
	fp_ADC = fopen(ADC_FILE_NAME, "w");
	if (NULL == fp_ADC) 
	{
		printf("ERROR: Open file %s !\n ", ADC_FILE_NAME);
	}
#endif

#ifdef IMU_OF_DATA
	fp_IMU = fopen(IMU_FILE_NAME, "w");
	fp_IMU_orignal = fopen("file_IMU_Orignal.csv", "w");
	if (NULL == fp_IMU || NULL==fp_IMU_orignal)
	{
		printf("ERROR: Open file %s !\n ", IMU_FILE_NAME);
	}

	for (int i=0;i<IMU_CH;i++)
	{
	fprintf(fp_IMU, ",ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw");
	}
	fprintf(fp_IMU, "\n");

	for (int i=0;i<IMU_CH;i++)
	{
	fprintf(fp_IMU_orignal, ",ACC_x,ACC_y,ACC_z,GYR_x,GYR_y,GYR_z,HX,HY,Hz,ROll,Pitch,Yaw");
	}
	fprintf(fp_IMU_orignal, "\n");


#endif 

#ifdef FT_OF_DATA
	fp_FT = fopen(FT_FILE_NAME, "w");
	if (NULL == fp_FT)
	{
		printf("ERROR: Open file %s !\n ", FT_FILE_NAME);
	}
#endif 



#ifdef ADC_MODE_OL
#else
	//绂荤嚎ADC鏂囦欢
	emgin.open("./data/file_ADC2.csv");
	if(!emgin.is_open())
	{
		std::cout<<"load emg error";
	}
	imuin.open("./data/file_IMU2.csv");
	if(!imuin.is_open())
	{
		std::cout<<"load IMU error";
	}
	std::string buff;
	getline(emgin,buff);
#endif

#ifdef IMU_MODE_OL
#else
#endif
}

static bool ADC_getData(float pData[8])
{
	using namespace std;
	std::string buff;
	if(getline(emgin,buff)){
		istringstream str_in;
		str_in.str(buff);
		string temp;
		int ch=0;
		while(getline(str_in,temp,',')){
			if(ch == 0){
				ch++;
				continue;
			}
			float data = (float)atof(temp.c_str());
			pData[ch-1] = data;
			ch++;
			temp.clear();
		}
	}
	else 
		return false;
	static long Ts;
	bool result = true;
	return result;
}

static void OFL_ADC(unsigned int Tnow, float SRC_Data[])
{
	fprintf(fp_ADC, "%d", Tnow);
	for (int n = 0; n < 8; n++) {
		fprintf(fp_ADC, ",%.6f", SRC_Data[n]);
	}
	fprintf(fp_ADC, "\n");
//	fflush(fp_ADC);
}

static void SHM_WRITE_ADC(unsigned int Tnow)
{
	static unsigned long T_count_ms_last_adc = Tnow;
	static float SRC_Data[8];
	static float FLT_Data[8];
	static unsigned int Ts = 1;
	if (Tnow - T_count_ms_last_adc >= Ts)
	{
		
		//鍦ㄧ嚎璇诲彇鏁版嵁
		T_count_ms_last_adc = Tnow;

		#ifdef ADC_MODE_OL
		if (!Sem_P(semid_adcs))
		{
			printf("4\n");
			exit(1);
		}
		for (int _CH = 0; _CH < 8; _CH++) 
		{
			SRC_Data[_CH] = p_shm_ADC->Data_src_CH[_CH];
			FLT_Data[_CH] = p_shm_ADC->Data_filt_CH[_CH];
		}
		p_shm_ADC->nflag = false;
		Sem_V(semid_adcs);
		#else
		//璇诲彇绂荤嚎鏂囦欢
		if (!ADC_getData(SRC_Data))
		{
			kill(pid_main, SIGINT);
		}
		ADC_filt(FLT_Data, SRC_Data);
		//FLT_Data = SRC_Data;
		for (int _CH = 0; _CH < 8; _CH++)
		{
			FLT_Data[_CH] = SRC_Data[_CH];
		}
		#endif
		
		//璁板綍鏁版嵁
		#ifdef ADC_OF_DATA
		OFL_ADC(Tnow, SRC_Data);
		#endif

		DATA_WIN->Append_ADC(FLT_Data);
	}
}


//
static bool OFL_IMU(int time, float pSrc[8][12])
{
	fprintf(fp_IMU, "%d", time);
	for (int _CH = 0; _CH < IMU_CH; _CH++)
	{
		//fprintf(fp_IMU, "\t");
		for (int n = 0; n < 12; n++)
		{
			fprintf(fp_IMU, ",%.6f", pSrc[_CH][n]);
		}
	}
	fprintf(fp_IMU, "\n");
//	fflush(fp_IMU);
	return true;
}

static bool OFL_IMU_orignal(int time, float pSrc[8][12])
{
	fprintf(fp_IMU_orignal, "%d", time);
	for (int _CH = 0; _CH < IMU_CH; _CH++)
	{
		//fprintf(fp_IMU, "\t");
		for (int n = 0; n < 12; n++)
		{
			fprintf(fp_IMU_orignal, ",%.6f", pSrc[_CH][n]);
		}
	}
	fprintf(fp_IMU_orignal, "\n");
	return true;
}

//锟斤拷锟揭伙拷锟斤拷锟斤拷锟絀MU锟斤拷锟捷ｏ拷 写锟诫窗锟斤拷
static void SHM_WRITE_IMU(unsigned int Tnow)
{
	static unsigned long T_count_ms_last = Tnow;
	static float Data_tmp_src[8][12];		//IMU 锟斤拷锟捷伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷8通锟斤拷锟斤拷每通锟斤拷12锟斤拷锟斤拷锟斤拷
	static float Data_tmp_src_ori[8][12];
	static float Data_tmp_dest[8][12];
	static int unsigned Ts = 10;
	if (Tnow - T_count_ms_last >= Ts)
	{
		T_count_ms_last = Tnow;
		//	锟接讹拷应锟侥癸拷锟斤拷锟节达拷锟斤拷锟斤拷锟�
#ifdef IMU_MODE_OL
		if (!Sem_P(semid_imus))
		{
			printf("5\n");
			exit(1);
		}
		for (int _CH = 0; _CH <IMU_CH; _CH++)
		{
			for (int n = 0; n < 12; n++)
			{
				Data_tmp_src[_CH][n] = p_shm_IMU->Data_src[_CH][n];
			}
		}
		Sem_V(semid_imus);
#else
		using namespace std;
		string buff;
		string temp;
		if(getline(imuin,buff)){
			istringstream str_in;
			str_in.str(buff);
			int j=-1;
			while(getline(str_in,temp,',')){
				if(j==-1){
					++j;
					temp.clear();
					continue;
				}
				float data = (float)atof(temp.c_str()); //stof(temp);
				int ch = j / 12;  //鍙栨ā
				int col =j % 12; //鍙栦綑
				Data_tmp_src[ch][col] = data;
				j++;
				temp.clear();
			}
		}
#endif
		IMU_filter(Data_tmp_dest,Data_tmp_src);
#ifdef IMU_OF_DATA
		OFL_IMU(Tnow, Data_tmp_src);	
#endif
		DATA_WIN->Append_IMU(Data_tmp_dest);
	}

}

//
static void OFL_FT(unsigned int Tnow, int FT_Data[])
{
	fprintf(fp_FT, "%d", Tnow);
	for (int n = 0; n < 4; n++)
	{
		fprintf(fp_FT, ",%d", FT_Data[n]);
	}
	fprintf(fp_FT, "\n");
}

static void SHM_WRITE_FT(unsigned int Tnow)
{
	static unsigned long T_count_ms_last_adc = Tnow;
	static int FT_Data[4]={1,1,1,1};
	if (Tnow - T_count_ms_last_adc >= 1)
	{
		T_count_ms_last_adc = Tnow;
		#ifdef FOOT_MODE_OL
		Sem_P(semid_foot);
		for (int i = 0; i < 4; i++)
		{
			FT_Data[i] = p_shm_FOOT->FT_DATA[i];
		}
		p_shm_ADC->nflag = false;
		Sem_V(semid_foot);
		#endif 
#ifdef FT_OF_DATA
		OFL_FT(Tnow, FT_Data);
#endif
	}
}


//
static void init_glbs()
{
	//
	pid_DATA = getpid();
	if (!Sem_P(semid_glbs))
	{
		printf("46\n");
		exit(1);
	}
	p_shm_GLB->PID.PID_DATA = pid_DATA;
	Sem_V(semid_glbs);

	//
	do 
	{
		usleep(300);
		if (!Sem_P(semid_glbs))
		{
			printf("6\n");
			exit(1);
		}
		pid_dsp = p_shm_GLB->PID.PID_DSP;
		pid_main = p_shm_GLB->PID.PID_MAIN;
		Sem_V(semid_glbs);
		
	} while (pid_dsp == 0 || pid_main == 0);
}

static void init_WIN_SLD()
{
	Sem_P(semid_timer);
	memset(p_shm_TIMER,0,sizeof(SHM_TIMER_t));
	Sem_V(semid_timer);
}


void main_data()
{
	init_sig();
	init_glbs();
	wait_ADC();
	wait_IMU();
	wait_FT();
	// wait_TCP();
	init_OFL();  	//锟斤拷锟缴硷拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟竭★拷
	int Tnow;
	init_WIN_SLD();
	printf("鍒濆鍖栬鏃跺櫒\n");
	init_timer();
	while (true)
	{	//	锟斤拷锟斤拷锟斤拷锟捷伙拷锟斤拷锟斤拷锟侥凤拷锟斤拷循锟斤拷
		if (proc_status == STATUS_OFF)
		{
			APP_exit();
		}
		Tnow = ms_T_count;
		//锟斤拷时锟斤拷锟斤拷锟斤拷锟斤拷每锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷一锟轿★拷锟节革拷锟斤拷锟斤拷锟斤拷之前锟斤拷锟叫硷拷时锟斤拷
		if (W_count == 0)
		{
			W_count++;
			T_WIN_SLD();
		}
		SHM_WRITE_ADC(Tnow);		//锟斤拷一锟斤拷锟斤拷锟斤拷锟叫讹拷取  35  锟斤拷锟斤拷通锟斤拷锟斤拷锟捷ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟绞憋拷锟酵拷锟斤拷锟斤拷萁峁癸拷小锟�
		SHM_WRITE_IMU(Tnow);
		SHM_WRITE_FT(Tnow);
		//锟叫讹拷锟角诧拷锟角达拷锟斤拷锟斤拷40锟斤拷.
		if (DATA_WIN->Enable())
		{
			//
			W_count = 0;
			Sem_P(semid_DATA);
			DATA_WIN->DataCpy(p_shm_DATA, &(DATA_WIN->DATW));	//锟斤拷锟斤拷锟斤拷锟捷匡拷锟斤拷锟斤拷锟斤拷锟斤拷锟节达拷
			p_shm_DATA->Tnow = Tnow;
			Sem_V(semid_DATA);
			DATA_WIN->Clear();
			Sem_P(semid_glbs);
			p_shm_GLB->proc_status.en_dsp = true;
			Sem_V(semid_glbs);
			kill(pid_dsp, 10);
		}
		//usleep(10);
	}
}
