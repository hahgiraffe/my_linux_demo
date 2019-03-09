#include "http_parse.h"
#define MAX_EVENT_NUMBER 1024
#define BUFFSIZE 10

int setnonblock(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void addfd(int epollfd,int fd,bool enable_et){ //这个函数就是将fd加到epollfd所指向的内核事件表中
    struct epoll_event epoll_e;
    epoll_e.events=EPOLLIN;
    epoll_e.data.fd=fd;
    if(enable_et){
        epoll_e.events|=EPOLLET;
    }
    int err= epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&epoll_e);
    if(err==-1){
        printf("epoll_ctl error\n");
    }
    setnonblock(fd);//每个设置为ET都是将fd设置为非阻塞
}

void use_LT(epoll_event *events,int number,int epollfd, int listenfd){
    //epoll默认是水平触发
    char buf[BUFFSIZE];
    for(int i=0;i<number;i++){
        int sockfd=events[i].data.fd;
        if(listenfd==sockfd){//当有新连接的时候，通过fd判断
            struct sockaddr_in client_addr;
            socklen_t client_addr_len=sizeof(client_addr);
            int clientfd=accept(listenfd,(struct sockaddr *)&client_addr,&client_addr_len);
            addfd(epollfd,clientfd,false);
        }
        else if(events[i].events & EPOLLIN){//当有读取事件的时候，代码即触发，当一次没有读取完，可以第二次触发
            printf("event target once\n");
            memset(buf,'\0',BUFFSIZE);
            int ret=recv(sockfd,buf,BUFFSIZE-1,0);
            if(ret<=0){//如果读完或者异常
                close(sockfd);
                continue;
            }
            printf("get %d byte of content : %s\n",ret,buf);
        }
        else{
            printf("something else happened\n");
        }
    }

}

void use_ET(epoll_event *events,int number,int epollfd, int listenfd){
    //设置为边缘触发
    char buf[BUFFSIZE];
    for(int i=0;i<number;i++){
        int sockfd=events[i].data.fd;
        if(listenfd==sockfd){//当有新连接的时候，通过fd判断
            struct sockaddr_in client_addr;
            socklen_t client_addr_len=sizeof(client_addr);
            int clientfd=accept(listenfd,(struct sockaddr *)&client_addr,&client_addr_len);
            addfd(epollfd,clientfd,true);//对新连接的client开启边缘触发
        }
        else if(events[i].events & EPOLLIN){//当有读取事件的时候，代码即触发，但是边缘触发不会重复触发，所以要确保缓冲区中的数据完全读出
            printf("event target once\n");
            while(1){
                memset(buf,'\0',BUFFSIZE);
                int ret=recv(sockfd,buf,BUFFSIZE-1,0);//最大可以取BUFFSIZE-1，最后一个'\0'表好似字符串结束
                if(ret<0){
                    //如果读完
                    if(errno==EAGAIN || errno==EWOULDBLOCK){
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                    
                }
                else if(ret==0){
                   close(sockfd);
                }
                else{
                     printf("get %d byte of content : %s\n",ret,buf);
                }
            }
        }
        else{
            printf("something else happened\n");
        }
    }

}


int main(int argc,char *argv[]){
    int listenfd;
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,argv[1],&addr.sin_addr);
    addr.sin_port=htons(atoi( argv[2]));
    //inet_pton(AF_INET,argv[2],&addr.sin_port);
    
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd>=0);
    int err=bind(listenfd,(struct sockaddr *)&addr,sizeof(addr));
    assert(err != -1);
    err=listen(listenfd,5);
    assert(err!=-1);
    epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd=epoll_create(5);
    assert(epoll_fd!=-1);
    addfd(epoll_fd,listenfd,true);
    //接下来分别用epoll的LT和ET来处理事件
    
    while(1){
        int ret=epoll_wait(epoll_fd,events,MAX_EVENT_NUMBER,-1);
        if(ret<0){
            printf("epoll wait error\n");
            break;
        }
        //printf("get one\n");
        //use_LT(events,ret,epoll_fd,listenfd);
        use_ET(events,ret,epoll_fd,listenfd);
    }
    
    close(listenfd);
    return 0;
}