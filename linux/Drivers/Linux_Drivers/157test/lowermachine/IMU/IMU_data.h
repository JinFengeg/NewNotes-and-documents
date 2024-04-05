#pragma once
#include <pthread.h> 
#include <fstream>
#include <iostream>
using namespace std;
extern FILE* IMU_OF_fd;

int init_IMU(const char path[],int,int);
void IMU_getdata(int ch_num,int imu_fd,int base,float data[8][12]);

void* thread_imu(void *);

struct IMU_CFG{
    int imu_fd;
    int ch;/*IMU总数*/
    float data[8][12];//处理的数据
    float data_original[8][12];//原始数据
    float last_data[8][12];//上一次的数据
    int flag;/*是否退出线程*/
    int base;/*表示第几个imu开始*/
    int count;/*突变数据次数*/
    bool firstflag;/*首轮数据标志位*/
    bool changeflag;/*数据突变标志位*/
    ofstream imu_file;/*两个数据流由线程输入*/
    ofstream imu_original_file;
    pthread_mutex_t mptr;/*线程互斥锁*/
    pthread_mutex_t Bptr;
    pthread_mutex_t Eptr;
    
    
};