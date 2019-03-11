#include "http_parse.h"
#include <signal.h>
/*
统一事件源：把对于信号的处理，与其他io事件一样处理
解决方法就是：将信号的主要处理逻辑放在程序的主循环中，当信号处理程序触发时，
信号处理程序网管道中的写入信号值，主循环则从管道中读出信号值（主循环利用io多路复用来监听管道读端的文件描述符的可读事件）
*/
#define MAX_EVENTS_NUMBER 1024
static int pipefd[2];

int setnonblock(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void addfd(int epollfd,int fd){ //这个函数就是将fd加到epollfd所指向的内核事件表中
    struct epoll_event epoll_e;
    epoll_e.events=EPOLLIN | EPOLLET;
    epoll_e.data.fd=fd;
    int err= epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&epoll_e);
    if(err==-1){
        printf("epoll_ctl error\n");
    }
    setnonblock(fd);//每个设置为ET都是将fd设置为非阻塞
}

void sig_handler(int sig){//信号处理函数
    int save_errno=errno;//保证程序的可重入性，保留errno
    int msg=sig;
    send(pipefd[1],(char *)msg,1,0);
    errno=save_errno;
}

void addsig(int sig){//注册信号
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_flags |= SA_RESTART;//这个表示如果信号处理函数中断了系统调用，函数返回的时候会重启系统调用
    sa.sa_handler=sig_handler;
    sigfillset(&sa.sa_mask);//在掩码中设置了所有信号
    assert(sigaction(sig,&sa,NULL)!=-1);
}


int main(int agrc,char *argv[]){
    int listenfd;
    struct sockaddr_in addr;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,argv[1],&addr.sin_addr);
    addr.sin_port=htons(atoi(argv[2]));
    int err=bind(listenfd,(sockaddr *)&addr,sizeof(addr));
    assert(err!=-1);
    err=listen(listenfd,5);
    assert(err!=-1);
    struct epoll_event events[MAX_EVENTS_NUMBER];
    int epollfd=epoll_create(MAX_EVENTS_NUMBER);
    addfd(epollfd,listenfd);

    //使用socketpair创建管道,pipefd[1]是管道写端，pipefd[0]是管道读端
    err=socketpair(PF_UNIX,SOCK_STREAM,0,pipefd);
    setnonblock(pipefd[1]);//把写置为非阻塞
    addfd(epollfd,pipefd[0]);//把pipefd[0]加入内核事件表监听

    addsig(SIGHUP);  //控制终端挂起
    addsig(SIGCHLD); //子进程状态变化
    addsig(SIGTERM); //kill命令默认发送的信号，也是终止进程
    addsig(SIGINT);  //键盘输入ctrl+C中断进程
    bool stop_server=false;
    while(!stop_server){
        int number=epoll_wait(epollfd,events,MAX_EVENTS_NUMBER,-1);
        printf("comin");
        if((number<0) && (errno !=EINTR)){
            printf("epoll error\n");
            break;
        }
        for(int i=0;i<number;i++){
            if(events[i].data.fd==listenfd){
                //这个还是表示有新的客户端接入
                struct sockaddr_in client_address;
                socklen_t client_addr_len=sizeof(client_address);
                int connfd=accept(listenfd,(sockaddr *)&client_address,&client_addr_len);
                assert(connfd!=-1);
                printf("a new tcpclient connect\n");
                addfd(epollfd,connfd); 
            }
            else if((events[i].data.fd==pipefd[0]) && (events[i].events & EPOLLIN)){
                //表示管道读这一端有事件触发，且事件为可读
                int sig;
                char signals[1024];
                int ret=recv(pipefd[0],signals,sizeof(signals),0);//ret正常时返回接收到了多少字节
                if(ret==-1){
                    continue;//表示接受完了
                }            
                else if(ret==0){
                    continue;
                }    
                else{
                    //因为每个信号占一个字节，所以按照字节逐个接收信号
                    for(int j=0;j<ret;j++){
                        switch(signals[j])
                        {
                            case SIGCHLD:
                            case SIGHUP:
                                continue;
                            case SIGTERM:
                            case SIGINT:
                                stop_server=true;
                        }
                    }
                }
                
            }
            else
            {
                printf("something else\n");
            }
        }
    }
    printf("close fds\n");
    close(listenfd);
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}