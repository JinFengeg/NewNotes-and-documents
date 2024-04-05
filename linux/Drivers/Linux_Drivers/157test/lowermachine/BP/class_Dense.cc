#include "tensorflow/lite/examples/LowerMachine/class_Dense.h"
#include <math.h>
Dense_layer::Dense_layer(int input,int output)
{
    input_nodes = input;
    output_nodes = output;
    weights = new float* [output_nodes];
    for(int i=0;i<output_nodes;++i)
    {
        weights[i] = new float[input_nodes];
    }
    bias = new float[output_nodes];

}
Dense_layer::~Dense_layer()
{
    delete[] weights;
    delete[] bias;
}

void Dense_layer::BP_forword(float *output, float* input)
{
    for(int i=0;i<output_nodes;++i)
    {
        for(int j=0;j<input_nodes;++j)
        {
            output[i] += input[j]*weights[i][j];
        }
        output[i] += bias[i];
    }
}

Batch_Norm::Batch_Norm(int input)
{
    nodes = input;
    moving_mean = new float[nodes];
    moving_var = new float[nodes];
    moving_beta = new float[nodes];
    moving_gamma = new float[nodes];
    
}

Batch_Norm::~Batch_Norm()
{
	delete[] moving_var;
	delete[] moving_mean;
	delete[] moving_beta;
	delete[] moving_gamma;
}

void Batch_Norm::BatchNormalization(float* output,float *input)
{
    for(int i=0;i<nodes;++i)
    {
        output[i] = (input[i]-moving_mean[i])/sqrt(moving_var[i]+1e-3)*moving_gamma[i]+moving_beta[i];
    }
}