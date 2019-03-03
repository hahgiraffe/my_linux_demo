#include "unp.h"

void str_cli(FILE* fp,int sockfd)
{
    //客户端监听两个描述符，一个是标准输入，一个是套接字
    fd_set rset;
    FD_ZERO(&rset);
    char buf[MAXLINE];
    int n;
    int maxfdp1,stdineof=0;
    FD_ZERO(&rset);
    for(;;){
        if(stdineof==0){
            FD_SET(fileno(fp),&rset);
        }
        FD_SET(sockfd,&rset);
        maxfdp1= max(fileno(fp),sockfd) +1;
        select(maxfdp1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(sockfd,&rset)){//socket准备就绪
            if((n=read(sockfd,buf,MAXLINE))==0){
                if(stdineof==1){
                    return;
                }
                else{
                    err_quit("server terminated\n");
                }
            }
            write(fileno(stdout),buf,n);

        }
        if( FD_ISSET(fileno(fp),&rset) ){
            if((n=read(fileno(fp),buf,MAXLINE))==0){ //当标准输入缓冲区读完的时候
                stdineof=1;
                shutdown(sockfd,SHUT_WR);
                FD_CLR(fileno(fp),&rset);
                continue;
            }
            write(sockfd,buf,n);
        }

    }
}

int main(int argc,char *argv[]){
    int clifd;
    struct sockaddr_in servaddr;
    clifd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(SERV_PORT);
    connect(clifd,&servaddr,sizeof(servaddr));
    str_cli(stdin,clifd);
    exit(0);
}
