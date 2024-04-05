#pragma once


#define ROW 8
#define COLUMN 12
#define IMU_PORT_NAME		"/dev/ttyUSB0"

//imu�����ڴ����ݽṹ��
struct SHM_IMU_t 
{
	bool nflag;
	float Data_src[8][12];
	float Data_src_original[8][12];
	//float Data_filt[8][12];
};



void data_IMU_main();
