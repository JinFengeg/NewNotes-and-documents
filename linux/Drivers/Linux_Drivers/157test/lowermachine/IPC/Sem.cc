#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "tensorflow/lite/examples/LowerMachine/Sem.h"


union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

//全锟街憋拷锟斤拷锟斤拷锟脚猴拷锟斤拷semid
int Sem_creat_GLB()
{
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid == -1)
	{
		perror("semget GLB error\n");
	}
	//printf("semid_glbs: %d\n", semid);
	return semid;
}

//ADC锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷锟脚猴拷锟斤拷
int Sem_creat_ADC()
{
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid == -1)
	{
		perror("semget ADC error\n");
	}
	//printf("semid_adcs: %d\n", semid);
	return semid;
}

//IMU锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷锟脚猴拷锟斤拷
int Sem_creat_IMU()
{
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid == -1)
	{
		perror("semget IMU error\n");
	}
	//printf("semid_imus: %d\n", semid);
	return semid;
}


int Sem_creat_FOOT()
{
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid == -1)
	{
		perror("semget IMU error\n");
	}
	return semid;
}


//
int Sem_creat_DATA()
{
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid == -1)
	{
		perror("semget DATA error\n");
	}
	//printf("semid_DATA: %d\n", semid);
	return semid;
}

int Sem_creat_TIMER()
{
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid == -1)
	{
		perror("semget TIMER error\n");
	}
	//printf("semid_DATA: %d\n", semid);
	return semid;
}

int Sem_creat_ACT()
{
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid == -1)
	{
		perror("semget ACT error\n");
	}
	//printf("semid_ACT: %d\n", semid);
	return semid;
}


int Sem_creat_TCP()
{
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (semid == -1)
	{
		perror("semget TCP error\n");
	}
	//printf("semid_TCP: %d\n", semid);
	return semid;
}


//锟脚猴拷锟斤拷P锟斤拷锟斤拷
bool Sem_P(int semid)/*传入int类型，返回布尔类型*/
{
	//bool result = true;
	int res;
	struct sembuf sem_b;
	/*此结构体定义没找到，是内核源码中的结构体，我说怎么应用里没有讲到
	struct sembuf  
{  
  unsigned short int sem_num;    信号量的序号从0~nsems-1  
  short int sem_op;             对信号量的操作，>0, 0, <0   
  short int sem_flg;            操作标识：0， IPC_WAIT, SEM_UNDO  
}; 此结构体应该是信号量操作的结构体*/
	sem_b.sem_num = 0;/*信号在信号集中的索引，0表示第一个信号*/
	sem_b.sem_op = -1;/*信号加上sem_op的值，表示进程释放控制的资源sem_op的正负决定对信号量进行P操作，正数的话进行V操作*/
	sem_b.sem_flg = SEM_UNDO;/*操作标志，只有当sem_flg指定为SEM_UNDO标志后，semadj才会更新，这里的semadj是所指定信号量
	针对调用进程的调整值*/
	do
	{
		res = semop(semid, &sem_b, 1);/*semid为信号量的标识符，操作结构体为sem_b，要进行操作的信号个数为1
		这里的semop主要是对信号进行p/v操作的，p操作是阻塞进程*/
		if ((res == -1) && (errno == 4))/*当执行p操作失败时返回-1，并且此时的错误码是4的话执行以下操作*/
		{
			res = semctl(semid, 0, GETVAL);/*此函数对信号量进行各种操作，在semid标识的信号量集上，或者该集合的第0个
			信号量上执行GETVAL指定的控制命令这里就是返回semval的值*/
			usleep(5);
			printf("sys EINTR:res : %d\n", res);/*这里就是当对信号量执行P操作错误的时候打印下当前信号量的值*/
		}
	} while((res == -1) && (errno == 4));/*如果执行p操作成功的话，直接返回true*/

	//res = semctl(semid, 0, GETVAL);
	//if (res < 0)
	//{
	//	printf("semid:%d\tres = %d\n", semid, res);
	//}
	return true;
}
//锟脚猴拷锟斤拷V锟斤拷锟斤拷
void Sem_V(int semid)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if (semop(semid, &sem_b, 1) == -1)/*执行释放信号量操作从而唤醒被阻塞的进程 ，如果执行失败打印错误码*/
	{
		printf("semid: %d, semop_V failed锟斤拷 errno = %d\n", semid, errno);
		exit(1);
	}
}

//锟脚猴拷锟斤拷锟斤拷锟斤拷为1
void Sem_set(int semid)
{
	semun sem_union;
	sem_union.val = 1;		
	if (semctl(semid, 0, SETVAL, sem_union) == -1)
	{
		perror("Semset error!\n");
		exit(1);
	}
}
void Sem_remove(int semid)
{
	if ((semctl(semid, IPC_RMID, 0)) == -1)
	{
		perror("semctl error:");
		exit(1);
	}
}