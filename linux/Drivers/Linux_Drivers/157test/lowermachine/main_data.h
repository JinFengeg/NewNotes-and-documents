#pragma once

struct ADC_Data
{
	float Data[256];
};

struct IMU_Data
{
	float ACC_x[50];
	float ACC_y[50];
	float ACC_z[50];
	float GYRO_x[50];
	float GYRO_y[50];
	float GYRO_z[50];
	float Q_0[50];
	float Q_1[50];
	float Q_2[50];
	float Q_3[50];
	float Pitch[50];
	float Yaw[50];
};

/*4*256
数据   数据  。。。。
左前：
左后：
右前：
右后：
*/
struct FT_Data
{
	int FT_L_P[256];	//左前
	int FT_L_H[256];	//左后
	int FT_R_P[256];	//右前
	int FT_R_H[256];	//右后
};


//同步之后的数据结构。8通道ADC数据和8通道IMU数据。
struct SHM_DATA_t
{
	int Tnow;
	int DAT_LEN;
	int ADC_LEN;
	int IMU_LEN;
	int FT_LEN;
	ADC_Data ADC_DATA_CH[8]; //每个通道256个点， 8个通道
	IMU_Data IMU_DATA_CH[8]; //每个通道50个点。
	FT_Data FT_DATA_CH;
};


void main_data();
