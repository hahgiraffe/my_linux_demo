#include "http_parse.h"
#include <vector>
using namespace std;
/*
此程序是用epoll实现聊天室demo
一个人发消息，只要连接到这个服务器上的客户端都可以收到这个消息
服务器用一个vector存储用户连接的端口
*/

#define MAX_EVENTS_NUMBER 1024
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

    int epollfd=epoll_create(MAX_EVENTS_NUMBER);
    addfd(epollfd,listenfd);
    vector<int> client_list_fd;
    while(1){
        struct epoll_event events[MAX_EVENTS_NUMBER];
        //printf("in\n");
        int ret=epoll_wait(epollfd,events,MAX_EVENTS_NUMBER,-1);
        if(ret<0){
            printf("epoll wait error\n");
            break;
        }
        for(int i=0;i<ret;i++){
            int sockfd=events[i].data.fd;
            if(sockfd==listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addr_len=sizeof(client_address);
                int connfd=accept(listenfd,(sockaddr *)&client_address,&client_addr_len);
                assert(connfd!=-1);
                //printf("a new tcpclient connect,ip%s  port%s\n",(char *)ntohl(client_address.sin_addr.s_addr),(char*)(client_address.sin_port));
                printf("a new tcpclient connect\n");
                client_list_fd.push_back(connfd);
                addfd(epollfd,connfd);                
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
                            //printf("read later\n");
                            break;
                        }
                        close(events[i].data.fd);
                        break;
                        
                    }
                    else if(num2==0){
                        close(events[i].data.fd);
                    }
                    else{
                        printf("get %d byte of content : %s",num2,buf);
                        for(int j=0;j<client_list_fd.size();j++){
                            if(client_list_fd[j] != events[i].data.fd){
                                //这里可以根据events[i].data.fd得到是哪个客户发来的，在vector中查找索引，然后可以连同这个信息一起发给其他用户
                                //printf("send one client\n");
                                send(client_list_fd[j],buf,num2,0);
                            }
                        }
                        
                    }
                }
            }
            else{
                printf("something else \n");
            }
        }
    }
    close(listenfd);
    return 0;
}
