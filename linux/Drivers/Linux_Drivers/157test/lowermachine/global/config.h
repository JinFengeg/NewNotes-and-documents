#pragma once

// #define ADC_MODE_OL		//ADC������������
// #define IMU_MODE_OL		//IMU���߲ɼ�
// #define FOOT_MODE_OL

#define IMU_CH_L 3
#define IMU_CH_R 3
#define IMU_CH 7
#define sEMG_CH_1 0
#define sEMG_CH_2 1
#define sEMG_CH_3 2
#define sEMG_CH_4 6
#define sEMG_CH_5 7

#define ANS


#define _OFL_DTZ_DATA_		//�趨 ������������


#define OFL_ACT				//�趨ACT���ߴ���
#define ADC_OF_DATA			//��ADC���ݱ����xls��ʽ
#define IMU_OF_DATA			//��IMU���ݱ����xls��ʽ
#define FT_OF_DATA          //��FOOT���ݱ����xls��ʽ

#define ADC_FILE_NAME		"file_ADC.csv"
#define IMU_FILE_NAME		"file_IMU.csv"
#define FT_FILE_NAME		"file_FOOT.csv"



#define DATW_LEN			40		//���ݰ�����
#define SLPW_LEN			64		//����������


#define ACT_LIM_S		0.001				//��ζ˵��⣬�����ֵ
#define ACT_LS			5					//�������⣬����
#define ACT_LIM_E		0.0001				//��ζ˵��⣬�յ���ֵ
#define ACT_LE			5					//����յ��⣬����
#define ACT_LEN			200

#define SAMPEN_m			2		//ʸ��ά��
#define SAMPEN_r			0.02

