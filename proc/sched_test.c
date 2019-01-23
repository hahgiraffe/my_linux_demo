#include <stdio.h>
#include <sched.h>
#include "tlpi_hdr.h"

int main(int argc, char const *argv[])
{
    int j;
    struct sched_param sp;
    int pol=(argv[1][0]=='r')? SCHED_RR:(argv[1][0]=='f')? SCHED_FIFO:SCHED_OTHER;
    sp.__sched_priority=atoi(argv[2]);
    for(j=3;j<argc;j++){
        if(sched_setscheduler(atol(argv[j]),pol,&sp)==-1){
            printf("error sched_setscheduler\n");
        }
    }

    return 0;
}

