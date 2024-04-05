#pragma once

#include"tensorflow/lite/examples/LowerMachine/class_Dense.h"



extern int layer[4];
extern Dense_layer* Dense_1;
extern Dense_layer* Dense_2;
extern Dense_layer* Dense_output;
extern Batch_Norm* BN_1;

void BP_DATA();
void BP_delete();