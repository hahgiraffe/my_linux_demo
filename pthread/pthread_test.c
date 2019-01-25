#include <pthread.h>
#include "tlpi_hdr.h"

static pthread_cond_t pct=PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pmt=PTHREAD_MUTEX_INITIALIZER;

static int totThreads=0;
static int numlive=0;
static int numunjoin=0;
enum tstate{
    TS_ALIVE,
    TS_TERMINATED,
    TS_JOINED
};

static struct{
    pthread_t pid;
    enum tstate state;
    int sleeptime;
} *thread;

static void* threadfun(void *arg)
{
    int idx=(int)arg;
    int s;
    sleep(thread[idx].sleeptime);
    printf("thread %d terminating\n",idx);
    s=pthread_mutex_lock(&pmt);
    if(s!=0)
        printf("pthread_mutex_lock error\n");
    numunjoin++;
    thread[idx].state=TS_TERMINATED;
    s=pthread_mutex_unlock(&pmt);
    if(s!=0)
        printf("pthread_mutex_unlock error\n");
    s=pthread_cond_signal(&pct);
    if(s!=0)
    {
        printf("pthread_cond_signal error\n");
    }
    return NULL;

} 

//这个还是有问题的


int main(int argc, char const *argv[])
{
    if(argc<2)
    {
        printf("input error\n");
    }
    thread = calloc(argc-1,sizeof(*thread));
    if(thread==NULL)
    {
        printf("calloc");
    }
    int idx;
    int s;
    for(idx=0;idx<argc-1;idx++)
    {
        thread[idx].sleeptime=atoi(argv[idx+1]);
        thread[idx].state=TS_ALIVE;
        s=pthread_create(&thread[idx].pid,NULL,threadfun,idx);
        if(s!=0)
        {
            printf("pthread_create error\n");
        }
    }

    totThreads=argc-1;
    numlive=argc-1;
    while(numlive>0)
    {
        s=pthread_mutex_lock(&pmt);
        if(s!=0)
        {
            printf("pthread_mutex_lock error\n");
        }
        while(numunjoin==0)
        {
            s=pthread_cond_wait(&pct,&pmt);
            if(s!=0)
            {
                printf("pthread_cond_wait error\n");
            }
        }

        for(idx=0;idx<totThreads;idx++)
        {
            if(thread[idx].state==TS_TERMINATED)
            {
                s=pthread_join(thread[idx].pid,NULL);
                if(s!=0)
                {
                    printf("pthread_join error\n");
                }
                thread[idx].state=TS_JOINED;
                numlive--;
                numunjoin--;
                printf("thread %d numlive %d\n",idx,numlive);
            }
        }

        s=pthread_mutex_unlock(&pmt);
        if(s!=0)
        {
            printf("pthread_mutex_unlock error\n");
        }
    }

    return 0;
}

