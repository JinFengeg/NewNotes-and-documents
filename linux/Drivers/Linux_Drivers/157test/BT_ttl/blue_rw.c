#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
    struct sockaddr_rc addr = { 0 };
    int s, status, len=0;
    char dest[18] = "00:0C:BF:03:61:3C";
    unsigned char buf[33];
    float data[11] = {0};
    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    
    if(status){
        printf(" failed to connect the device!\n");
        return -1;
    }else{
        printf("success to connect the device!\n");
    }
    while(1)
    {
len = read(s, buf, 33);
    
  //数据转换
//加速度
		data[0] = (float)((short)((buf[3] << 8) | buf[2])) / 32768.0 * 16.0;  //xx g  (g=9.8m/s2)
		data[1] = (float)((short)((buf[5] << 8) | buf[4])) / 32768.0 * 16.0;  //xx g  (g=9.8m/s2)
		data[2] = (float)((short)((buf[7] << 8) | buf[6])) / 32768.0 * 16.0;  //xx g  (g=9.8m/s2)																		   
		//温度
		data[3] = (float)(((short)((buf[9] << 8) | buf[8]))  /32768 * 96.38 + 36.53);
        //角速度
		data[4] = (float)((short)((buf[14] << 8) | buf[13])) / 32768 * 2000;
		data[5] = (float)((short)((buf[16] << 8) | buf[15])) / 32768 * 2000;
		data[6] = (float)((short)((buf[18] << 8) | buf[17])) / 32768 * 2000;
      //电压
      data[7] = (float)((short)((buf[20] << 8) | buf[19])) / 100;
        //角度
		data[8] = (float)((short)((buf[25] << 8) | buf[24])) / 32768 * 180;
		data[9] = (float)((short)((buf[27] << 8) | buf[26])) / 32768 * 180;
		data[10] = (float)((short)((buf[29] << 8) | buf[28])) / 32768 * 180;


 /*printf("[ ");
 for (n = 0; n < ret; n++)
 printf("0x%hhx ", buf[n]);
 printf("]\n");*/


	printf("第1个IMU:加x=%f,加y=%f,加z=%f,\
温度=%f,角x=%f,角y=%f,角z=%f,电压=%f,\
角度x=%f,角度y =%f,角度z=%f\r\n",\
	data[0],data[1],data[2],data[3],data[4],data[5]\
	,data[6],data[7],data[8],data[9],data[10]);
	
    }

    close(s);
    return 0;
}