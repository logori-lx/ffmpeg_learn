#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <time.h>
#include <arpa/inet.h>

void t_up();
void t_stop();
void t_down();
void t_left();
void t_right();
void right_90();
void left_90();
void left_180();
void right_180();
void go_to_right_one();
void go_to_right_one_back();
void go_to_left_one();
void go_to_left_one_back();
void car_init();