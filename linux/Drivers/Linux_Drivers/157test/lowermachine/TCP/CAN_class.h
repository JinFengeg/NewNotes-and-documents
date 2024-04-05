#ifndef CAN_class_
#define CAN_class_

#include <wiringPi.h>
#include <wiringSerial.h>
#define CAN_PORT "/dev/ttyUSB"
#define CAN_Baud 115200
struct CAN_RES
{
    char label;
    float time;
    long int seconds;
    long int usecond;
};

class CAN_class
{
    public:
        CAN_class();
        int serial_fd;
        int init();
        void send_data(char[],int);
    private:
        bool init_flag;
        
};


#endif
