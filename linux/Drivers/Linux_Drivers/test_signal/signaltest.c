#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

void sigint_handler(int num)
{
    printf("\r\n SIGINT signal!\r\n");
    exit(0);
}
int main(void){
    signal(SIGINT,sigint_handler);
    while(1);
    return 0;
}