#include <pthread.h>
#include "tlpi_hdr.h"
//测试pthread_key_t的用法
//就是一键多值的意思，让一个全局变量可以在不同线程中关联不同的值
char str[]="hello string";
pthread_key_t key;
pthread_mutex_t pmt=PTHREAD_MUTEX_INITIALIZER;
void *func1(void* arg){
    pthread_mutex_lock(&pmt);
    strcat(str," hello chs");
    printf("pthread1 %s\n",str);
    //fflush(STDOUT_FILENO);
    pthread_mutex_unlock(&pmt);

    int a=3;
    pthread_setspecific(key,(void*)a);
    printf("pthread1 %d\n",(int)pthread_getspecific(key));
    return (void *)1;
}

void *func2(void* arg){
    pthread_mutex_lock(&pmt);
    strcat(str," hello chs");
    printf("pthread2 %s\n",str);
    //fflush(STDOUT_FILENO);
    pthread_mutex_unlock(&pmt);

    int a=5;
    pthread_setspecific(key,(void*)a);
    printf("pthread1 %d\n",(int)pthread_getspecific(key));
    return (void*)2;
}

int main(){
    
    pthread_t tid1,tid2;
    int err;
    
    pthread_key_create(&key,NULL);
    err=pthread_create(&tid1,NULL,(void*)func1,NULL);
    if(err==-1){
        printf("create error\n");
    }
    err=pthread_create(&tid2,NULL,(void*)func2,NULL);
    if(err==-1){
        printf("create error\n");
    }
    printf("main pthread\n");
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    //sleep(2);
    int a=8;
    pthread_setspecific(key,(void*)a);
    printf("pthread1 %d\n",(int)pthread_getspecific(key));
    pthread_key_delete(key);
    return;
}