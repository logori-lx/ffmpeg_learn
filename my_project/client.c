#include<sys/types.h>      //像AF_INET这些参数，如果函数用到了的话都需要加这个头文件
#include<sys/socket.h>     //用到的函数socket，如果函数需要传递sockfd参数，则也需要用到这个头文件
#include<stdlib.h>         //用到的函数calloc,函数atoi
#include<stdio.h>
#include<signal.h>         //sigaction函数及sigaction结构体以及其他宏定义
#include<string.h>         //strlen、strcpy、bzero
#include<unistd.h>         //write、read、close
#include<netinet/in.h>     //存放 struct sockaddr_in
#include<arpa/inet.h>      //inet_pton函数
#include<stdbool.h>        //用于定义bool类型变量
#include<errno.h>          //用于变量errno

#include"car_control.h"
#include"dealdata.h"

///@brief 定义在main函数中，标识服务器的运行状态，当该状态位为真，所有线程都应该停止并退出
bool stop_srv;
///@brief 定义在main函数中，标识如果产生中断时，引起中断的类型
int sig_type;




int car_simple_control(char *message){
    if(strstr(message, "up")){
        t_up();
        t_stop();
    }else if(strstr(message, "down")){
        t_down();
        t_stop();
    }else if(strstr(message, "stop")){
        t_stop();
    }else if(strstr(message, "left")){
        left_90();
        t_stop();
    }else if(strstr(message, "right")){
        right_90();
        t_stop();
    }
    return 0;
}

/**
 * @brief 用于处理SIGINT信号的信号处理函数
 * @param  signo  信号变量，用于设置sig_type，判断信号的类型
 */
void sig_int(int signo)
{
    printf("[srv] SIGINT is comming!\n");
    stop_srv = true;
    sig_type = signo;
    return;
}
/**
 * @brief 用于处理SIGPIPE信号的信号处理函数
 * @param  signo  信号变量，用于设置sig_type，判断信号的类型          
 */
void sig_pipe(int signo)
{
    sig_type = signo;
    int num;
    printf("[srv] SIGPIPE is coming!\n");
}





int main(int argc, char* argv[]){
    struct sigaction sigact_int, old_sigact_int, sigact_pipe, old_sigact_pipe;
    int clientfd, numbytes;
    char buf[MAX_CMD_STR];
    struct sockaddr_in server_addr;
    /*
    if(argc != 3){
      printf("usage: %s <server IP address> <server port>",argv[0]);
      exit(0);
    }
    */
    if((clientfd = socket(AF_INET, SOCK_STREAM,0)) == -1){
      perror("Socket failed!\n");
      exit(-1);
    }
    int on = 1;
    if((setsockopt(clientfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}
    
    
     //设置信号处理函数为sig_pipe这个函数
    sigact_pipe.sa_handler = sig_pipe;
    //将sigint的信号屏蔽字清空
    sigemptyset(&sigact_int.sa_mask);
    sigact_pipe.sa_flags = 0;
    //使被信号打断的系统调用能够自动重新开始
    sigact_pipe.sa_flags |= SA_RESTART;
    sigaction(SIGPIPE,&sigact_pipe,&old_sigact_pipe);

    sigact_int.sa_handler = sig_int;
    sigemptyset(&sigact_int.sa_mask);
    sigact_int.sa_flags = 0;
    sigaction(SIGINT,&sigact_int, &old_sigact_int);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    if(inet_pton(AF_INET, "42.194.193.254", &server_addr.sin_addr) == -1){
       perror("Server IP Address Error!\n");
       exit(-1);
    }

    server_addr.sin_port = htons(atoi("7211"));
    
    if(connect(clientfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
      perror("Connect failed!\n");
      exit(-1);
    }
    printf("Connect success! The server's address is: %s, port is %d\n",\
        inet_ntop(AF_INET,&server_addr.sin_addr,buf,sizeof(struct sockaddr)),\
        ntohs(server_addr.sin_port));
    writeData(clientfd,"car");
    char *messag = readData(clientfd,buf);
    printf("received from server: %s\n",messag);
    car_init();
    while(!stop_srv)
	{
        char *message = readData(clientfd, buf);
        if(buf == NULL)
        {
            printf("[car] server disconnected!\n");
            close(clientfd);
            return;
        }
        
        printf("received from server: %s\n",message);
        if(strstr(message,"C")){
            car_simple_control(message);
        }

	}
    close(clientfd);
}
