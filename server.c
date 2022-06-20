#include <stdio.h>
#include <stdlib.h>                          // exit
#include <string.h>
#include <unistd.h>                          // bind、listen
#include <time.h>                            // time(NULL)、ctime(&ticks)
#include <netinet/in.h>                      // sockaddr_in、sockaddr
#include <arpa/inet.h>                       // inet_ntop
#include <pthread.h>
#define PORT 8000
#define MAXMEM 10                            // 套接字的长度，用于人数上限设置 
#define BUFFSIZE 128

//#define DEBUG_PRINT                        // 宏定义
#ifdef DEBUG_PRINT
#define DEBUG(format, ...) printf("FILE: "__FILE__", LINE: %d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif

int listenfd, connfd[MAXMEM];                // 定义监听套接字和连接套接字
void quit();                                 // 服务端退出函数
void rcv_snd(int p);                         // 服务端接受信息函数

int main(){
    struct sockaddr_in serv_addr, cli_addr;
    int i;                                   // 循环变量
    time_t ticks;                            // 进入时间
    pthread_t thread;                
    char buff[BUFFSIZE];                     // 存放地址
    printf("server start...\n(input quit to QUIT!)\n");
    DEBUG("=== STARTING...");               // 初始化填充服务端地址结构
    bzero(&serv_addr, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;          // 协议
    serv_addr.sin_port = htons(PORT);        // 端口
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                                             // 地址转换
    DEBUG("=== LISTENING...");                 // 创建服务端的监听套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        perror("listen fault");
        exit(-1);
    }
    DEBUG("=== 正在绑定...");                // 将套接字与填充好的地址结构进行绑定
    if(bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("绑定失败");
        exit(-2);
    }
    DEBUG("=== LISTENING...");                // 将主动连接套接字变为被动倾听套接字
    listen(listenfd, MAXMEM);
    // 创建一个线程，对服务器程序进行管理，调用quit函数
    pthread_create(&thread, NULL, (void *)(quit), NULL);
    // 将套接字描述符数组初始化为-1，表示空闲
    for(i=0; i<MAXMEM; i++)
        connfd[i] = -1;
    while(1){
	    int len;
	    for(i=0; i<MAXMEM; i++){					
            if(connfd[i] == -1)
		    break;
        }
        // 从listen接受的连接队列中取得一个连接
        connfd[i] = accept(listenfd, (struct sockaddr *)&cli_addr, &len);
	    if(connfd[i] < 0)
            perror("接收失败");					
        ticks = time(NULL);               
        printf("%.24s\n\tconnect from: %s, port %d\n",ctime(&ticks), inet_ntop(AF_INET, &(cli_addr.sin_addr), buff, BUFFSIZE),ntohs(cli_addr.sin_port));   
        // 针对当前套接字创建一个线程，对当前套接字的消息进行处理
	    pthread_create(malloc(sizeof(pthread_t)), NULL, (void *)(&rcv_snd), (void *)i);
    }										
    return 0;									
}	

void quit(){									
    char msg[10];							
    while(1){						
        scanf("%s", msg);                      
        if(strcmp(msg, "quit") == 0){					
            printf("退出... \n");
            close(listenfd);
            exit(0);							
        }						
    }
}									

void rcv_snd(int n){							
    int len, i;
    char name[32], mytime[32], buf[BUFFSIZE];			
    time_t ticks;							
    int ret;									
    // 获取此线程对应的套接字用户的名字			
    write(connfd[n], "请输入用户名: ", strlen("请输入用户名: "));
    len = read(connfd[n], name, 32);					
    if(len > 0)								
        name[len-1] = '\0';                  // 去除换行符			
    strcpy(buf, name);						
    strcat(buf, "\t加入聊天室\n\0");						
    // 把当前用户的加入告知所有用户						
    for(i=0; i<MAXMEM; i++){				
        if(connfd[i] != -1)							
            write(connfd[i], buf, strlen(buf));			
    }									
    while(1){								
        char temp[BUFFSIZE];							
        if((len=read(connfd[n], temp, BUFFSIZE)) > 0){				
            temp[len-1] = '\0';					
            // 				
            if(strcmp(temp, "Bye") == 0){		
                close(connfd[n]);				
                connfd[n] = -1;				
                pthread_exit(&ret);
            }
            ticks = time(NULL);
            sprintf(mytime, "%.24s\r\n", ctime(&ticks));		
            strcpy(buf, name);
            strcat(buf, "\t");
            strcat(buf, mytime);
            strcat(buf, "\r\t");
            strcat(buf, temp);
            strcat(buf, "\n");
            for(i=0; i<MAXMEM; i++){
                if(connfd[i] != -1)
                    write(connfd[i], buf, strlen(buf));
            }
        }
    }
}