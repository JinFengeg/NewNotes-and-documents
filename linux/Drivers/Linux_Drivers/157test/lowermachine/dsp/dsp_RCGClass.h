#pragma once

/*****************************************************
*		������ݴ�����ؼ������
*****************************************************/
#pragma once
#include <stdio.h>
#include "tensorflow/lite/examples/LowerMachine/main_dsp.h"
#include "tensorflow/lite/examples/LowerMachine/BPNet.h"
#include "tensorflow/lite/examples/LowerMachine/config.h"
#include <fstream>
#include <string>
#include <vector>
#include <memory>
// tflite的头文件
#include "absl/memory/memory.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/optional_debug_tools.h"

#ifdef ANS
extern std::ofstream BP_off;
#endif


struct TZ_vec_t 
{
	int VECLen;
	float Data[50];
};

class dsp_RCGClass
{
public:
	dsp_RCGClass();
	~dsp_RCGClass();
public:
	bool Init();	//��ʼ��
	bool BackUp(SHM_ACT_DATA *pSrc);	//���ݿ���
	bool Recg();	//������ݴ���
	int softmax(float* arr, unsigned len);
public:
	float RCG_ret[6];
	int Max_Index;

	int actionU;
	int gaitV;
	SHM_ACT_DATA ACTD;	//���������Ļ������

	std::vector<std::unique_ptr<tflite::FlatBufferModel>> model;
	std::vector<std::unique_ptr<tflite::Interpreter>> interpreter;




private:
	bool TZ_calc();	//���������ļ���
	float calc_WAV(float *IN, int len);
	float calc_WL(float *IN, int len);
	float calc_AV(float *IN, int len);
	float calc_IAR(float *IN, int len);
	float calc_RMS(float *IN, int len);
	float calc_SPEN(float *IN, int len);
	float calc_Euler(float *IN, int x, int y);
	float calc_ACC(float*x, float*y, float*z, int i);
	float in_emg[20][10][8];
	float in_imu[20][10][7];
public:
	//tensorflow lite

private:
	// dsp_FFTClass FFT;
	TZ_vec_t VEC;
	BPNet *BP;
};


