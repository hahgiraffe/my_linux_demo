#include "tlpi_hdr.h"
#include <pthread.h>

void cleanup(void *arg){
    printf("clean up %s\n",arg);
}

void *pthread_start(void *arg){
    printf("pthead 1 start\n");
    pthread_cleanup_push(cleanup,"thread1 clean up 1");
    pthread_cleanup_push(cleanup,"thread1 clean up 2");
    printf("pthead 1 cleanup end\n");
    if(arg){
        return (void*)1;
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    return (void *)1;
}

void *pthread_start2(void *arg){
    printf("pthead 2 start\n");
    pthread_cleanup_push(cleanup,"thread2 clean up 1");
    pthread_cleanup_push(cleanup,"thread2 clean up 2");
    printf("pthead 2 cleanup end\n");
    if(arg){
        pthread_exit((void*)2);  //pthead_exit调用
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    return (void *)2;
}

int main(int argc,char *argv[]){
    int err;
    pthread_t p1,p2;
    void *ret;
    err=pthread_create(&p1,NULL,pthread_start,(void *)1);
    err=pthread_create(&p2,NULL,pthread_start2,(void *)2);

    err=pthread_join(p1,&ret);
    printf("pthread 1 exit code %ld\n",ret);
    err=pthread_join(p2,&ret);
    printf("pthread 2 exit code %ld\n",ret);
    return 0;
}