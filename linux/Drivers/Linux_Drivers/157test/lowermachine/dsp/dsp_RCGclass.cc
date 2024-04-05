#include <math.h>
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "tensorflow/lite/examples/LowerMachine/config.h"
#include "tensorflow/lite/examples/LowerMachine/dsp_RCGClass.h"
#include "tensorflow/lite/examples/LowerMachine/BP_DATA.h"

#include <fstream>
#include <iostream>

#include <memory>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <fcntl.h>      // NOLINT(build/include_order)
#include <getopt.h>     // NOLINT(build/include_order)
#include <sys/time.h>   // NOLINT(build/include_order)
#include <sys/types.h>  // NOLINT(build/include_order)
#include <sys/uio.h>    // NOLINT(build/include_order)
#include <unistd.h>     // NOLINT(build/include_order)


//#include "MyresizeInput.h"

using namespace std;
#ifdef ANS
std::ofstream BP_off;
#endif

#define TFLITE_MINIMAL_CHECK(x)                              \
  if (!(x)) {                                                \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
    exit(1);                                                 \
  }


//找出序列中最大值
int dsp_RCGClass::softmax(float* arr, unsigned len)
{
    float tmp = arr[0];
    int ans = 0;
    for(unsigned i=0;i<len;i++){
        if(arr[i]>tmp){
            ans = i;
            tmp = arr[i];
        }
    }
    return ans;
}

dsp_RCGClass::dsp_RCGClass()
{
	BP = new(BPNet);

	
	//走 跳 上楼梯 下楼梯 上坡 下坡
	const char* modelpath[7] = {"./model/actionmodel.tflite","./model/uphillmodel.tflite","./model/downhillmodel.tflite","./model/upstairsmodel.tflite","./model/downstairsmodel.tflite","./model/walkmodel.tflite","./model/jumpmodel.tflite"};
	for(int i=0;i<7;++i){
		//加载模型
		std::unique_ptr<tflite::FlatBufferModel> actionmodel = tflite::FlatBufferModel::BuildFromFile(modelpath[i]);
		TFLITE_MINIMAL_CHECK(actionmodel != nullptr);
		//建立解释器， 加载的模型可以被多个解释器使用
		tflite::ops::builtin::BuiltinOpResolver resolver;
		tflite::InterpreterBuilder builder(*actionmodel, resolver);
		std::unique_ptr<tflite::Interpreter> tempinterpreter;
		builder(&tempinterpreter);
		TFLITE_MINIMAL_CHECK(tempinterpreter != nullptr);
		//多线程
		// const int num_threads = 4;
		// if(num_threads != 1){
		// 	interpreter->SetNumThreads(num_threads);
		// }
		TFLITE_MINIMAL_CHECK(tempinterpreter->AllocateTensors() == kTfLiteOk);
		model.push_back(std::move(actionmodel));
		interpreter.push_back(std::move(tempinterpreter));
	}

}

