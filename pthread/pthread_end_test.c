#include <pthread.h>
#include "tlpi_hdr.h"
static int num=1;

void *thread_start(void *arg){
    char *str="thread 1 exiting";
    num++;
    return (void*)str;
}

void *thread_start2(void *arg){
    char *str="thread 2 exiting";
    num++;
    pthread_exit((void*)str);
}

int main(int argc,char *argv[]){
    pthread_t p,p2;
    int err;
    void *tret;
    err=pthread_create(&p,NULL,thread_start,NULL);
    if(err!=0){
        printf("create pthread error\n");
    }
    err=pthread_create(&p2,NULL,thread_start2,NULL);
    if(err!=0){
        printf("create pthread error\n");
    }
    err=pthread_join(p,&tret);
    printf("ptread1 exit code %s \n",tret);   //ptread1 exit code thread 1 exiting 
    err=pthread_join(p2,&tret);
    printf("ptread2 exit code %s \n",tret);   //ptread2 exit code thread 1 exiting 
    printf("num is %d\n",num);   //num is 3,表明线程是共享进程的资源的
    return;
}