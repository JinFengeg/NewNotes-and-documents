#pragma once
#include <pthread.h> 
#include <fstream>
#include <iostream>
using namespace std;
extern FILE* IMU_OF_fd;

int init_IMU(const char path[]);
void IMU_getdata(int ch_num,int imu_fd,int base,float data[8][12]);
void* thread_imu(void *);
struct IMU_CFG{
    int imu_fd;
    int ch;
    float data[8][12];
    int flag;
    int base;
    ofstream imu_file; 
    pthread_mutex_t mptr;
      
};