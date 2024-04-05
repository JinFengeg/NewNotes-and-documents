#include "tensorflow/lite/examples/LowerMachine/tcp_TCPClass.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <math.h>
#include <pthread.h>
#define TCP_MAX_Listen 5

tcp_TCPClass::tcp_TCPClass()
{
	
}


tcp_TCPClass::~tcp_TCPClass()
{
	//fclose(print_off);
	close(TCP_fd);
}

//��ʼ��������TCP�������,ֱ�����ڿͻ�����������������Ӻ���з���
int tcp_TCPClass::init()
{
	/*	1.�������˽��н���sockfd������	*/
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{	// AF_INET:IPV4	SOCK_STREAM:TCP	
		fprintf(stderr, "Socket error:%s\n\a", strerror(errno));
		return -1;
	}
	/*	2.����������� sockaddr �ṹ	*/
	bzero(&server_addr, sizeof(sockaddr_in_t));		//��ʼ�� ��0
	server_addr.sin_family = AF_INET;		//Internet IPV4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//(���������ϵ�long����ת��Ϊ�����ϵ�long����)�������������������κ�ip��������  //INADDR_ANY ��ʾ��������������IP��ַ����������������԰󶨵����е�IP��
														//server_addr.sin_addr.s_addr=inet_addr("192.168.1.1");  //���ڰ󶨵�һ���̶�IP,inet_addr���ڰ����ּӸ�ʽ��ipת��Ϊ����ip
	server_addr.sin_port = htons(TCP_port);         // (���������ϵ�short����ת��Ϊ�����ϵ�short����)�˿ں�

	int reuse = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("setsockopet error\n");
		return -2;
	}
	/* 
	if (bind(socket_fd, (sockaddr_t *)(&server_addr), sizeof(sockaddr_t)) == -1)
	{
		fprintf(stderr, "Bind error:%s\n\a", strerror(errno));
		return -3;
	}
	/* 4.�����������ӵ����ͻ����� */
	if (listen(socket_fd, TCP_MAX_Listen) == -1) {
		fprintf(stderr, "Listen error:%s\n\a", strerror(errno));
		return -4;
	}



	socklen_t sin_size = sizeof(sockaddr_in_t);
	printf("TCP Waiting for connect ... ...\n");


	if ((TCP_fd = accept(socket_fd, (sockaddr_t *)(&client_addr), &sin_size)) == -1)
	{
		fprintf(stderr, "Accept error:%s\n\a", strerror(errno));
		return -5;
	}


	printf("TCP Server get connection from %s\n", inet_ntoa(client_addr.sin_addr));
	c_flag = true;		
	return 0;
}

//��������
int tcp_TCPClass::send(char src[],int coun)
{
	int ret;
	ret = write(TCP_fd, src, coun);
	//fprintf(print_off, "%s\n", src);
	if (ret < 0) 
	{
		fprintf(stderr, "Write TCP Error:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}
