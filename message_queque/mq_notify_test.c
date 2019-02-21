#include <mqueue.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <error.h>
int main(int argc,char *argv[]){
    mqd_t mq;
    mq=mq_open(argv[1],O_CREAT|O_EXCL|O_RDWR,0666,NULL);
    struct mq_attr attr;
    if(mq_getattr(mq,&attr)==-1){
        printf("mq_getattr error\n");
        exit(0);
    }

}