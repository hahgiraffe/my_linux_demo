#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "sys/wait.h"
#include <ctype.h>
#define BUFSIZE 10000
/*
从标准输入输入到父进程，父进程通过管道传输给子进程，子进程变成大写
再通过管道传输回父进程，父进程输入到标准输出
*/
int main(int argc, char const *argv[])
{
    int pipe1[2];//parent write child read
    int pipe2[2];//parent read child write
    int fd;
    int sz;
    char buf[BUFSIZE];
    int j;
    if(pipe(pipe1)==-1){
        printf("pipe1 error\n");
    }
    if(pipe(pipe2)==-1){
        printf("pipe2 error\n");
    }
    switch(fork()){
    case -1:
        printf("error fork\n");

    case 0:
        //child proc
        if(close(pipe1[1])==-1){
            printf("pipe1 close error\n");
        }
        if(close(pipe2[0])==-1){
            printf("pipe1 close error\n");
        }
        while((sz=read(pipe1[0],buf,BUFSIZE))>0){
            for(j=0;j<sz;j++){
                buf[j]=toupper(buf[j]);
            }
            if(write(pipe2[1],buf,sz)!=sz){
                printf("child write error\n");
            }
        }
        //如果sz==0说明读完，sz==-1说明出错
        if(sz==-1){
            printf("child read error\n");
        }   
        _exit(0);
    default:
        break;
    }
    if(close(pipe1[0])==-1){
        printf("pipe1 close error\n");
    }
    if(close(pipe2[1])==-1){
        printf("pipe1 close error\n");
    }
    while(sz=read(STDIN_FILENO,buf,BUFSIZE))
    {
        if(write(pipe1[1],buf,sz)!=sz){
            printf("parent write error\n");
        }
        if((sz=read(pipe2[0],buf,BUFSIZE))==-1){
            printf("parent read error\n");
        }
        if(sz>0){
            if(write(STDOUT_FILENO,buf,sz)!=sz){
                printf("output error\n");
            }
        }
    }
    if(sz==-1){
        printf("parent read error\n");
    }
    return 0;
}
