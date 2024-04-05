#pragma once

struct ACT_ADC_DATA
{
	float Data[1000];
};

struct ACT_IMU_DATA
{
	float ACC_x[100];
	float ACC_y[100];
	float ACC_z[100];
	float GYRO_x[100];
	float GYRO_y[100];
	float GYRO_z[100];
	float Q_0[100];
	float Q_1[100];
	float Q_2[100];
	float Q_3[100];
	float Pitch[100];
	float Yaw[100];
};

//用来将数据发送给rcg
struct SHM_ACT_DATA
{
	int Tnow;
	int WS_SP;		//标识本次活动段是从第多少次计算样本熵开始的
	int DATLen;
	int ADCLen;
	int IMULen;
	ACT_ADC_DATA ADC_Data_CH[8];
	ACT_IMU_DATA IMU_Data_CH[8];
};


void main_dsp();