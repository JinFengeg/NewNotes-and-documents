#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include"tensorflow/lite/examples/LowerMachine/class_Dense.h"


Dense_layer* Dense_1;
Dense_layer* Dense_2;


Dense_layer* Dense_output;
Batch_Norm* BN_1;



int layer[4];

static void read_dense_weights_bias(std::ifstream &in,Dense_layer* dense);
static void BN_read_weights(std::ifstream& in, Batch_Norm* BN);

void BP_DATA()
{
	using namespace std;
	ifstream weights_in;
	weights_in.open("./weights.csv");
	if(!weights_in.is_open())
	{
		cout<<"load weights error";
		exit(0);
	}
	string buff;
	string temp;
	//读取网络节点数
	// if(getline(weights_in,buff))
	// {
	// 	istringstream str_in;
	// 	str_in.str(buff);
	// 	int i=0;
	// 	while(getline(str_in, temp, ','))
	// 	{
	// 		layer[i] = atoi(temp.c_str());
	// 		i++;
	// 	}
	// 	//分配参数空间
	// 	BN_1 = new Batch_Norm(layer[0]);
	// 	Dense_1 = new Dense_layer(layer[0],layer[1]);
	// 	Dense_2 = new Dense_layer(layer[1],layer[2]);
	// 	Dense_output = new Dense_layer(layer[2],layer[3]);
				
	// }
	// else
	// {
	// 	cout<<"load weights error";
	// 	exit(0);
	// }
	layer[0] = 40;
	layer[1] = 30;
	layer[2] = 15;
	layer[3] = 6;
	BN_1 = new Batch_Norm(layer[0]);
	Dense_1 = new Dense_layer(layer[0],layer[1]);
	Dense_2 = new Dense_layer(layer[1],layer[2]);
	Dense_output = new Dense_layer(layer[2],layer[3]);
	BN_read_weights(weights_in,BN_1);
	read_dense_weights_bias(weights_in,Dense_1);
	read_dense_weights_bias(weights_in,Dense_2);
	read_dense_weights_bias(weights_in,Dense_output);

	weights_in.close();
	printf("read BP data ok!!!");
}
static void read_dense_weights_bias(std::ifstream &in,Dense_layer* dense)
{
	using namespace std;
	string buff;
	string temp;
	int nodes = dense->output_nodes;
	//读取weights
	for(int i=0;i<nodes;i++)
	{
		getline(in,buff);
		istringstream str_in;
		str_in.str(buff);
		int j=0;
		// getline(str_in, temp, ',');//跳过说明 C_w1
		while (getline(str_in, temp, ','))
		{
			istringstream temp_in;
			temp_in.str(temp);
			float mean;
			temp_in >> mean;
			dense->weights[i][j] = mean;
			j++;
		}

	}
	//读取bias
	if(getline(in,buff))
	{
		istringstream str_in;
		str_in.str(buff);
		int i = 0;
		// getline(str_in, temp, ',');//跳过说明c_b1
		while (getline(str_in, temp, ','))
		{
			dense->bias[i] = stof(temp);
			i++;
			temp.clear();
		}
		buff.clear();
	}
}
static void BN_read_weights(std::ifstream& in, Batch_Norm* BN)
{
	using namespace std;
	string buff;
	string temp;
	if(getline(in,buff))
	{
		istringstream str_in;
		str_in.str(buff);
		int i = 0;
		// getline(str_in, temp, ',');//跳过说明moving_mean
		while (getline(str_in, temp, ','))
		{
			BN->moving_mean[i] = stof(temp);
			i++;
			temp.clear();
		}
		buff.clear();
	}
	//读取var
	if(getline(in,buff))
	{
		istringstream str_in;
		str_in.str(buff);
		int i = 0;
		// getline(str_in, temp, ',');//跳过说明moving_mean
		while (getline(str_in, temp, ','))
		{
			BN->moving_var[i] = stof(temp);
			i++;
			temp.clear();
		}
		buff.clear();
	}
	//读取gama
	if(getline(in,buff))
	{
		istringstream str_in;
		str_in.str(buff);
		int i = 0;
		// getline(str_in, temp, ',');//跳过说明moving_mean
		while (getline(str_in, temp, ','))
		{
			
			BN->moving_gamma[i] = stof(temp);
			i++;
			temp.clear();
		}
		buff.clear();
	}
	//读取beta
	if(getline(in,buff))
	{
		istringstream str_in;
		str_in.str(buff);
		int i = 0;
		// getline(str_in, temp, ',');//跳过说明moving_mean
		while (getline(str_in, temp, ','))
		{
			BN->moving_beta[i] = stof(temp);
			i++;
			temp.clear();
		}
		buff.clear();
	}
}
void BP_delete()
{
	delete Dense_1;
	delete Dense_2;
	delete Dense_output;
	delete BN_1;
}