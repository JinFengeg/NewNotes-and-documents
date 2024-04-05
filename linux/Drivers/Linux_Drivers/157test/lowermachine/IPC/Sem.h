#pragma once


int Sem_creat_GLB();//创建全局变量的信号量
int Sem_creat_ADC();//ADC数据一级缓存信号量
int Sem_creat_IMU(); //数据一级缓存信号量。
int Sem_creat_FOOT();//足底压力信号量
int Sem_creat_DATA(); //整个数据包结构体信号量。
int Sem_creat_ACT();
int Sem_creat_TCP();
int Sem_creat_TIMER();
bool Sem_P(int semid);//信号量P操作
void Sem_V(int semid);//信号量V操作
void Sem_set(int semid);//信号量全部置为1；
void Sem_remove(int semid);
