#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <vector>
#include <iostream>
#include "tensorflow/lite/examples/LowerMachine/config.h"
#include "tensorflow/lite/examples/LowerMachine/global.h"
#include "tensorflow/lite/examples/LowerMachine/ADC_OL.h"


//	閿熸枻鎷烽敓鏂ゆ嫹閿燂拷 閿熸枻鎷烽敓灞婂嚱閿熸枻鎷�
ADC_OL::ADC_OL()
{
	//	ADCbuf = (unsigned char *)malloc(sizeof(unsigned char)*ADC_CH_NUM * 2);	//閿熸枻鎷烽敓鏂ゆ嫹鍗犳磳顒婃嫹閿熸枻鎷烽敓鏂ゆ嫹濞差敡D杞敓鏂ゆ嫹涔嬮敓鏂ゆ嫹閿熸枻鎷风焊閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
}
//閿熸枻鎷烽敓鏂ゆ嫹閿燂拷 閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
ADC_OL::~ADC_OL()
{
	//	free(ADCbuf);
	//	free(Data);
}
//	ADC妯￠敓浠嬪伐閿熸枻鎷烽敓鏂ゆ嫹濮嬮敓鏂ゆ嫹
bool ADC_OL::init()
{
	wiringPiSetup();		//閿熸枻鎷疯帗閿熸枻鎷稩O閿熸枻鎷烽敓鏂ゆ嫹妯″紡閿熸枻鎷烽敓鏂ゆ嫹
	setSpiSpeed(ADC_SPICLK);	// 閿熸枻鎷烽敓鏂ゆ嫹AD7606閿熸枻鎷稴PI閿熸枻鎷烽敓鏂ゆ嫹閫氳
	setGPIO();					//閿熸枻鎷烽敓绲€O閿熸枻鎷烽敓鑴氱櫢鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
	setOS(0);					//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷锋ā寮忛敓鍊熷畾,default:OS2 OS1 OS0 = 000
	setRange(0);				//閿熷€熷畾閿熸枻鎷烽敓璇寖鍥存ā寮�,default:range = -5 ~ +5    1: 閿熸枻鎷�10
	reset();					//ADC閿熸枻鎷蜂綅閿熸枻鎷烽敓鏂ゆ嫹
	setIOtoCVT();
	return true;
}
// 閿熷€熷畾SPI閿熸枻鎷烽敓鏂ゆ嫹閫氳閿熷姭璁规嫹
void ADC_OL::setSpiSpeed(unsigned int speed = 5000000)
{
	int fd;
	if ((fd = wiringPiSPISetupMode(SPI_CHANNEL_0, speed, SPI_MODE_2)) < 0)
	{
		fprintf(stderr, "Can't open the SPI bus: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}
//	閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓绱綪IO閿熸枻鎷烽敓鏂ゆ嫹 
void ADC_OL::setGPIO()
{
	pinMode(RST, OUTPUT);
	pinMode(RANGE, OUTPUT);
	pinMode(CVAB, OUTPUT);
	pinMode(CS, OUTPUT);
	pinMode(BUSY, INPUT);
	pinMode(OS_0, OUTPUT);
	pinMode(OS_1, OUTPUT);
	pinMode(OS_2, OUTPUT);

	pullUpDnControl(BUSY, PUD_UP);
	digitalWrite(RANGE, LOW);
	digitalWrite(RST, LOW);
	digitalWrite(CS, HIGH);
	digitalWrite(CVAB, HIGH);
	digitalWrite(OS_0, LOW);
	digitalWrite(OS_1, LOW);
	digitalWrite(OS_2, LOW);
}
//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷锋ā寮忛敓鍊熷畾
void ADC_OL::setOS(uint8_t _ucMode = 0)//default:OS2 OS1 OS0 = 000
{
	switch (_ucMode) {			//			閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
	case 0:		// NO OS					1
		digitalWrite(OS_0, LOW);
		digitalWrite(OS_1, LOW);
		digitalWrite(OS_2, LOW);
		break;
	case 1: 	// 						2
		digitalWrite(OS_0, HIGH);
		digitalWrite(OS_1, LOW);
		digitalWrite(OS_2, LOW);
		break;
	case 2: 	//						4
		digitalWrite(OS_0, LOW);
		digitalWrite(OS_1, HIGH);
		digitalWrite(OS_2, LOW);
		break;
	case 3: 	// 						8
		digitalWrite(OS_0, HIGH);
		digitalWrite(OS_1, HIGH);
		digitalWrite(OS_2, LOW);
		break;
	case 4: 	//						16
		digitalWrite(OS_0, LOW);
		digitalWrite(OS_1, LOW);
		digitalWrite(OS_2, HIGH);
		break;
	case 5: 	//						32
		digitalWrite(OS_0, HIGH);
		digitalWrite(OS_1, LOW);
		digitalWrite(OS_2, HIGH);
		break;
	case 6: 	//						64
		digitalWrite(OS_0, LOW);
		digitalWrite(OS_1, HIGH);
		digitalWrite(OS_2, HIGH);
		break;
	default: 	//						閿熸枻鎷锋晥		
		break;
	}
}

//閿熷€熷畾閿熸枻鎷烽敓璇寖鍥存ā寮�	0:閿熸枻鎷�5V 1:閿熸枻鎷�10V
void ADC_OL::setRange(int _ucRange = 0)
{
	switch (_ucRange) {
	case 0:
		digitalWrite(RANGE, LOW);		//閿熸枻鎷�5V
		MAXRange = 5;
		break;
	case 1:
		digitalWrite(RANGE, HIGH);		//閿熸枻鎷�10V
		MAXRange = 10;
		break;
	default:
		break;
	}
}

//ADC閿熸枻鎷烽敓鍙潻鎷蜂綅閿熸枻鎷烽敓鏂ゆ嫹
void ADC_OL::reset()
{
	digitalWrite(CS, HIGH);
	digitalWrite(CVAB, HIGH);         /* AD7606 is high level reset閿熸枻鎷穉t least 50ns */
	digitalWrite(RST, LOW);
	//delayMicroseconds(1);        
	digitalWrite(RST, HIGH);
	//delayMicroseconds(1);        
	digitalWrite(RST, LOW);
}

//閿熸枻鎷烽敓鏂ゆ嫹IO閿熸枻鎷烽敓鐭綇鎷烽€氱煡AD妯￠敓鏂ゆ嫹閿熸枻鎷烽敓绱窪杞敓鏂ゆ嫹
void ADC_OL::setIOtoCVT()
{
	/* Conv in rising edge at least 25ns  */
	digitalWrite(CVAB, LOW);
	delayMicroseconds(1);
	digitalWrite(CVAB, HIGH);
}


//	閿熸枻鎷烽敓鐭紮鎷峰彇涓€甯ч敓閾扮鎷烽敓鏂ゆ嫹閿熸嵎锝忔嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷蜂竴閿熸枻鎷稟DC閿熸枻鎷烽敓鏂ゆ嫹杞敓鏂ゆ嫹
bool ADC_OL::getData(float *pData)
{
	int ret;
	if (digitalRead(BUSY) == 0) {
		digitalWrite(CS, LOW);
		/*	閿熸枻鎷烽敓鏂ゆ嫹閿熺璁规嫹鍙栭敓鏂ゆ嫹閿熸枻鎷�		*/
		ret = wiringPiSPIDataRW(SPI_CHANNEL_0, ADCbuf, 16);
		if (ret == -1) {
			printf("ONLine: Sample failure! < %s >\n", strerror(errno));
			return false;
		}
		digitalWrite(CS, HIGH);
		setIOtoCVT();
		while (digitalRead(BUSY) == 1);		//閿熼ズ杈炬嫹閿熸枻鎷烽敓鏂ゆ嫹AD杞敓鏂ゆ嫹閿熸枻鎷烽敓锟�
		for (int _CH = 0; _CH < 8; _CH++)
			pData[_CH] = compute(ADCbuf[_CH * 2], ADCbuf[_CH * 2 + 1]);
		ofset(pData);
		return true;
	}
	else
		return false;
}

//閿熸枻鎷烽敓鏂ゆ嫹AD閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓濮愶紝閿熸枻鎷烽敓绱窪閿熸枻鎷烽敓鏂ゆ嫹閿熶茎纰夋嫹鍘嬪€�
float ADC_OL::compute(unsigned char HH, unsigned char LL)
{
	unsigned int tmp;
	float disp;
	tmp = (HH << 8) + LL;
	disp = 1.0 * tmp *(MAXRange * 2) / 65535;
	if (disp >= MAXRange)
	{
		disp = MAXRange * 2 - disp;
		return disp * (-1);
	}
	else
		return disp;
}

bool ADC_OL::ofset(float *pData)
{
	static float ADC_OFSET[8] = { 0.016, 0.017, 0.018, 0.016, 0.022, 0.028, 0.013, 0.019 };
	for (int n = 0; n < 8; n++)
		pData[n] -= ADC_OFSET[n];
	return true;
}
