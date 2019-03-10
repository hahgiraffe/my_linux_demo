#include "http_parse.h"

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

int main(int agrc,char *argv[]){
    int clifd;
    struct sockaddr_in servaddr;
    clifd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[2]));
    int err=connect(clifd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    assert(err!=-1);
    int epollfd=epoll_create(1024);
    addfd(epollfd,clifd);
    addfd(epollfd,STDIN_FILENO);
    while(1){
        struct epoll_event events[1024];
        //printf("in\n");
        int ret=epoll_wait(epollfd,events,1024,-1);
        if(ret<0){
            printf("epoll wait error\n");
            break;
        }
        for(int i=0;i<ret;i++){
            if(events[i].data.fd==STDIN_FILENO){
                //标准输入输入
                char buffer[1024];
                int read_in=read(STDIN_FILENO,buffer,1024);
                send(clifd,buffer,read_in,0);
            }
            else if(events[i].events & EPOLLIN){
                char buf[1024];
                while(1){
                    memset(buf,'\0',1024);
                    int num2=recv(events[i].data.fd,buf,1024-1,0);//最大可以取BUFFSIZE-1，最后一个'\0'表好似字符串结束
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
                        //printf("get message %s\n",buf);
                    }
                }
            }
        }
    }
}