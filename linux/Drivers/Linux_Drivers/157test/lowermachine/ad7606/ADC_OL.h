/************************************************************
*			ADC采样底层控制
***************************************************************/
#pragma once
#include <stdint.h>

#define RST 			0		//IO引脚定义
#define CVAB 			2
#define CS 				3
#define BUSY  			4
#define RANGE			25
#define MISO			13
#define SCLK			14
#define OS_0			22
#define OS_1			23
#define OS_2			24
#define ADC_SPICLK 		5000000		// SPI总线通讯速率
#define SPI_MODE_0		0u   //CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
#define SPI_MODE_1		1u   //CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
#define SPI_MODE_2		2u   //CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
#define SPI_MODE_3		3u   //CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
#define SPI_CHANNEL_0	0u
#define SPI_CHANNEL_1	1u

//#define ADC_CH_NUM			8



class ADC_OL
{
public:
	ADC_OL();
	~ADC_OL();
public:
	bool init();	//初始化操作，指定当前有port_num个通道来进行AD采样
	bool getData(float *pData);	//获取数据
//public:
//	float Data[8];
private:
	void setSpiSpeed(unsigned int speed);	//SPI总线初始化操作
	void setGPIO();				//相关IO引脚功能配置
	void setOS(uint8_t _ucMode);	//过采样工作模式设定
	void setRange(int _ucRange);	//设定输入范围模式
	void reset();				//ADC复位操作
	void setIOtoCVT();			//进行IO设置，通知AD模块进行AD转换
	float compute(unsigned char HH, unsigned char LL);	//进行AD采样后结果计算，获得AD采样的电压值
	bool ofset(float *pData);	//为原始数据添加偏置
private:
	int MAXRange = 5;
	unsigned char ADCbuf[16];
};