dsp_RCGClass::~dsp_RCGClass()
{
	delete BP;
}
//��ʼ�� 初始化
bool dsp_RCGClass::Init()
{
	BP->Init(layer);	//��ʼ��BP������ 初始化BP神经网络
	#ifdef ANS
	BP_off.open("./A_BP.csv");
    if(!BP_off.is_open())
    {
        cout<<"ERROR: open BP_off file\n"<<endl;  
    }
	#endif

	return true;
}
//���ݿ���数据拷贝
bool dsp_RCGClass::BackUp(SHM_ACT_DATA *pSrc)
{
	ACTD.Tnow = pSrc->Tnow;
	ACTD.WS_SP = pSrc->WS_SP;
	ACTD.ADCLen = pSrc->ADCLen;
	ACTD.IMULen = pSrc->IMULen;
	ACTD.DATLen = pSrc->DATLen;
	for (int _CH = 0; _CH < 8; _CH++) 
	{
		//ADC
		for (int n = 0; n < ACTD.ADCLen; n++) 
		{
			ACTD.ADC_Data_CH[_CH].Data[n] = pSrc->ADC_Data_CH[_CH].Data[n];
		}
		//IMU
		for (int n = 0; n < ACTD.IMULen; n++) 
		{
			ACTD.IMU_Data_CH[_CH].ACC_x[n] = pSrc->IMU_Data_CH[_CH].ACC_x[n];
			ACTD.IMU_Data_CH[_CH].ACC_y[n] = pSrc->IMU_Data_CH[_CH].ACC_y[n];
			ACTD.IMU_Data_CH[_CH].ACC_z[n] = pSrc->IMU_Data_CH[_CH].ACC_z[n];
			ACTD.IMU_Data_CH[_CH].GYRO_x[n] = pSrc->IMU_Data_CH[_CH].GYRO_x[n];
			ACTD.IMU_Data_CH[_CH].GYRO_y[n] = pSrc->IMU_Data_CH[_CH].GYRO_y[n];
			ACTD.IMU_Data_CH[_CH].GYRO_z[n] = pSrc->IMU_Data_CH[_CH].GYRO_z[n];
			ACTD.IMU_Data_CH[_CH].Q_0[n] = pSrc->IMU_Data_CH[_CH].Q_0[n];
			ACTD.IMU_Data_CH[_CH].Q_1[n] = pSrc->IMU_Data_CH[_CH].Q_1[n];
			ACTD.IMU_Data_CH[_CH].Q_2[n] = pSrc->IMU_Data_CH[_CH].Q_2[n];
			ACTD.IMU_Data_CH[_CH].Q_3[n] = pSrc->IMU_Data_CH[_CH].Q_3[n];
			ACTD.IMU_Data_CH[_CH].Pitch[n] = pSrc->IMU_Data_CH[_CH].Pitch[n];
			ACTD.IMU_Data_CH[_CH].Yaw[n] = pSrc->IMU_Data_CH[_CH].Yaw[n];

		}
	}


	return false;
}
//重组
bool dsp_RCGClass::TZ_calc()
{
	//reshape EMG
	for(int i=0;i<200;++i){
		for(int ch=0;ch<8;++ch){
			in_emg[i/10][i%10][ch] = ACTD.ADC_Data_CH[ch].Data[i];
		}
	}
	//reshape IMU
	for(int i=0;i<20;++i){
		for(int ch=0;ch<7;++ch){
			float ax = ACTD.IMU_Data_CH[ch].ACC_x[i];
			float ay = ACTD.IMU_Data_CH[ch].ACC_y[i];
			float az = ACTD.IMU_Data_CH[ch].ACC_z[i];
			in_imu[i][0][ch] = ax;
			in_imu[i][1][ch] = ay;
			in_imu[i][2][ch] = az;
			in_imu[i][3][ch] = ACTD.IMU_Data_CH[ch].GYRO_x[i];
			in_imu[i][4][ch] = ACTD.IMU_Data_CH[ch].GYRO_y[i];
			in_imu[i][5][ch] = ACTD.IMU_Data_CH[ch].GYRO_z[i];
			in_imu[i][6][ch] = ACTD.IMU_Data_CH[ch].Q_3[i];
			in_imu[i][7][ch] = ACTD.IMU_Data_CH[ch].Pitch[i];
			in_imu[i][8][ch] = ACTD.IMU_Data_CH[ch].Yaw[i];
			in_imu[i][9][ch] = ax*ax+ay*ay+az*az;
		}
	}
	VEC.VECLen = 40;
	return true;
}
bool dsp_RCGClass::Recg()
{
	TZ_calc();
	/*
	//步态行为
	const char* actionmodelpath = "./model/actionmodel.tflite";
	//走 跳 上楼梯 下楼梯 上坡 下坡
	const char* gaitmodelpath[6] = {"./model/uphillmodel.tflite","./model/downhillmodel.tflite","./model/upstairsmodel.tflite","./model/downstairsmodel.tflite","./model/walkmodel.tflite","./model/jumpmodel.tflite"};
//====================================================步态行为识别=========================================================
//加载模型
	std::unique_ptr<tflite::FlatBufferModel> actionmodel = tflite::FlatBufferModel::BuildFromFile(actionmodelpath);
    TFLITE_MINIMAL_CHECK(actionmodel != nullptr);
//建立解释器， 加载的模型可以被多个解释器使用
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*actionmodel, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    builder(&interpreter);
    TFLITE_MINIMAL_CHECK(interpreter != nullptr);
//多线程
	const int num_threads = 4;
    if(num_threads != 1){
        interpreter->SetNumThreads(num_threads);
    }
	TFLITE_MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
	*/

//推理
	std::vector<int> inputs_indexarray = interpreter[0]->inputs();  //输入张量的索引数组
	float* input_EMG = interpreter[0]->typed_tensor<float>(inputs_indexarray[0]);//填充数据即可
    float* input_IMU = interpreter[0]->typed_tensor<float>(inputs_indexarray[1]);
	int inputemgindex = 0;
	int inputimuindex = 0;
	for(int row=0;row<20;++row){
		for(int col = 0;col<10;++col){
			for(int ch=0;ch<8;++ch){
				input_EMG[inputemgindex] = (float)in_emg[row][col][ch];
				++inputemgindex;
			}
			for(int ch=0;ch<7;++ch){
				input_IMU[inputimuindex] = (float)in_imu[row][col][ch];
				++inputimuindex;
			}
		}
	}
	if (interpreter[0]->Invoke() != kTfLiteOk){
	std::cout<<"Failed to invoke tflite!"<<std::endl;
	exit(-1);
	}
	std::vector<int> output_indexarray = interpreter[0]->outputs();
	float* modeloutput = interpreter[0]->typed_tensor<float>(output_indexarray[0]);
	int U = softmax(modeloutput, 6);

	/*
	//===========================================步态相位识别================================================================
	//加载模型
	std::unique_ptr<tflite::FlatBufferModel> gaitmodel = tflite::FlatBufferModel::BuildFromFile(gaitmodelpath[U]);
    TFLITE_MINIMAL_CHECK(gaitmodel != nullptr);
//建立解释器， 加载的模型可以被多个解释器使用
    tflite::ops::builtin::BuiltinOpResolver gaitresolver;
    tflite::InterpreterBuilder gaitbuilder(*gaitmodel, gaitresolver);
    std::unique_ptr<tflite::Interpreter> gaitinterpreter;
    gaitbuilder(&gaitinterpreter);
    TFLITE_MINIMAL_CHECK(gaitinterpreter != nullptr);
//多线程
	const int gaitnum_threads = 4;
    if(gaitnum_threads != 1){
        interpreter->SetNumThreads(gaitnum_threads);
    }
	TFLITE_MINIMAL_CHECK(gaitinterpreter->AllocateTensors() == kTfLiteOk);
	*/

//推理
	std::vector<int> gaitinputs_indexarray = interpreter[U+1]->inputs();  //输入张量的索引数组
	float* gaitinput_EMG = interpreter[U+1]->typed_tensor<float>(inputs_indexarray[0]);//填充数据即可
    float* gaitinput_IMU = interpreter[U+1]->typed_tensor<float>(inputs_indexarray[1]);
	int gaitinputemgindex = 0;
	int gaitinputimuindex = 0;
	for(int row=0;row<20;++row){
		for(int col = 0;col<10;++col){
			for(int ch=0;ch<8;++ch){
				gaitinput_EMG[gaitinputemgindex] = (float)in_emg[row][col][ch];
				++gaitinputemgindex;
			}
			for(int ch=0;ch<7;++ch){
				gaitinput_IMU[gaitinputimuindex] = (float)in_imu[row][col][ch];
				++gaitinputimuindex;
			}
		}
	}
	if (interpreter[U+1]->Invoke() != kTfLiteOk){
	std::cout<<"Failed to invoke tflite!"<<std::endl;
	exit(-1);
	}
	std::vector<int> gaitoutput_indexarray = interpreter[U+1]->outputs();
	float* gaitmodeloutput = interpreter[U+1]->typed_tensor<float>(gaitoutput_indexarray[0]);
	int V = softmax(gaitmodeloutput, 5);

	actionU = U;
	gaitV = V;
	
	return true;
}

