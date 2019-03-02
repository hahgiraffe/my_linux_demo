#include "unp.h"

void echo(int clifd){
    char buf[MAXLINE];
    int n;

again:
    while((n=read(clifd,buf,MAXLINE))>0){
        writen(clifd,buf,n);
        int i=0;
        while(buf[i]>='a' && buf[i]<='z'){
            buf[i]=buf[i]-32;
            i++;
        }
        write(STDOUT_FILENO,buf,n);
    }
    if(n<0 && errno==EINTR){
        goto again;
    }
    else if(n<0){
        printf("read error\n");
    }

}
void sig_chld(int signo){//当信号处理函数进行时候，只能处理一个相同类型的信号，即不排队，所以要用waitpid
    pid_t pid;
    int stat;
    //pid=wait(&stat);
    //printf(" child %d terminated\n",pid);
    while((pid=waitpid(-1,&stat,WNOHANG))>0){
        printf("child %d terminated\n,pid");
    }
    return;
}

int main(int argc,char *argv[]){
    int listenfd,clifd;
    struct sockaddr_in seraddr,cliaddr;
    int clilen;
    pid_t clipid;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&seraddr,sizeof(seraddr));
    seraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    seraddr.sin_family=AF_INET;
    seraddr.sin_port=htons(SERV_PORT);
    bind(listenfd,(SA*)&seraddr,sizeof(seraddr));
    listen(listenfd,LISTENQ);
    signal(SIGCHLD,sig_chld);
    for(;;){
        clilen=sizeof(cliaddr);
        if((clifd=accept(listenfd,&cliaddr,&clilen))<0){
            if(errno=EINTR){
                //这里代表在accept阻塞时候，遇到信号，接着就去信号处理函数，
                //但是信号处理函数返回的时候内核没有重启accept,就会有EINTR(部分os会有这个问题，Ubuntu上没有)
                continue;
            }
            else{
                printf("accept error\n");
            }

        }
        if((clipid=fork())==0){
            //child
            close(listenfd);
            echo(clifd);
            close(clifd);//如果没有这个，exit也会关掉本进程的所有打开的描述符
            _exit(0);
        }
        close(clilen);
    }

}