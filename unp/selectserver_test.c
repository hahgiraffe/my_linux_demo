#include "unp.h"

int main(int argc,char *argv[]){
    int listenfd,maxfd,sockfd,connfd,i,n;
    struct sockaddr_in serveraddr,clientaddr;
    int maxi,nready;
    int clientlen;
    int client[FD_SETSIZE];
    fd_set rset,allset;
    char buf[MAXLINE];
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    serveraddr.sin_port=htons(SERV_PORT);
    bind(listenfd,(SA*)&serveraddr,sizeof(serveraddr));
    listen(listenfd,LISTENQ);
    maxfd=listenfd;
    maxi=-1;    //chlient数组下标
    for(i=0;i<FD_SETSIZE;i++){
        client[i]=-1;//初始化-1
    }
    
    FD_ZERO(&allset);
    FD_SET(listenfd,&allset);
    for(;;){
        rset=allset;
        nready=select(maxfd+1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(listenfd,&rset)){  //新客户端连接
            clientlen=sizeof(clientaddr);
            connfd=accept(listenfd,(SA*)&clientaddr,&clientlen);
            struct sockaddr_in addr;
            int len=sizeof(addr);
            if(getsockname(connfd,(SA*)&addr,&len)<0){
                err_quit("getsockname error\n");
            }
            printf("the client address is %d the port is %d\n",(int)addr.sin_addr.s_addr,(int)addr.sin_port);
            fflush(stdout);
            for(i=0;i<FD_SETSIZE;i++){
                if(client[i]<0){
                    client[i]=connfd;  //把已连接的套接字保存在数组中
                    break;
                }
            }
            if(i==FD_SETSIZE){
                printf("length error\n");
            }
            FD_SET(connfd,&allset);//注意这里是添加到allset，所以接下来第一次不会遍历其
            if(connfd>maxfd){
                maxfd=connfd;
            }
            if(i>maxi){
                maxi=i;
            }
            if(--nready <=0){
                continue;
            }
        }
        //遍历一遍client数组，看哪个描述符准备就绪
        for(i=0;i<=maxi;i++){
            sockfd=client[i];
            if(sockfd<0){
                continue;
            }
            if(FD_ISSET(sockfd,&rset)){
                if((n=read(sockfd,buf,MAXLINE))==0){//客户断开
                    close(sockfd);
                    FD_CLR(sockfd,&rset);
                    client[i]=-1;
                }
                else{
                    //write(STDOUT_FILENO,buf,n);加上这个就不行了，是不是因为进程也监视了标准输出1？
                    printf("%s\n",buf);
                    fflush(stdout);
                    write(sockfd,buf,n);
                }

                if(--nready <=0){
                    break;
                }
            }
        }
    }

}