/*****************************************************************/
//
float dsp_RCGClass::calc_WAV(float *IN, int len)
{
	float tmp = 0;
	for (int n = 0; n < len; n++)
	{
		tmp += abs(IN[n]);
	}
	return (tmp / len);
}

//���γ���
float dsp_RCGClass::calc_WL(float *IN, int len)
{
	float tmp = 0;
	for (int n = 0; n < len - 1; n++) {
		tmp += abs(IN[n] - IN[n + 1]);
	}
	return tmp;
}

//��ֵ
float dsp_RCGClass::calc_AV(float *IN, int len)
{
	float tmp = 0;
	for (int n = 0; n < len; n++) {
		tmp += IN[n];
	}
	return (tmp / len);
}

//����
float dsp_RCGClass::calc_IAR(float *IN, int len)
{
	float tmp = 0;
	for (int n = 0; n < len; n++) {
		tmp += pow(IN[n], 2);
	}
	tmp /= (len - 1);
	return (tmp / (len - 1));
}

//������
float dsp_RCGClass::calc_RMS(float *IN, int len)
{
	float tmp = 0;
	for (int n = 0; n < len; n++) {
		tmp += pow(IN[n], 2);
	}
	return sqrt(tmp / (len - 1));
}

//������
float dsp_RCGClass::calc_SPEN(float *IN, int len)
{
	float tmp = 0;

	return tmp;
}

float  dsp_RCGClass::calc_Euler(float *IN, int x, int y) 
{
	float result;
	result = IN[x] - IN[y];
	return result;
}

float dsp_RCGClass::calc_ACC(float*x, float*y, float*z, int i)
{
	float result;
	result = sqrt(x[i] * x[i] + y[i] * y[i] + z[i] * z[i]);
	return result;
}
