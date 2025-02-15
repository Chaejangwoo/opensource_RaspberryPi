#include <signal.h> //Signal 사용 헤더파일
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h> //exit() 사용 헤더파일
#include <wiringPi.h>
#define PUMP 21 // BCM_GPIO 5
void sig_handler(int signo); // 마지막 종료 함수
int main (void)
{
signal(SIGINT, (void *)sig_handler); //시그널 핸들러 함수
if (wiringPiSetup () == -1)
{
fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
return 1 ;
}
pinMode (PUMP, OUTPUT) ;

printf("here - pump on\n");
digitalWrite (PUMP, 1) ; // On
delay (1000) ; // ms
digitalWrite (PUMP, 0) ; // Off

return 0 ;
}
void sig_handler(int signo) //시스널 핸들러 함수 구현 //GPIO 핀 OFF 상태로 한다.
{
 printf("process stop\n");
digitalWrite (PUMP, 0) ; // Off
exit(0);
}

