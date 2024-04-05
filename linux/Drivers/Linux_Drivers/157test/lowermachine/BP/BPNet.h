
/*******************************************************
*		BP������ǰ�򴫲��㷨
********************************************************/
#pragma once



class BPNet
{
public:
	BPNet();
	~BPNet();
public:
	void Init(int BP_layer[]);	//��ʼ������
	bool BPForward(float *pVec);			//BP������ǰ�򴫲��㷨
public:
	float BP_ret[6];					//BP����������
	int max_index;
private:

	void activation_tanh(float* output, float* intput, int nodes);
	void activation_softmax(float* output, float* intput, int nodes);
private:
	bool BP_Init_flag = false;			//�����ǲ��ǳ�ʼ����

	
	float *BN_1_out;
	float *dense_1_out, *dense_2_out,*dense_3_out;


};

