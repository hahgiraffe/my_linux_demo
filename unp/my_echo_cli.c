#include "unp.h"

void cli_echo(int clifd){
    char send_buf[MAXLINE],recive_buf[MAXLINE];
    while(Fgets(send_buf,MAXLINE,stdin)){
        writen(clifd,send_buf,strlen(send_buf));
        if(Readline(clifd,recive_buf,MAXLINE)==0){
            printf("read error\n");
        }
        fputs(recive_buf,stdout);
        
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
    cli_echo(clifd);
    exit(0);
}