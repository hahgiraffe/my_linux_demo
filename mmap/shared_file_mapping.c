#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//共享文件映射例子，可直接反映到底层的映射文件
#define MEMSIZE 10
int main(int argc,char *argv[]){
    if(argc<2){
        printf("input argc error\n");
    }
    int fd=open(argv[1],O_RDWR);
    if(fd==-1){
        printf("open error\n");
    }
    char *addr=mmap(NULL,MEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(addr==MAP_FAILED){
        printf("mmap error\n");
    }    
    
    if(close(fd)==-1){
        printf("close fd error\n");
    }

    printf("current string %.*s\n",MEMSIZE,addr);//*表示MEMSIZE,即输出长度
    if(argc>2){
        if(strlen(argv[2])>MEMSIZE){
            printf("input too big error\n");
        }
        memset(addr,0,MEMSIZE);//内存空间清零或者赋值
        strncpy(addr,argv[2],MEMSIZE-1);
        if(msync(addr,MEMSIZE,MS_SYNC)==-1){
            printf("msync error\n");
        }

        printf("copy \" %s \" to shared memory\n",argv[2]);

    }
    exit(0);
}