#pragma once


#include "tensorflow/lite/examples/LowerMachine/main_data.h"


class data_WIN
{
public:
	bool Init();	//��ʼ��
	bool Clear();				//������ݻ�����
	bool Enable();				//��������
	bool DataCpy(SHM_DATA_t *pDest, SHM_DATA_t *pSrc);	//�����ݸ��Ƶ������ڴ�
	SHM_DATA_t DATW;
	bool Append_ADC(float Data[8]);	//����һ������
	bool Append_IMU(float Data[8][12]);	//����һ������
};