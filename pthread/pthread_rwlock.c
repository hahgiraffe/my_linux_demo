#define _XOPEN_SOURCE 500
#include <pthread.h>
#include "tlpi_hdr.h"
#include <stdlib.h>

//如果两个都是读模式下加锁，就会出现如下不确定情况。如果两个中只要有一个是写锁就不会出问题
// main pthread begin,readlock
// son pthread begin,writelock
// value of a 1
// son pthread end,write unlock
// value of a 0
// main pthread end,read unlock

pthread_rwlock_t prwt=PTHREAD_RWLOCK_INITIALIZER;
static int a=1;
void *pthread_func(void *arg){
    int err;
    err=pthread_rwlock_rdlock(&prwt);
    printf("son pthread begin,writelock\n");
    a++;
    printf("value of a %d\n",a);
    printf("son pthread end,write unlock\n");
    err=pthread_rwlock_unlock(&prwt);
    return (void*)0;
}

int main(){
    int err;
    pthread_t p;
    //err=pthread_rwlock_init(&prwt,NULL);
    err=pthread_create(&p,NULL,pthread_func,NULL);
    err=pthread_rwlock_rdlock(&prwt);
    printf("main pthread begin,readlock\n");
    a--;
    printf("value of a %d\n",a);
    printf("main pthread end,read unlock\n");
    err=pthread_rwlock_unlock(&prwt);
    //pthread_attr_destroy(&prwt);
    err=pthread_join(p,NULL);
    return 0;
}