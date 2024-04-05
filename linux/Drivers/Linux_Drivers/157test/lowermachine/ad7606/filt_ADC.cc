/******************************************************
*	ADC�����˲�
******************************************************/
#include "tensorflow/lite/examples/LowerMachine/config.h"

void ADC_filt(float *dest, float *src)
{
	// 10~150HZ bandpass parameters
    static const float A[5] = { 1.0, -2.7231, 2.8589, -1.44704, 0.3197 };
    static const float B[5] = { 0.1041, 0.0, -0.2082, 0.0, 0.1041 };
    static float temp_x[8][5] = { {0},{0},{0},{0},{0} };
    static float temp_y[8][5] = { {0},{0},{0},{0},{0} };
    //50HZ ��Ƶ�ݲ�����
    static const float A_50[3] = { 1				,-1.8527	,0.9481 };
    static const float B_50[3] = { 0.9740			,-1.8527	,0.9740 };
    static float temp_x_50[8][3] = { {0},{0},{0} };
    static float temp_y_50[8][3] = { {0},{0},{0} };

    //50hz��Ƶ�ݲ�
    for (int ch = 0; ch < 8; ch++)
    {
        for (int j = 0; j < 2; j++)
        {
            temp_x_50[ch][j] = temp_x_50[ch][j + 1];
        }
        temp_x_50[ch][2] = src[ch];
    }//����Xn

    for (int ch = 0; ch < 8; ch++)
    {
        dest[ch] = B_50[0] * temp_x_50[ch][2] + B_50[1] * temp_x_50[ch][1] + B_50[2] * temp_x_50[ch][0]\
            - A_50[1] * temp_y_50[ch][2] - A_50[2] * temp_y_50[ch][1];
    }//�õ����µ�Yn

    for (int ch = 0; ch < 8; ch++)
    {
        temp_y_50[ch][0] = temp_y_50[ch][1];
        temp_y_50[ch][1] = temp_y_50[ch][2];
        temp_y_50[ch][2] = dest[ch];
    }//����Yn

    //10~150��ͨ�˲�
    for (int ch = 0; ch < 8; ch++)
    {
        for (int i = 0; i < 4; i++)
        {
            temp_x[ch][i] = temp_x[ch][i + 1];
        }
        temp_x[ch][4] = dest[ch];
    }//����Xn

    for (int ch = 0; ch < 8; ch++)
    {
        dest[ch] = B[0] * temp_x[ch][4] + B[1] * temp_x[ch][3] + B[2] * temp_x[ch][2] + B[3] * temp_x[ch][1] + B[4] * temp_x[ch][0]\
            - A[1] * temp_y[ch][4] - A[2] * temp_y[ch][3] - A[3] * temp_y[ch][2] - A[4] * temp_y[ch][1];
    }//�õ����µ�Yn

    for (int ch = 0; ch < 8; ch++)
    {
        for (int i = 0; i < 4; i++)
        {
            temp_y[ch][i] = temp_y[ch][i + 1];
        }
        temp_y[ch][4] = dest[ch];
    }//����Yn
}

void IMU_filter(float dest[8][12], float src[8][12])
{
	static const float A[6] = { 1,-0.79820,0.85663,-0.29829,0.09124,-0.00841 };
    static const float B[6] = { 0.02634,0.13171,0.26343,0.26343,0.13171,0.02634 };
    static float Xn[8][12][6] = { 0 };
    static float Yn[8][12][6] = { 0 };
    for (int ch = 0; ch < IMU_CH; ch++)
    {
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                Xn[ch][i][j] = Xn[ch][i][j + 1];
            }
            Xn[ch][i][5] = src[ch][i];
        }
    }//����Xn
    for (int ch = 0; ch < IMU_CH; ch++)
    {
        for (int i = 0; i < 12; i++)
        {
            dest[ch][i] = B[0] * Xn[ch][i][5] + B[1] * Xn[ch][i][4] + B[2] * Xn[ch][i][3] + B[3] * Xn[ch][i][2] + B[4] * Xn[ch][i][1] + B[5] * Xn[ch][i][0]\
                - A[1] * Yn[ch][i][5] - A[2] * Yn[ch][i][4] - A[3] * Yn[ch][i][3] - A[4] * Yn[ch][i][2] - A[5] * Yn[ch][i][1];
        }
    }//��ּ����yn
    for (int ch = 0; ch < IMU_CH; ch++)
    {
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                Yn[ch][i][j] = Yn[ch][i][j + 1];
            }
            Yn[ch][i][5] = dest[ch][i];
        }
    }//����Yn
}