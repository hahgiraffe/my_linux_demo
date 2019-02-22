#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <error.h>
#include <semaphore.h>

//信号量机制用于进程同步,命名信号量 
int main(int argc,char *argv[])
{
    sem_t *sem;
    sem=sem_open("new_sem",O_CREAT,O_RDWR,0);
    if(sem==SEM_FAILED){
        printf("error\n");
        exit(0);
    }
    int value;
    pid_t pid=fork();
    switch (pid)
    {
        case -1:
            printf("error\n");
            break;
        case 0:
            
            sem_getvalue(sem,&value);
            printf("child value %d\n",value);
            if(sem_wait(sem)==-1){
                printf("error\n");
            }
            printf("child end\n");
            _exit(0);
        default:
            break;
    }
    sleep(2);
    if(sem_post(sem)==-1){
        printf("error\n");
    }
    sem_getvalue(sem,&value);
    printf("father value %d\n",value);
    wait(NULL);
    printf("father end\n");
    sem_unlink("new_sem");
    return 0;
}