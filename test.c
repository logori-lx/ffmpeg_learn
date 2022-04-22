#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <time.h>
#include <arpa/inet.h>

#include <wiringPi.h>
#include <softPwm.h>

#define BUFSIZE 512

//pin 18 控制速度
int PWMA = 1;
//pin 22 如为真控制左边轮子向前转
int AIN2 = 2;
//pin 27 如为真控制左边轮子向后转
int AIN1 = 3;

//pin 23 控制速度
int PWMB = 4;
//pin 25 如为真控制右边轮子向前转
int BIN2 = 5;
//pin 24  如为真控制右边轮子向后转
int BIN1 = 6;

int speed = 50;

void  t_up(unsigned int speed,unsigned int t_time)
{
	 digitalWrite(AIN2,0);
	 digitalWrite(AIN1,1);
	 softPwmWrite(PWMA,speed);
	 
	 digitalWrite(BIN2,0);
	 digitalWrite(BIN1,1);
	 softPwmWrite(PWMB,speed);
	 delay(t_time);
}

void t_stop(unsigned int t_time)
{
	 digitalWrite(AIN2,0);
	 digitalWrite(AIN1,0);
	 softPwmWrite(PWMA,0);
	 
	 digitalWrite(BIN2,0);
	 digitalWrite(BIN1,0);
	 softPwmWrite(PWMB,0);
	 delay(t_time);	
}

void t_down(unsigned int speed,unsigned int t_time)
{
	 digitalWrite(AIN2,1);
	 digitalWrite(AIN1,0);
	 softPwmWrite(PWMA,speed);
	 
	 digitalWrite(BIN2,1);
	 digitalWrite(BIN1,0);
	 softPwmWrite(PWMB,speed);
	 delay(t_time);	
}

void t_left(unsigned int speed,unsigned int t_time)
{
	 digitalWrite(AIN2,1);
	 digitalWrite(AIN1,0);
	 softPwmWrite(PWMA,speed);
	 
	 digitalWrite(BIN2,0);
	 digitalWrite(BIN1,1);
	 softPwmWrite(PWMB,speed);
	 delay(t_time);	
}

void t_right(unsigned int speed,unsigned int t_time)
{
	 digitalWrite(AIN2,0);
	 digitalWrite(AIN1,1);
	 softPwmWrite(PWMA,speed);
	 
	 digitalWrite(BIN2,1);
	 digitalWrite(BIN1,0);
	 softPwmWrite(PWMB,speed);
	 delay(t_time);	
}

void right_90(){
    
    digitalWrite(AIN2,0);
    digitalWrite(AIN1,1);
    softPwmWrite(PWMA,speed);
    
    digitalWrite(BIN2,1);
    digitalWrite(BIN1,0);
    softPwmWrite(PWMB,speed);
    delay(1430);	
}
void left_90(){
    
    digitalWrite(AIN2,1);
    digitalWrite(AIN1,0);
    softPwmWrite(PWMA,speed);
    
    digitalWrite(BIN2,0);
    digitalWrite(BIN1,1);
    softPwmWrite(PWMB,speed);
    delay(1430);	
}

void left_180(){
	digitalWrite(AIN2,1);
    digitalWrite(AIN1,0);
    softPwmWrite(PWMA,speed);
    
    digitalWrite(BIN2,0);
    digitalWrite(BIN1,1);
    softPwmWrite(PWMB,speed);
    delay(2860);
}

void right_180(){
    
    digitalWrite(AIN2,0);
    digitalWrite(AIN1,1);
    softPwmWrite(PWMA,speed);
    
    digitalWrite(BIN2,1);
    digitalWrite(BIN1,0);
    softPwmWrite(PWMB,speed);
    delay(2950);	
}

void go_to_right_one(){
	t_up(50,2000);
	right_90();
	t_up(50,1500);
}

void go_to_right_one_back(){
	right_180();
	t_up(50,1500);
	left_90();
	t_up(50,2000);
	right_180();
}
void go_to_left_one(){
	t_up(50,2000);
	left_90();
	t_up(50,1500);
}
void go_to_left_one_back(){
	left_180();
	t_up(50,1500);
	right_90();
	t_up(50,2000);
	left_180();
}
int main(int argc, char *argv[])
{
      /*RPI*/
    wiringPiSetup();
    /*WiringPi GPIO*/
    pinMode (1, OUTPUT);	//PWMA
    pinMode (2, OUTPUT);	//AIN2
    pinMode (3, OUTPUT);	//AIN1
	
    pinMode (4, OUTPUT);	//PWMB
    pinMode (5, OUTPUT);	//BIN2
	pinMode (6, OUTPUT);    //BIN1
	
	/*PWM output*/
    softPwmCreate(PWMA,0,100);//
	softPwmCreate(PWMB,0,100);
    
    //t_up(50,2000);
    //t_down(50,2000);
    //t_left(50,20000);
    //t_right(50,2000);
	go_to_left_one();
	go_to_left_one_back();
    t_stop(2000);
}