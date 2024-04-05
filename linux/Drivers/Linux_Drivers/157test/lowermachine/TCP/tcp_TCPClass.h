/******************************************************
*			TCP �����
*
*********************************************************/
#pragma once
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tensorflow/lite/examples/LowerMachine/config.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"

#define TCP_port	3333
typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;

typedef struct {		//	TCP ���ݽṹ	
	char Data[100];
} TCP_Data_t;

class tcp_TCPClass
{
public:
	tcp_TCPClass();
	~tcp_TCPClass();
public:
	int init();	//��ʼ��
	int send(char *src,int coun);	//��������
	bool c_flag = false;	//true:	�ѳɹ�����TCP����
private:
	int socket_fd;	//�豸��ʶ��
	int TCP_fd;
	//FILE* print_off;
	bool s_flag = false;	//true:	����TCP���ݷ���
	sockaddr_in_t server_addr, client_addr;
};

//extern bool TCP_bkup(TCP_Data_t *dest, TCP_Data_t *src);
