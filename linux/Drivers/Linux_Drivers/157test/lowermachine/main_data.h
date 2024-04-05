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
����   ����  ��������
��ǰ��
���
��ǰ��
�Һ�
*/
struct FT_Data
{
	int FT_L_P[256];	//��ǰ
	int FT_L_H[256];	//���
	int FT_R_P[256];	//��ǰ
	int FT_R_H[256];	//�Һ�
};


//ͬ��֮������ݽṹ��8ͨ��ADC���ݺ�8ͨ��IMU���ݡ�
struct SHM_DATA_t
{
	int Tnow;
	int DAT_LEN;
	int ADC_LEN;
	int IMU_LEN;
	int FT_LEN;
	ADC_Data ADC_DATA_CH[8]; //ÿ��ͨ��256���㣬 8��ͨ��
	IMU_Data IMU_DATA_CH[8]; //ÿ��ͨ��50���㡣
	FT_Data FT_DATA_CH;
};


void main_data();
