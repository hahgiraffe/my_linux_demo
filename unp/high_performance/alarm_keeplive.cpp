#include "list_timer.h"
#include "http_parse.h"
#include <signal.h>
/*
处理非活动连接
也可以通过socket选项的KEEPALIVE设置，但是对于程序管理连接较为复杂
本程序在应用层实现类似KEEPALIVE机制，以管理所有长时间处于非活动状态的连接
主要流程：利用alarm函数周期性的触发SIGALARM信号，信号处理函数利用管道通知主循环执行定时器链表上的定时任务，即关闭非活动连接
*/

#define FD_LIMIT 65535
#define MAX_EVENTS_NUMBER 1024
#define TIMESLOT 5
static int pipefd[2];
static sort_time_list timer_list;
static int epollfd=0;

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

//回调函数,删除非活动连接socket上的注册事件，并关闭他
void cb_func(client_data *user_data){
    
    epoll_ctl(epollfd,EPOLL_CTL_DEL,user_data->sockfd,0);
    close(user_data->sockfd);
    printf("close fd %d\n",user_data->sockfd);
    return;
}

void time_handler(){
    timer_list.tick();
    alarm(TIMESLOT);//一次alarm只能触发一次SIGALRM，所以需要重新定时
}

int main(int argc,char *argv[]){
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
    
    //设置信号处理
    addsig(SIGALRM);
    addsig(SIGTERM);
    bool stop_server=false;
    bool timeout=false;
    alarm(TIMESLOT);
    client_data *users=new client_data[FD_LIMIT];//用一个数组存储每个客户端的数据

    while(!stop_server){
        int number=epoll_wait(epollfd,events,MAX_EVENTS_NUMBER,-1);
        if( (number<0) && (errno!=EINTR) ){
            printf("epoll fail\n");
            break;
        }
        for(int i=0;i<number;i++){
            int sockfd=events[i].data.fd;
            if(sockfd==listenfd){
                //新客户端请求
                struct sockaddr_in client_address;
                socklen_t client_addr_len=sizeof(client_address);
                int connfd=accept(listenfd,(sockaddr *)&client_address,&client_addr_len);
                assert(connfd!=-1);
                printf("a new tcpclient connect\n");
                addfd(epollfd,connfd); 
                users[connfd].address=client_address;
                users[connfd].sockfd=connfd;
                //接下来创建定时器，设置回调函数与超时时间
                util_timer *timer=new util_timer;
                timer->user_data=&users[connfd];
                timer->cb_func=cb_func;
                time_t cur=time(NULL);
                timer->expire=cur+3*TIMESLOT;
                users[connfd].timer=timer;
                timer_list.add_timer(timer);
            }
            else if( (sockfd==pipefd[0]) && (events[i].events & EPOLLIN)){
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
                            case SIGALRM:
                                timeout=true;//timeout标记有定时任务需要处理
                                break;
                            case SIGTERM:
                                stop_server=true;
                        }
                    }
                }
            }
            else if(events[i].events & EPOLLIN){
                    //这里是处理客户连接上收到的数据
                    memset(users[sockfd].buf,'\0',BUFF_SEIZE);
                    int num2=recv(sockfd,users[sockfd].buf,BUFF_SEIZE-1,0);//最大可以取BUFFSIZE-1，最后一个'\0'表好似字符串结束
                    printf("get %d byte of content : %s from %d\n",num2,users[sockfd].buf,sockfd);
                    util_timer *timer=users[sockfd].timer;
                    if(num2<0){
                        //如果读完
                        if(errno==EAGAIN || errno==EWOULDBLOCK){//ET设置为非阻塞io，这个if条件成立表示数据已经全部读取完毕，
                                                                //epoll就可以再次触发events[i].data.fd上的EPOLLIN，驱动下一次读操作
                            //printf("read later\n");
                            break;
                        }
                        //如果发生错误，则关闭连接，移除响应定时器
                        if(errno != EAGAIN){
                            cb_func(&users[sockfd]);
                            if(timer){
                                timer_list.del_timer(timer);
                            }
                        }
                        
                    }
                    else if(num2==0){

                        cb_func(&users[sockfd]);
                        if(timer){
                            timer_list.del_timer(timer);
                        }
                    }
                    else{
                        //如果某个客户连接有数据可读，则我们需要调整该连接对应的定时器
                        if(timer){
                            time_t cur=time(NULL);
                            timer->expire=cur+3*TIMESLOT;
                            printf("adjust timer once\n");
                            timer_list.adjust_timer(timer);
                        }
                        
                    }
            }
            else
            {
                
            }
            
        }

        //处理定时事件
        if(timeout){
            time_handler();
            timeout=false;
        }
    }

    close(listenfd);
    close(pipefd[0]);
    close(pipefd[1]);
    delete [] users;
    return 0;

}