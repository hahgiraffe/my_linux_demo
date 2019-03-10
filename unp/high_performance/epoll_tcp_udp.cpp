#include "http_parse.h"
/*
    此demo是利用epoll实现一个server同时对tcp和udp进行响应
    一个socket对应着一个（ip，port），如果要要求一个port同时响应tcp和udp也要多次
    bind
*/
#define MAX_EVENTS_NUMBER 1024
#define UDP_BUFFER_SIZE 1024
#define TCP_BUFFER_SIZE 1024

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

int main(int argc,char *argv[]){
    //先创建tcp套接字并绑定到端口
    int listenfd;
    struct sockaddr_in addr;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,argv[1],&addr.sin_addr);
    //inet_pton(AF_INET,argv[2],&addr.sin_port);
    addr.sin_port=htons(atoi(argv[2]));
    int err=bind(listenfd,(sockaddr *)&addr,sizeof(addr));
    assert(err!=-1);
    err=listen(listenfd,5);
    assert(err!=-1);
    
    //创建udp套接字
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,argv[1],&addr.sin_addr);
    addr.sin_port=htons(atoi(argv[2]));
    int udpfd=socket(AF_INET,SOCK_DGRAM,0);
    err=bind(udpfd,(sockaddr *)&addr,sizeof(addr));
    assert(err!=-1);

    //利用epoll创建内核事件表，
    int epollfd=epoll_create(MAX_EVENTS_NUMBER);
    addfd(epollfd,listenfd);
    addfd(epollfd,udpfd);

    //epoll_wait循环监听内核事件表
    while(1){
        struct epoll_event events[MAX_EVENTS_NUMBER];
        //printf("in\n");
        int ret=epoll_wait(epollfd,events,MAX_EVENTS_NUMBER,-1);
        if(ret<0){
            printf("epoll wait error\n");
            break;
        }

        for(int i=0;i<ret;i++){
            if(listenfd==events[i].data.fd){
                struct sockaddr_in client_address;
                socklen_t client_addr_len=sizeof(client_address);
                int connfd=accept(listenfd,(sockaddr *)&client_address,&client_addr_len);
                assert(connfd!=-1);
                //printf("a new tcpclient connect,ip%s  port%s\n",(char *)ntohl(client_address.sin_addr.s_addr),(char*)(client_address.sin_port));
                printf("a new tcpclient connect\n");
                addfd(epollfd,connfd);
            }
            else if(udpfd==events[i].data.fd){
                char buf[UDP_BUFFER_SIZE];
                memset(buf,'\0',UDP_BUFFER_SIZE);
                struct sockaddr_in udpclient_address;
                socklen_t udpclient_addr_len=sizeof(udpclient_address);
                int num=recvfrom(udpfd,buf,UDP_BUFFER_SIZE-1,0,(sockaddr*)&udpclient_address,&udpclient_addr_len);
                if(num>0){
                    sendto(udpfd,buf,UDP_BUFFER_SIZE-1,0,(sockaddr*)&udpclient_address,udpclient_addr_len);
                }
            }
            else if(events[i].events & EPOLLIN){
                char buf[TCP_BUFFER_SIZE];
                while(1){
                    memset(buf,'\0',TCP_BUFFER_SIZE);
                    int num2=recv(events[i].data.fd,buf,TCP_BUFFER_SIZE-1,0);//最大可以取BUFFSIZE-1，最后一个'\0'表好似字符串结束
                    if(num2<0){
                        //如果读完
                        if(errno==EAGAIN || errno==EWOULDBLOCK){//ET设置为非阻塞io，这个if条件成立表示数据已经全部读取完毕，
                                                                //epoll就可以再次触发events[i].data.fd上的EPOLLIN，驱动下一次读操作
                            printf("read later\n");
                            break;
                        }
                        close(events[i].data.fd);
                        break;
                        
                    }
                    else if(num2==0){
                        close(events[i].data.fd);
                    }
                    else{
                        printf("get %d byte of content : %s\n",num2,buf);
                        send(events[i].data.fd,buf,num2,0);
                    }
                }
                
            }
            else
            {
                printf("something else happened\n");
            }
            
        }


    }
    close(listenfd);
    return 0;
}