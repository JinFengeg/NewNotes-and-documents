#������
CC     = gcc
#�����ļ�����Ŀ¼
INCLUDE = .
#����ѡ��
#CFLAGS = -O -Wall
CFLAGS = -g -Wall
#Object�ļ�
OBJ    = 	main.o	\
			global.o\
			Sem.o\
			shm.o\
			main_data.o\
			data_WIN.o\
			main_dsp.o\
			BP_DATA.o\
			main_tcp.o\
			tcp_TCPClass.o\
			data_ADC_main.o\
			ADC_OL.o\
			filt_ADC.o\
			data_IMU_main.o\
			IMU_data.o\
			dsp_ACT.o\
			dsp_RCG_main.o\
			dsp_RCGclass.o\
			BPNet.o\
			data_FOOT_main.o\
			class_Dense.o\
			CAN_class.o\
			main_can.cpp

#			SLPWClass.o\
#			act.o\
#			ACTClass.o\
#			dsp.o\
#			DSPClass.o\
#			BPClass.o\
#			tcp.o\
#			TCPClass.o\
#			filter.o\
#			kalman.o\
#			sampen.o

#-include <filename>
APP_tst:$(OBJ)
	@echo Linking......
	@$(CC) -Wall -o APP_tst $(OBJ) -lwiringPi -lm -lpthread -lstdc++
	@echo Complete !!!


%.o:%.cpp
	@echo Compiling $^ .......
	@$(CC) -Wall -c $^ -lm -lpthread -lstdc++ -lwiringPi -lstdc++
	@echo OK!!!
	

	
.PHONY:cleanall cleanobj cleandiff clean
clean:
	rm -rf APP *.o
cleanall:cleanobj cleandiff
	@echo Cleaning all......
	@-rm APP_tst
	@echo Complete!!!
cleanobj:
	@echo Cleaning Obj......
	@-rm *.o
	@echo Complete!!!
cleandiff:
	@echo Cleaning Diff......
	@-rm *.diff
	@echo Complete!!!
