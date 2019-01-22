#include "stdio.h"
#include "unistd.h"
int main(){
    char buf[10];
    int readnum=read(STDIN_FILENO,buf,10);
    if(readnum==-1){
        printf("error read\n");
    }
    if(write(STDOUT_FILENO,buf,readnum)!=readnum){
        printf("error write\n");
    }
    return 0;
}