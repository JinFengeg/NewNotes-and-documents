#pragma once


/*****************************************************************
*				���ݲɼ����̡���ADC���ݲɼ�
*
*****************************************************************/

typedef struct {
	bool nflag;		//true:ո�µ�����(flag��Ҫ����������ģʽʹ��)
	float Data_src_CH[8];
	float Data_filt_CH[8];
}SHM_ADC_t;


void data_ADC_main();