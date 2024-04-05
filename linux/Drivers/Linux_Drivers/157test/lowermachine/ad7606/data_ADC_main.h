#pragma once


/*****************************************************************
*				数据采集进程——ADC数据采集
*
*****************************************************************/

typedef struct {
	bool nflag;		//true:崭新的数据(flag主要是用于离线模式使用)
	float Data_src_CH[8];
	float Data_filt_CH[8];
}SHM_ADC_t;


void data_ADC_main();