#include <mqueue.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <error.h>
int main(int argc,char *argv[])
{
    mqd_t mq;
    mq=mq_open("/new_mq",O_CREAT|O_EXCL|O_RDWR,0666,NULL);//mq名称须以“/”开头
    if(mq<0){
        printf("mq_open error\n");
        exit(0);
    }
    int pid;
    struct mq_attr attr;
    if(mq_getattr(mq,&attr)==-1){
        printf("mq_getattr error\n");
        exit(0);
    }
    printf("maxmsg is %ld\n",attr.mq_maxmsg);
    printf("mq_msgsize is %ld\n",attr.mq_msgsize);
    printf("mq_curmsgs is %ld\n",attr.mq_curmsgs);
    pid=fork();
    if(pid<0){
        printf("fork error\n");
        exit(0);
    }
    else if(pid==0){//child
        char buf_send[]="hello";
        if(mq_send(mq,buf_send,sizeof(buf_send),1)<0){
            printf("send error\n");
            //printf("error information %s",strerror(errno));
            exit(0);
        }
        mq_send(mq,buf_send,sizeof(buf_send)+1,3);
        mq_send(mq,buf_send,sizeof(buf_send)+1,2);
        _exit(0);
    }
    else{
        sleep(2);
        char *buff=malloc(attr.mq_msgsize);
        for(int i=1;i<=3;i++){
            if(mq_receive(mq,buff,attr.mq_msgsize,NULL)<0){
                printf("mq_recieve error\n");
                exit(0);
            }
            printf("%s\n",buff);
        }
        mq_close(mq);
        mq_unlink("/new_mq");
    }
    return 0;
}