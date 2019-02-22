#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <error.h>
#include <semaphore.h>
#include <pthread.h>
//信号量机制也可以用于线程同步,未命名信号量对全局变量进行访问，
//因为全局static变量在虚拟地址空间中是处于初始化和未初始化区域，是同一个进程中不同线程共享的数据
static int global=0;
static sem_t sem;
static void * threadFunc(void *arg){
    int loop=*(int*)arg;
    int j=0;
    int loc;
    for(j=0;j<loop;j++){
        if(sem_wait(&sem)==-1){
            printf("error\n");
        }
        /*loc=global;
        loc++;
        global=loc;
        */
       global++;
        if(sem_post(&sem)==-1){
            printf("error\n");
        }

    }
    return NULL;
}
int main(int argc,char *argv[])
{
    
    if(sem_init(&sem,0,1)==-1){
        printf("error\n");
        exit(0);
    }
    int loop=1000;
    int k;
    pthread_t pid1,pid2;
    k=pthread_create(&pid1,NULL,threadFunc,&loop);
    if(k!=0){
        printf("error\n");
        exit(0);
    }
    k=pthread_create(&pid2,NULL,threadFunc,&loop);
    if(k!=0){
        printf("error\n");
        exit(0);
    }
    k=pthread_join(pid1,NULL);
    k=pthread_join(pid2,NULL);
    printf("global is %d\n",global);
    sem_destroy(&sem);
    return 0;
}