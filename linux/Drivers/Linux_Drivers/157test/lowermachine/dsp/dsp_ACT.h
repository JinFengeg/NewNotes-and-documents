/*************************************************
*		��ζ˵���
*************************************************/
#pragma once
#include <stdio.h>
#include <sys/time.h>
#include <sys/signal.h>
#include "tensorflow/lite/examples/LowerMachine/config.h"
#include "tensorflow/lite/examples/LowerMachine/main_data.h"
#include "tensorflow/lite/examples/LowerMachine/main_dsp.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/Sem.h"



class dsp_ACTClass
{
public:
	dsp_ACTClass();
	~dsp_ACTClass();
public:
	bool Init(pid_t PID);		//��ʼ��
	bool CpyD2A(SHM_DATA_t *pSrc);		//���ݰ����ݱ��ݵ�DATW
	bool CpyA2A(SHM_ACT_DATA *pDest);	//�� ACTDATA�е����ݿ�����Ŀ������
	bool Check();	//��ζ˵���
	bool EnAble();		//�����200����
	bool Reset();	//��μ�⸴λ
	bool Clear(SHM_ACT_DATA *pDest);	//��ջ�����ݻ���
	bool Clear();
	SHM_ACT_DATA ACT_DATA;	//�������
//private:
public:
	SHM_DATA_t DATW;
	SHM_ACT_DATA ACT_DATA_tmp;

	//int ACT_status_t; //�ж��Ƿ��ڻ���ڡ�  0�����ǻ�Σ� 1�������ڻ�Ρ�
	int T_status;  //0������ʼ��ʱ�� 1����֪ͨ����ʱ�ˡ�
	int ACT_STATUS = 0;   //1 ȷ����ν���������֪ͨrcg���̾���״̬��0��ʼ��
	pid_t pid_RCG;
	int W_cnt = 0;	//���ݰ�����
	int T_now = 0;	//��ǰ�������ݰ�ʱ���
	int W_cnt_S = 0;	//�������
	int W_cnt_E = 0;	//��ζ˵��
	float SLPW_buf[SLPW_LEN] = { 0 };	//����������
	float SampEN_num = 0;	//�����ؼ�����
	int Status = 0;		//��ǰ��ζ˵���״̬Status
	bool EN_flag = false;  //��⵽������׼����ʼʶ��
	bool MoveForward();
private:
	bool Push(SHM_ACT_DATA *pDest);		//���������ݰ����ӵ�������ݻ���
	bool Append(SHM_ACT_DATA *pDest, SHM_ACT_DATA *pSrc);
	bool Status_0();		//��������
	bool Status_1();		//����յ���
	bool Mk_slpw();		//����������
	bool SampEnCal();//��������������
	float SampEn_Br(float *Data, int m, float r, int N);
private:

	int Ecount = 0;	//��⵽���ܻ�ζ˵�Ļ��Ϳ�ʼ�Լ�һ
};

