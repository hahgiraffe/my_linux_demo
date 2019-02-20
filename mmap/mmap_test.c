#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
//相当于cat的功能,私有文件映射（写时复制技术）
int main(int argc,char *argv[]){
    if(argc!=2){
        printf("error argc\n");
    }
    int fd=open(argv[1],O_RDONLY);
    if(fd==-1){
        printf("open file error\n");
    }
    struct stat sb;
    if(fstat(fd,&sb)==-1){
        printf("fstat error\n");
    }
    char *addr=mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(addr==MAP_FAILED){
        printf("mmap error\n");
    }
    if(write(STDOUT_FILENO,addr,sb.st_size)!=sb.st_size){
        printf("write error\n");
    }
    exit(0);
}