#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>     
#include <pthread.h>               
#define BUFFSIZE 128

#define PORT 8000
int sockfd;                                // socket描述字
void snd();                                // 发送消息的函数

int main(int argc, char** argv){
    if(argc != 2){
        puts("please input the IP address!\n");
        exit(-1);
    }
    pthread_t thread;                      // 创建线程
    struct sockaddr_in serv_addr;          // 连接的服务端ip地址
    char buf[BUFFSIZE];                    // 初始化服务端地址结构
    bzero(&serv_addr, sizeof(serv_addr));  // 将结构体清零
    serv_addr.sin_family = AF_INET;        // 协议类型
    serv_addr.sin_port = htons(PORT);      // 端口
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
                                           // 将hostIP转换成网络字节序表示的ip地址
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
                                           // socket 创建套接字
    if(sockfd < 0){
        perror("socket创建失败！");
        exit(-1);
    }
    // 与服务端建立连接
    printf("正在连接服务端... \n");
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("连接失败！");
        exit(-2);
    }
    // 创建发送消息的线程，调用发送消息的函数snd
    pthread_create(&thread, NULL, (void *)(&snd), NULL); 
    // 接收消息的线程
    while(1){
        int len;
        if((len=read(sockfd, buf, BUFFSIZE)) > 0){ // read 读取通信套接字
            buf[len] = '\0';                       // 添加结束符，避免显示缓冲区中残留的内容
            printf("\n%s", buf);
            fflush(stdout);                        // fflush 清空标准输出缓冲区
        }
    }
    return 0;
}

// 发送消息的函数
void snd(){
    char name[32], buf[BUFFSIZE];
    fgets(name, 32, stdin);                       
    write(sockfd, name, strlen(name));              // write 写入通信套接字
    while(1){
        fgets(buf, BUFFSIZE, stdin);
        write(sockfd, buf, strlen(buf));
        if(strcmp(buf, "Bye\n") == 0)              // 读取出缓冲区字符是'再见\n'则退出
            exit(0);
    }
}