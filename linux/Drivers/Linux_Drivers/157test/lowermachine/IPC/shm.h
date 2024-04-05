#pragma once




int Shm_creat_glb(int size);
int Shm_creat_adc(int size);
int Shm_creat_imu(int size);
int Shm_creat_data(int size);
int Shm_creat_ACT(int size);
int Shm_creat_TCP(int size);
int Shm_creat_TIMER(int size);
int Shm_creat_foot(int size);
void *Shm_mat(int shmid_t);
void Shm_remove(int semid, int shmid, void* shm_p);

