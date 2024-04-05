#ifndef class_Dense_
#define class_Dense_

#include <iostream>


class Dense_layer
{
    public:
        Dense_layer(int input,int output);
        ~Dense_layer();
        // bool update_weights_bias(float** weights, float *bias);
        void BP_forword(float* output,float* input);//W*x+b
    public:
        int input_nodes;
        int output_nodes;
        float** weights;
        float *bias;


};

class Batch_Norm
{
    public:
        Batch_Norm(int input);
        ~Batch_Norm();
        void BatchNormalization(float* output,float *input);//(x-mean)/sart(var+eplison)*gamma+beta
    public:
        int nodes;
        float* moving_var;
        float* moving_mean;
        float* moving_beta;
        float* moving_gamma;
};


#endif