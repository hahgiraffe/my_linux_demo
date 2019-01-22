#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "sys/wait.h"
#define BUFFSIZE 20

int main(int argc, char const *argv[])
{
    int pfd[2];
    char buf[BUFFSIZE];
    if(argc!=2){
        printf("error input\n");
    }
    if(pipe(pfd)==-1){
        printf("error pipe\n");
    }
    switch( fork()){
    case -1:
        printf("error fork\n");
    case 0:

        //child proc
        close(pfd[1]);
        for(;;){
            size_t readfrompipe=read(pfd[0],buf,BUFFSIZE);
            if(readfrompipe==-1){
                printf("error read\n");
            }
            if(readfrompipe==0){
                write(STDOUT_FILENO,"\nbye\n",5);
                break;
            }
            if(write(STDOUT_FILENO,buf,readfrompipe)!=readfrompipe){
                printf("error write\n");
            }
        }
        if(close(pfd[0])==-1){
            printf("error close\n");
        }
        _exit(0);
    default:
        break;
    }
    if(close(pfd[0])==-1){
        printf("error close\n");
    }
    if(write(pfd[1],argv[1],strlen(argv[1]))!=strlen(argv[1])){
        printf("error write\n");
    }

    if(close(pfd[1])==-1){
        printf("error close\n");
    }
    wait(NULL);
    return 0;
}
